-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local loaded

local spawnShips = function ()
	local population = Game.system.population

	if population == 0 then
		return
	end

	local stations = Space.GetBodies(function (body) return body:isa("SpaceStation") end)
	if #stations == 0 then
		return
	end

	local shipdefs = build_array(filter(function (k,def) return def.tag == 'STATIC_SHIP' end, pairs(ShipDef)))
	if #shipdefs == 0 then return end

	-- one ship per three billion, min 1, max 2*num of stations
	local num_bulk_ships = #stations*2

	for i=1, num_bulk_ships do
	local station = stations[Engine.rand:Integer(1,#stations)]
		Space.SpawnShipParked(shipdefs[Engine.rand:Integer(1,#shipdefs)].id, station)
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
