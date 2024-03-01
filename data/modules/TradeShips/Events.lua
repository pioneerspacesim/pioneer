-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Comms = require 'Comms'
local Engine = require 'Engine'
local Event = require 'Event'
local Game = require 'Game'
local ShipDef = require 'ShipDef'
local utils = require 'utils'
local CommodityType = require 'CommodityType'

local Core = require 'modules.TradeShips.Core'
local Flow = require 'modules.TradeShips.Flow'
local Trader = require 'modules.TradeShips.Trader'

-- this module contains all events affecting tradeships

local onGameStart = function ()
	if Flow.calculateSystemParams() then
		if Core.ships == nil then -- new game
			Flow.spawnInitialShips()
			Flow.run()
		else -- deserialization
			-- check if any trade ships were waiting on a timer
			-- restart
			for ship, trader in pairs(Core.ships) do
				if trader.delay and trader.fnc and trader.delay > Game.time then
					Core.log:add(ship, "Resume " .. trader.fnc)
					Trader.assignTask(ship, trader.delay, trader.fnc)
				end
			end
			Flow.run()
		end
	else
		-- Failed to init parameters for this system, just make a table to prevent further errors
		Core.ships = Core.ships or {}
	end
end
Event.Register("onGameStart", onGameStart)

local onEnterSystem = function (ship)
	-- dont crash when entering unexplored systems
	if Game.system.explored == false then
		return
	end

	-- clearing the cache of ships that attacked traders, but are no longer relevant
	Core.attackers.clean()

	-- if the player is following a ship through hyperspace that ship may enter first
	-- so update the system when the first ship enters (see Space::DoHyperspaceTo)
	if ship:IsPlayer() then
		Flow.updateTradeShipsTable()
		if Flow.calculateSystemParams() then
			Flow.spawnInitialShips()
			Flow.run()
		end
	elseif Core.ships[ship] ~= nil then
		local trader = Core.ships[ship]
		Core.log:add(ship, 'Entered '..Game.system.name..' from '..trader.from_path:GetStarSystem().name)
		if trader.route then
			ship:AIDockWith(trader.route.to)
			Core.ships[ship]['starport'] = trader.route.to
			Core.ships[ship]['status'] = 'inbound'
		else
			-- starport == nil happens if player has followed ship to empty system, or
			-- no suitable port found (e.g. all stations atmospheric for ship without atmoshield)
			Trader.getSystemAndJump(ship)
			-- if we couldn't reach any systems wait for player to attack
		end
	end
end
Event.Register("onEnterSystem", onEnterSystem)

local onLeaveSystem = function (ship)
	if ship:IsPlayer() then
		-- the next onEnterSystem will be in a new system
		local total, removed = 0, 0
		for t_ship, trader in pairs(Core.ships) do
			total = total + 1
			if trader.status == 'hyperspace' then
				if trader.dest_path:IsSameSystem(Game.system.path) then
					-- remove ships that are in hyperspace to here
					Core.ships[t_ship] = nil
					removed = removed + 1
				end
			else
				-- remove all ships that are not in hyperspace
				Core.ships[t_ship] = nil
				removed = removed + 1
			end
		end
		Core.log:add(nil, 'onLeaveSystem:total:'..total..',removed:'..removed)
	elseif Core.ships[ship] ~= nil then
		local system = Core.ships[ship]['dest_path']:GetStarSystem()
		Core.log:add(ship, 'Left '..Game.system.name..' for '.. system.name)
		Flow.cleanTradeShipsTable()
	end
end
Event.Register("onLeaveSystem", onLeaveSystem)

local onShipDocked = function (ship, starport)
	if Core.ships[ship] == nil then return end
	local trader = Core.ships[ship]

	trader.route = nil
	Core.log:add(ship, 'Docked with '..starport.label)

	if trader.status == 'fleeing' then
		trader['status'] = 'cowering'
	else
		trader['status'] = 'docked'
	end
	if trader.chance then
		trader['chance'] = trader.chance / 2
		trader['last_flee'], trader['no_jump'] = nil, nil
	end

	-- 'sell' trade cargo
	---@type CargoManager
	local cargoMgr = ship:GetComponent('CargoManager')

	for name, info in pairs(cargoMgr.commodities) do
		local commodity = CommodityType.GetCommodity(name)
		cargoMgr:RemoveCommodity(commodity, info.count)
	end

	local damage = ShipDef[trader.ship_name].hullMass - ship.hullMassLeft
	if damage > 0 then
		ship:SetHullPercent()
		Trader.addEquip(ship)
	end
	Trader.addFuel(ship)
	ship:SetFuelPercent()

	if trader.status == 'docked' then
		Trader.assignTask(ship, Game.time + utils.deviation(Core.params.port_params[starport].time * 3600, 0.8), 'doUndock')
	end
end
Event.Register("onShipDocked", onShipDocked)

local onShipUndocked = function (ship, starport)
	if Core.ships[ship] == nil then return end
	local trader = Core.ships[ship]
	ship:AIEnterLowOrbit(trader.starport:GetSystemBody().system:GetStars()[1].body)
	Trader.assignTask(ship, Game.time + 10, 'hyperjumpAtDistance')
	trader['status'] = 'outbound'
end
Event.Register("onShipUndocked", onShipUndocked)

local onAICompleted = function (ship, ai_error)
	if Core.ships[ship] == nil then return end
	local trader = Core.ships[ship]
	if ai_error ~= 'NONE' then
		Core.log:add(ship, 'AICompleted: Error: '..ai_error..' Status: '..trader.status)
	end
	if trader.status == 'orbit' then
		if ai_error == 'NONE' then
			trader.ts_error = "wait_6h"
			Trader.assignTask(ship, Game.time + 21600, 'doRedock')
		end
		-- XXX if ORBIT_IMPOSSIBLE asteroid? get parent of parent and attempt orbit?
	elseif trader.status == 'inbound' then
		if ai_error == 'REFUSED_PERM' then
			Trader.doOrbit(ship)
			trader.ts_error = "refused_perm"
		end
	end
end
Event.Register("onAICompleted", onAICompleted)

local onShipLanded = function (ship, _)
	if Core.ships[ship] == nil then return end
	Core.log:add(ship, 'Landed: '..Core.ships[ship].starport.label)
	Trader.doOrbit(ship)
end
Event.Register("onShipLanded", onShipLanded)

local onShipAlertChanged = function (ship, alert)
	if Core.ships[ship] == nil then return end
	if alert == 'SHIP_FIRING' then
		Core.log:add(ship, 'Alert changed to '..alert) end
	local trader = Core.ships[ship]
	if trader.attacker == nil then return end

	if alert == 'NONE' or not trader.attacker.ref:exists() or
		(alert == 'SHIP_NEARBY' and ship:DistanceTo(trader.attacker.ref) > 100) then
		if trader.status == 'fleeing' then
			-- had not reached starport yet
			trader['status'] = 'inbound'
			trader.ts_error = "alert!"
		elseif trader.status == 'cowering' then
			-- already reached starport and docked
			-- we need to do everything that the ship does after docking
			onShipDocked(ship, trader.starport)
		end
	end
end
Event.Register("onShipAlertChanged", onShipAlertChanged)

local onShipHit = function (ship, attacker)
	if attacker == nil then return end-- XX

	-- XXX this whole thing might be better if based on amount of damage sustained
	if Core.ships[ship] == nil then return end
	local trader = Core.ships[ship]
	trader.ts_error = "HIT"

	trader['chance'] = trader.chance or 0
	trader['chance'] = trader.chance + 0.1

	-- don't spam actions
	if trader.last_flee and Game.time - trader.last_flee < Engine.rand:Integer(5, 7) then return end

	-- if outbound jump now
	if trader.status == 'outbound' then
		if Trader.getSystemAndJump(ship) == 'OK' then
			return
		end
	end

	trader['status'] = 'fleeing'
	Core.attackers.add(trader, attacker)

	-- update last_flee
	trader['last_flee'] = Game.time

	-- if distance to starport is far attempt to hyperspace
	if trader.no_jump ~= true then
		local starports = Core.params.local_routes
		if #starports == 0 then
			trader['no_jump'] = true -- it already tried in onEnterSystem
		elseif trader.starport and Engine.rand:Number(1) < trader.chance then
			local distance = ship:DistanceTo(trader.starport)
			if distance > Core.AU * (2 - trader.chance) then
				if Trader.getSystemAndJump(ship) == 'OK' then
					return
				else
					trader['no_jump'] = true
					trader['chance'] = trader.chance + 0.3
				end
			end
		end
	end

	-- maybe jettison a bit of cargo
	if Engine.rand:Number(1) < trader.chance then
		local cargo_type = nil
		local max_cap = ShipDef[ship.shipId].capacity

		---@type CargoManager
		local cargoMgr = ship:GetComponent('CargoManager')

		for name, info in pairs(cargoMgr.commodities) do
			if info.count > 1 and Engine.rand:Number(1) < (info.count / max_cap) then
				cargo_type = CommodityType.GetCommodity(name)
				break
			end
		end

		if cargo_type and ship:Jettison(cargo_type) then
			Comms.ImportantMessage(attacker.label..', take this and leave us be, you filthy pirate!', ship.label)
			trader['chance'] = trader.chance - 0.1
		end
	end
end
Event.Register("onShipHit", onShipHit)

local onShipCollided = function (ship, other)
	if Core.ships[ship] == nil then return end
	if other:isa('CargoBody') then return end
	Core.ships[ship].ts_error = "collided"

	if other:isa('Ship') and other:IsPlayer() then
		onShipHit(ship, other)
		return
	end
end
Event.Register("onShipCollided", onShipCollided)

local onShipDestroyed = function (ship, attacker)
	if attacker == nil then return end-- XX
	if Core.ships[ship] ~= nil then
		local trader = Core.ships[ship]
		Core.log:add(ship, 'Destroyed by '..attacker.label..', status:'..trader.status..' starport:'..(trader.starport and trader.starport.label or 'N/A'))
		Core.ships[ship] = nil
		-- XXX consider spawning some CargoBodies if killed by a ship
	elseif Core.attackers.check(attacker) then
		for t_ship, trader in pairs(Core.ships) do
			if trader.attacker and trader.attacker.ref == ship then
				trader.attacker = nil
				if trader.status == 'fleeing' then
					-- had not reached starport yet
					trader['status'] = 'inbound'
					trader.ts_error = "attack!"
				elseif trader.status == 'cowering' then
					-- already reached starport and docked
					-- we need to do everything that the ship does after docking
					onShipDocked(t_ship, trader.starport)
				end
				return
			end
		end
	end
end
Event.Register("onShipDestroyed", onShipDestroyed)

local onShipOutOfFuel = function (ship)
	if not Core.ships[ship] then return end
	-- we don't want to bother yet
	Core.ships[ship] = nil
	ship:Explode()
end
Event.Register("onShipOutOfFuel", onShipOutOfFuel)

local onGameEnd = function ()
	-- drop the references for our data so Lua can free them
	-- and so we can start fresh if the player starts another game
	Core.ships, Core.params = nil, nil
	Core.log:clear()
end
Event.Register("onGameEnd", onGameEnd)
