-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local SpaceStation = import_core("SpaceStation")
local Event = import("Event")
local Space = import("Space")
local utils = import("utils")
local ShipDef = import("ShipDef")
local Engine = import("Engine")
local Timer = import("Timer")
local Game = import("Game")

SpaceStation.shipsOnSale = {}

local groundShips = utils.build_array(utils.filter(function (k,def) return def.tag == "SHIP" and def.equipSlotCapacity.ATMOSHIELD > 0 end, pairs(ShipDef)))
local spaceShips  = utils.build_array(utils.filter(function (k,def) return def.tag == "SHIP" end, pairs(ShipDef)))

local function updateShipsOnSale (station)
	if not SpaceStation.shipsOnSale[station] then SpaceStation.shipsOnSale[station] = {} end
	local shipsOnSale = SpaceStation.shipsOnSale[station]

	local toAdd, toRemove = 0, 0
	if #shipsOnSale == 0 then
		toAdd = Engine.rand:Integer(20)
	elseif Engine.rand:Integer(2) > 0 then
		toAdd = 1
	elseif #shipsOnSale > 0 then
		toRemove = 1
	else
		return
	end

	if toAdd > 0 then
		local avail = station.type == "STARPORT_SURFACE" and groundShips or spaceShips
		for i=1,toAdd do
			table.insert(shipsOnSale, avail[Engine.rand:Integer(1,#avail)])
		end
	end

	if toRemove > 0 then
		table.remove(shipsOnSale, Engine.rand:Integer(1,#shipsOnSale))
	end

	Event.Queue("onShipMarketUpdate", station, shipsOnSale)
end

local function updateSystem ()
	local stations = Space.GetBodies(function (b) return b.superType == "STARPORT" end)
	for i=1,#stations do updateShipsOnSale(stations[i]) end
end
local function destroySystem ()
	SpaceStation.shipsOnSale = {}
end

Event.Register("onGameStart", function ()
	updateSystem()
	Timer:CallEvery(3600, updateSystem)
end)
Event.Register("onEnterSystem", function (ship)
	if ship ~= Game.player then return end
	updateSystem()
end)

Event.Register("onLeaveSystem", function (ship)
	if ship ~= Game.player then return end
	destroySystem()
end)
Event.Register("onGameEnd", destroySystem)

return SpaceStation
