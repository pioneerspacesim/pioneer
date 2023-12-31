-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Lang = require 'Lang'
local Game = require 'Game'
local Space = require 'Space'
local Comms = require 'Comms'
local Event = require 'Event'
local Legal = require 'Legal'
local Serializer = require 'Serializer'
local Equipment = require 'Equipment'
local ShipDef = require 'ShipDef'
local Timer = require 'Timer'
local Commodities = require 'Commodities'

local l = Lang.GetResource("module-policepatrol")
local l_ui_core = Lang.GetResource("ui-core")

-- Fine at which police will hunt down outlaw player
-- This is a copy from CrimeTracking.lua.
local maxFineTolerated = 300

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

local attackShip = function (ship)
	for i = 1, #patrol do
		piracy = true
		patrol[i]:AIKill(ship)
	end
end

local onShipDestroyed = function (ship, attacker)
	if ship:IsPlayer() or attacker == nil then return end

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

local onShipHit = function (ship, attacker)
	if attacker == nil then return end
	if not attacker:isa('Ship') then return end

	-- Support all police, not only the patrol
	local police = ShipDef[Game.system.faction.policeShip]
	if ship.shipId == police.id and attacker.shipId ~= police.id then
		attackShip(attacker)
	end
end

local onShipFiring = function (ship)
	if piracy then return end

	local police = ShipDef[Game.system.faction.policeShip]
	if ship.shipId ~= police.id then
		for i = 1, #patrol do
			if ship:DistanceTo(patrol[i]) <= 4000000 then
				Comms.ImportantMessage(string.interp(l_ui_core.X_CANNOT_BE_TOLERATED_HERE, { crime = l_ui_core.PIRACY }, patrol[i].label))
				attackShip(ship)
				break
			end
		end
	end
end

local onAICompleted = function (ship, ai_error)
	if not piracy or ai_error ~= 'NONE' then return end

	for i = 1, #patrol do
		if patrol[i] == ship then
			-- Ships which act in self-defence have 5 seconds to cease fire
			Timer:CallAt(Game.time + 5, function ()
				piracy = false
			end)
			break
		end
	end
end

local onJettison = function (ship, cargo)
	if not ship:IsPlayer() or Game.system:IsCommodityLegal(cargo.name) then return end

	if #patrol > 0 and showMercy then
		local manifest = ship:GetComponent('CargoManager').commodities
		if not hasIllegalGoods(manifest) then
			Comms.ImportantMessage(l.TAKE_A_HIKE, patrol[1].label)
			piracy = false
			for i = 1, #patrol do
				patrol[i]:CancelAI()
			end
		end
	end
end

local onEnterSystem = function (player)
	if not player:IsPlayer() then return end

	if not hasIllegalGoods(Commodities) then return end

	local system = Game.system
	if (1 - system.lawlessness) < Engine.rand:Number(4) then return end

	local crimes, fine = player:GetCrimeOutstanding()
	local ship
	local shipdef = ShipDef[system.faction.policeShip]
	local n = 1 + math.floor((1 - system.lawlessness) * (system.population / 3))
	for i = 1, n do
		ship = Space.SpawnShipNear(shipdef.id, player, 50, 100)
		ship:SetLabel(l_ui_core.POLICE)
		ship:AddEquip(Equipment.laser.pulsecannon_1mw)
		table.insert(patrol, ship)
	end

	if Engine.rand:Number(1) < system.lawlessness then
		-- You are lucky. They are busy, eating donuts ;-)
		Comms.ImportantMessage(string.interp(l["RESPECT_THE_LAW_" .. Engine.rand:Integer(1, getNumberOfFlavours("RESPECT_THE_LAW"))], { system = system.name }), ship.label)
	else
		if fine > maxFineTolerated then
			Comms.ImportantMessage(string.interp(l["OUTLAW_DETECTED_" .. Engine.rand:Integer(1, getNumberOfFlavours("OUTLAW_DETECTED"))], { ship_label = player.label }, ship.label))
			showMercy = false
			attackShip(player)
		else
			Comms.ImportantMessage(l["INITIATE_CARGO_SCAN_" .. Engine.rand:Integer(1, getNumberOfFlavours("INITIATE_CARGO_SCAN"))], ship.label)
			Timer:CallAt(Game.time + Engine.rand:Integer(3, 9), function ()
				if not Game.system then return end -- Shut up when the player is already in hyperspace

				local manifest = player:GetComponent('CargoManager').commodities
				if hasIllegalGoods(manifest) then
					Comms.ImportantMessage(l.ILLEGAL_GOODS_DETECTED, ship.label)
					attackShip(player)
					Comms.ImportantMessage(l["POLICE_TAUNT_" .. Engine.rand:Integer(1, getNumberOfFlavours("POLICE_TAUNT"))], ship.label)
				else
					Comms.ImportantMessage(l.NOTHING_DETECTED, ship.label)
				end
			end)
		end
	end
end

local onLeaveSystem = function (ship)
	if ship:IsPlayer() then
		patrol = {}
		showMercy = true
		piracy = false
	end
end


local loaded_data

local onGameStart = function ()
	if loaded_data then
		patrol = loaded_data.patrol
		showMercy = loaded_data.showMercy
		piracy = loaded_data.piracy
		loaded_data = nil
	end
end

local onGameEnd = function ()
	patrol = {}
	showMercy = true
	piracy = false
end

local serialize = function ()
	return { patrol = patrol, showMercy = showMercy, piracy = piracy }
end

local unserialize = function (data)
	loaded_data = data
end

Event.Register("onEnterSystem", onEnterSystem)
Event.Register("onLeaveSystem", onLeaveSystem)
Event.Register("onShipDestroyed", onShipDestroyed)
Event.Register("onShipHit", onShipHit)
Event.Register("onShipFiring", onShipFiring)
Event.Register("onAICompleted", onAICompleted)
Event.Register("onJettison", onJettison)
Event.Register("onGameStart", onGameStart)
Event.Register("onGameEnd", onGameEnd)

Serializer:Register("PolicePatrol", serialize, unserialize)
