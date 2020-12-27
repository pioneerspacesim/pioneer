-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Lang = require 'Lang'
local Game = require 'Game'
local Space = require 'Space'
local cargo = require 'Commodities'
local Comms = require 'Comms'
local Event = require 'Event'
local Legal = require 'Legal'
local Serializer = require 'Serializer'
local Equipment = require 'Equipment'
local ShipDef = require 'ShipDef'
local Timer = require 'Timer'

local l_rondel = Lang.GetResource("module-rondel")
local l_ui_core = Lang.GetResource("ui-core")

local patrol = {}
local shipFiring = false
local jetissionedCargo = false

local attackShip = function (ship)
	for i = 1, #patrol do
		patrol[i]:AIKill(ship)
	end
end

local onShipDestroyed = function (ship, attacker)
	if ship:IsPlayer() or attacker == nil or Game.system.name ~= "Rondel" then return end

	for i = 1, #patrol do
		if patrol[i] == ship then
			table.remove(patrol, i)
			if #patrol > 1 then
				Comms.ImportantMessage(l_rondel.DEFENSE_CRAFT_ELIMINATED, patrol[1].label)
			end
			break
		elseif patrol[i] == attacker then
			Comms.ImportantMessage(l_rondel.INTERCEPTION_SUCCESSFUL, attacker.label)
			break
		end
	end
end

local onShipFiring = function (ship)
	if Game.system.name ~= "Rondel" then return end

	local police = ShipDef[Game.system.faction.policeShip]
	if ship.shipId ~= police.id then
		for i = 1, #patrol do
			if ship:DistanceTo(patrol[i]) <= 4000000 and not shipFiring then
				shipFiring = true
				Comms.ImportantMessage(string.interp(l_rondel.WEAPONS_EVASIVE_ACTION, patrol[1].label))
				attackShip(ship)
				break
			end
		end
	end
end

local onJettison = function (ship, cargo)
	if Game.system.name ~= "Rondel" then return end
	if not jetissionedCargo then
		Comms.ImportantMessage(l_rondel.JETTISON_DEFENSIVE_PRECAUTION, patrol[1].label)
		jetissionedCargo = true
	end
	attackShip(ship)
end

local onEnterSystem = function (player)
	if not player:IsPlayer() then return end

	local system = Game.system
	if system.name ~= "Rondel" then return end

	local tolerance = 1
	local hyperdrive = Game.player:GetEquip('engine',1)
	if hyperdrive.fuel == cargo.military_fuel then
		tolerance = 0.5
	end

	local ship
	local shipdef = ShipDef[system.faction.policeShip]
	for i = 1, 7 do
		ship = Space.SpawnShipNear(shipdef.id, player, 50, 100)
		ship:SetLabel(l_ui_core.POLICE)
		ship:AddEquip(Equipment.laser.pulsecannon_2mw)
		table.insert(patrol, ship)
	end

	Game.SetTimeAcceleration("1x")
	Timer:CallAt(Game.time + 2, function ()
		Comms.ImportantMessage(string.interp(l_rondel.RONDEL_RESTRICTED_ZONE, {seconds = tostring(120*tolerance), playerShipLabel = Game.player:GetLabel()}), ship.label)
	end)

	Timer:CallAt(Game.time + 120*tolerance, function()
		attackShip(player)
		Comms.ImportantMessage(l_rondel.HOSTILE_ACTION_REPORTED, ship.label)
	end)
end

local onLeaveSystem = function (ship)
	if ship:IsPlayer() then
		shipFiring = false
		jetissionedCargo = false
		patrol = {}
	end
end

local loaded_data

local onGameStart = function ()
	shipFiring = false
	jetissionedCargo = false
	if loaded_data then
		patrol = loaded_data.patrol
		loaded_data = nil
	end
end

local onGameEnd = function ()
	shipFiring = false
	jetissionedCargo = false
	patrol = {}
end

local serialize = function ()
	return { patrol = patrol }
end

local unserialize = function (data)
	loaded_data = data
end

Event.Register("onEnterSystem", onEnterSystem)
Event.Register("onLeaveSystem", onLeaveSystem)
Event.Register("onShipDestroyed", onShipDestroyed)
Event.Register("onShipFiring", onShipFiring)
Event.Register("onJettison", onJettison)
Event.Register("onGameStart", onGameStart)
Event.Register("onGameEnd", onGameEnd)

Serializer:Register("Rondel", serialize, unserialize)
