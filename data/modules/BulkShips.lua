-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local loaded
local s = { }

local spawnShips = function ()
	local population = Game.system.population

	if population == 0 then
		return
	end

	local stations = Space.GetBodies(function (body) 
		return body:isa("SpaceStation") and body.type == 'STARPORT_SURFACE'
	end)
	if #stations == 0 then
		return
	end

	local shipdefs = build_array(filter(function (k,def) return def.tag == 'STATIC_SHIP' end, pairs(ShipDef)))
	if #shipdefs == 0 then return end

	-- one ship per three billion, min 1, max 2*num of stations
	local num_bulk_ships = #stations

	for i=1, num_bulk_ships do
	local station = stations[i]
		s[i] = Space.SpawnShipParkedOffset(shipdefs[Engine.rand:Integer(1,#shipdefs)].id, station)
	end
end

local onEnterSystem = function (player)
	if not player:IsPlayer() then return end
	spawnShips()
	
	if s[1] then
	local body = s[1].frameBody
	local x,y,z = s[1]:GetPos()
	x=x+100
	Game.player:SetPos(s[1],x,y,z)
	--Game.player:AIHoldPos()
	end


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
