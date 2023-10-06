-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Game = require 'Game'
local Space = require 'Space'
local Event = require 'Event'
local Serializer = require 'Serializer'
local ShipDef = require 'ShipDef'
local Ship = require 'Ship'
local utils = require 'utils'

local loaded

local spawnShips = function ()
	local population = Game.system.population

	if population == 0 then
		return
	end

	local stations = utils.filter_array(Space.GetBodies("SpaceStation"), function (body) return not body.isGroundStation end)
	if #stations == 0 then
		return
	end

	local shipdefs = utils.build_array(utils.filter(function (k,def) return def.tag == 'STATIC_SHIP' end, pairs(ShipDef)))
	if #shipdefs == 0 then return end

	-- one ship per three billion, min 1, max 2*num of stations
	local num_bulk_ships = math.min(#stations*2, math.floor((math.ceil(population)+2)/3))

	for i=1, num_bulk_ships do
		local station = stations[Engine.rand:Integer(1,#stations)]
		local ship = Space.SpawnShipParked(shipdefs[Engine.rand:Integer(1,#shipdefs)].id, station)
		ship:SetLabel(Ship.MakeRandomLabel())
	end
end

local onEnterSystem = function (player)
	if not player:IsPlayer() then return end

	spawnShips()
end

local onGameStart = function ()
	if loaded == nil then
		spawnShips()
	end
	loaded = nil
end

local serialize = function ()
	return true
end

local unserialize = function (data)
	loaded = true
end

Event.Register("onEnterSystem", onEnterSystem)
Event.Register("onGameStart", onGameStart)

Serializer:Register("BulkShips", serialize, unserialize)
