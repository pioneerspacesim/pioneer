-- Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Lang = require 'Lang'
local Game = require 'Game'
local Space = require 'Space'
local Comms = require 'Comms'
local Event = require 'Event'
local Serializer = require 'Serializer'
local Timer = require 'Timer'
local Commodities = require 'Commodities'
local PlayerState = require 'PlayerState'
local ui = require 'pigui'

local MissionUtils = require 'modules.MissionUtils'
local ShipBuilder  = require 'modules.MissionUtils.ShipBuilder'

local l = Lang.GetResource("module-policepatrol")
local l_ui_core = Lang.GetResource("ui-core")

-- Fine at which police will hunt down outlaw player
-- This is a copy from CrimeTracking.lua.
local maxFineTolerated = 5500
--   The distance, in meters, at which the police patrol upholds the law
local lawEnforcedRange = 4000000

local getNumberOfFlavours = function (str)
	local num = 1

	while l:get(str .. "_" .. num) do
		num = num + 1
	end
	return num - 1
end

local hasIllegalGoods = function (cargo)
	local illegal = false

	for name in pairs(cargo) do
		if not Game.system:IsCommodityLegal(name) then
			illegal = true
			break
		end
	end
	return illegal
end

local patrol = {}
local showMercy = true
local piracy = false
local target = nil

local attackShip = function (ship)
	for i = 1, #patrol do
		patrol[i]:AIKill(ship)
	end
	target = ship
end

local onShipDestroyed = function (ship, attacker)
	if ship:IsPlayer() or attacker == nil then return end

	if ship == target then
		target = nil
	else
		for i = 1, #patrol do
			if patrol[i] == ship then
				table.remove(patrol, i)
				if attacker:isa("Ship") and attacker:IsPlayer() then
					showMercy = false
				end
				break
			elseif patrol[i] == attacker then
				Comms.ImportantMessage(l["TARGET_DESTROYED_" .. Engine.rand:Integer(1, getNumberOfFlavours("TARGET_DESTROYED"))], attacker.label)
				break
			end
		end
	end
end

local onShipHit = function (ship, attacker)
	if attacker == nil then return end
	if not attacker:isa('Ship') then return end

	-- Support all police, not only the patrol
	-- TODO: this should use a property set on the ship rather than checking for the shipid
	-- (what happens if the player is committing piracy in a police hull?)
	local policeId = Game.system.faction.policeShip
	if ship.shipId == policeId and attacker.shipId ~= policeId then
		piracy = true
		attackShip(attacker)
	end
end

local onShipFiring = function (ship)
	if piracy or target then return end

	local policeId = Game.system.faction.policeShip
	if ship.shipId ~= policeId then
		for i = 1, #patrol do
			if ship:DistanceTo(patrol[i]) <= lawEnforcedRange then
				Comms.ImportantMessage(string.interp(l_ui_core.X_CANNOT_BE_TOLERATED_HERE, { crime = l_ui_core.UNLAWFUL_WEAPONS_DISCHARGE }), patrol[i].label)
				attackShip(ship)
				break
			end
		end
	end
end

local onJettison = function (ship, cargo)
	if not ship:IsPlayer() or Game.system:IsCommodityLegal(cargo.name) then return end

	if #patrol > 0 and showMercy then
		local manifest = ship:GetComponent('CargoManager').commodities
		if not hasIllegalGoods(manifest) then
			Comms.ImportantMessage(l.TAKE_A_HIKE, patrol[1].label)
			for i = 1, #patrol do
				patrol[i]:CancelAI()
			end
			target = nil
		end
	end
end

---@param player Player
local onEnterSystem = function (player)
	if not hasIllegalGoods(Commodities) then return end

	local system = assert(Game.system)

	local cargoMgr = player:GetComponent('CargoManager')

	local illegalCount = 0
	local illegalValue = 0

	local totalCargo = player.usedCargo

	for name, count in cargoMgr:Commodities() do
		if not Game.system:IsCommodityLegal(name) then
			illegalCount = illegalCount + count
			illegalValue = illegalValue + Commodities[name].price * count
		end
	end

	local scanChance = Engine.rand:Number(4)

	if illegalCount > 0 then
		-- 25% chance of a scan in 1.0-sec worlds... let's bump that number up a bit
		-- Narratively, it's more interesting to be scanned when carrying illegal goods
		-- than when not...
		scanChance = scanChance * 0.75
	end

	if (1 - system.lawlessness) < scanChance then return end


	local _, fine = PlayerState.GetCrimeOutstanding()
	local ship
	local n = 1 + math.floor((1 - system.lawlessness) * (system.population / 3))

	local threat = 10.0 + Engine.rand:Number(10, 50) * system.lawlessness
	local template = MissionUtils.ShipTemplates.PolicePatrol:clone {
		shipId = system.faction.policeShip,
		label = system.faction.policeName
	}

	-- The scene is set for a police patrol, just wait!
	Timer:CallAt(Game.time + Engine.rand:Integer(5, 10), function ()
		if not Game.system then return end -- Shut up when the player is already in hyperspace

		for i = 1, n do
			ship = ShipBuilder.MakeShipNear(player, template, threat, 50, 100) -- "Ship detected nearby"
			assert(ship)

			table.insert(patrol, ship)
		end

		Timer:CallAt(Game.time + Engine.rand:Integer(5, 10), function () -- Oh crap, it's the police!
			if not Game.system or Game.system.path ~= system.path then return end -- Shut up if the player has jumped away

			if Engine.rand:Number(1) < system.lawlessness then
				-- You are lucky. They are busy, eating donuts ;-)
				Comms.ImportantMessage(string.interp(l["RESPECT_THE_LAW_" .. Engine.rand:Integer(1, getNumberOfFlavours("RESPECT_THE_LAW"))], { system = system.name }), ship.label)
			else
				if fine > maxFineTolerated then
					Comms.ImportantMessage(string.interp(l["OUTLAW_DETECTED_" .. Engine.rand:Integer(1, getNumberOfFlavours("OUTLAW_DETECTED"))], { ship_label = player.label }), ship.label)
					showMercy = false
					attackShip(player)

					return
				end

				Comms.ImportantMessage(l["INITIATE_CARGO_SCAN_" .. Engine.rand:Integer(1, getNumberOfFlavours("INITIATE_CARGO_SCAN"))], ship.label)

				Timer:CallAt(Game.time + Engine.rand:Integer(3, 9), function ()

					if not Game.system or Game.system.path ~= system.path then return end -- Shut up when the player is already in hyperspace

					if player:hasprop("cargo_shield_cap") then
						local hiddenGoods = math.min(illegalCount, player["cargo_shield_cap"])
						illegalCount = illegalCount - hiddenGoods
						totalCargo = totalCargo - hiddenGoods
					end

					local proportion = illegalCount / math.max(totalCargo, 1)
					local detection_chance = math.min(0.45 + proportion, 1.0)

					if illegalCount > 0 and Engine.rand:Number() < detection_chance then
						Comms.ImportantMessage(l.ILLEGAL_GOODS_DETECTED, ship.label)

						-- $5000 in fines for every $5000 in goods in a high-sec system
						-- This includes goods in shielded holds for simplicity (they look closer after finding evidence of wrongdoing...)
						local newfine = Legal:fine("TRADING_ILLEGAL_GOODS", Game.system.lawlessness)
						newfine = (newfine * illegalValue / 5000) * Engine.rand:Normal(1.0, 0.2)

						PlayerState.AddCrime("TRADING_ILLEGAL_GOODS", newfine)

						if fine + newfine > maxFineTolerated then
							attackShip(player)
							Comms.ImportantMessage(l["POLICE_TAUNT_" .. Engine.rand:Integer(1, getNumberOfFlavours("POLICE_TAUNT"))], ship.label)

							return
						end
					else
						-- Got away clean... this time.
						Comms.ImportantMessage(l.NOTHING_DETECTED, ship.label)
					end

					if fine > 100 then
						local message = l["FINES_INTRO_" .. Engine.rand:Integer(1, getNumberOfFlavours("FINES_INTRO"))]
						message = message .. " " .. l["FINES_MESSAGE_" .. Engine.rand:Integer(1, getNumberOfFlavours("FINES_MESSAGE"))]
						if fine > 1000 then
							message = message .. " " .. l["FINES_ADMONISHING_HARSH_" .. Engine.rand:Integer(1, getNumberOfFlavours("FINES_ADMONISHING_HARSH"))]
						else
							message = message .. " " .. l["FINES_ADMONISHING_" .. Engine.rand:Integer(1, getNumberOfFlavours("FINES_ADMONISHING"))]
						end
						local policeforce = Game.system.faction.policeName
						local ship_label = player.label
						Comms.ImportantMessage(string.interp(message, {policeforce = policeforce, ship_label = ship_label, fine = ui.Format.Money(fine)}), ship.label)
					end
				end)
			end
		end)
	end)

	local policeId = system.faction.policeShip
	Timer:CallEvery(15, function ()
		if not Game.system or #patrol == 0 then return true end
		if target then return false end

		local ships = Space.GetBodiesNear(patrol[1], lawEnforcedRange, "Ship")
		for _, enemy in ipairs(ships) do
			if enemy.shipId ~= policeId and enemy:GetCurrentAICommand() == "CMD_KILL" then
				if not piracy then
					Comms.ImportantMessage(string.interp(l_ui_core.X_CANNOT_BE_TOLERATED_HERE, { crime = l_ui_core.PIRACY }), patrol[1].label)
					Comms.ImportantMessage(string.interp(l["RESTRICTIONS_WITHDRAWN_" .. Engine.rand:Integer(1, getNumberOfFlavours("RESTRICTIONS_WITHDRAWN"))], { ship_label = player.label }), patrol[1].label)
					piracy = true
				end
				attackShip(enemy)
				break
			end
		end
		if piracy and not target then
			Comms.ImportantMessage(string.interp(l["RESTRICTIONS_ESTABLISHED_" .. Engine.rand:Integer(1, getNumberOfFlavours("RESTRICTIONS_ESTABLISHED"))], { ship_label = player.label }), patrol[1].label)
			piracy = false
		end
	end)
end

local onLeaveSystem = function (ship)
	patrol = {}
	showMercy = true
	piracy = false
	target = nil
end

local onShipDocked = function (player)
	if not player:IsPlayer() then return end
	local crimes, fine = PlayerState.GetCrimeOutstanding()
	if fine > 0 then
		Comms.ImportantMessage("[" .. string.upper(Game.system.faction.policeName) .. " - " ..
			l_ui_core.AUTOMATED_MESSAGE .. "] " .. l_ui_core.OUTSTANDING_FINES .. ": " .. ui.Format.Money(fine))
	end
end

local loaded_data

local onGameStart = function ()
	if loaded_data then
		patrol = loaded_data.patrol
		showMercy = loaded_data.showMercy
		piracy = loaded_data.piracy
		target = loaded_data.target
		loaded_data = nil
	end
end

local onGameEnd = function ()
	patrol = {}
	showMercy = true
	piracy = false
	target = nil
end

local serialize = function ()
	return { patrol = patrol, showMercy = showMercy, piracy = piracy, target = target }
end

local unserialize = function (data)
	loaded_data = data
end

Event.Register("onEnterSystem", onEnterSystem)
Event.Register("onLeaveSystem", onLeaveSystem)
Event.Register("onShipDocked", onShipDocked)
Event.Register("onShipDestroyed", onShipDestroyed)
Event.Register("onShipHit", onShipHit)
Event.Register("onShipFiring", onShipFiring)
Event.Register("onJettison", onJettison)
Event.Register("onGameStart", onGameStart)
Event.Register("onGameEnd", onGameEnd)

Serializer:Register("PolicePatrol", serialize, unserialize)
