-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- This module defines basic "Search and Rescue" module utility functions. 

local Engine = require 'Engine'
local Game = require 'Game'
local Space = require 'Space'
local ShipDef = require 'ShipDef'
local utils = require 'utils'

local sar_config = require 'modules.SearchRescue.sar_config'

local sar_utils = {}


function sar_utils.splitName (name)
	-- Splits the supplied name into first and last name and returns a table of both separately.
	-- Idea from http://stackoverflow.com/questions/2779700/lua-split-into-words.
	local names = {}
	for word in name:gmatch("%w+") do table.insert(names, word) end
	return names
end

function sar_utils.decToDegMinSec (coord_orig)
	-- Converts geographic coordinates from decimal to degree/minutes/seconds format
	-- and returns a string.
	local coord = math.abs(coord_orig)
	local degrees = math.floor(coord)
	local minutes = math.floor(60*(coord - degrees))
	local seconds = math.floor(3600 * ((coord - degrees) - minutes / 60))
	if coord_orig < 0 then degrees = degrees * -1 end
	local str = string.format("%i° %i' %i\"", degrees, minutes, seconds)
	return str
end

function sar_utils.findNearbyStations (vacuum, body)
	-- Return a list with stations within this system sorted by distance from supplied body (ascending). If vacuum is set to true
	-- then only return orbital stations

	-- get station bodies within current system depending on vacuum variable
	local nearbystations_raw
	if vacuum == true then
		nearbystations_raw = utils.filter_array(Space.GetBodies("SpaceStation"), function (body)
			return body.type == 'STARPORT_ORBITAL' or (not body.path:GetSystemBody().parent.hasAtmosphere)
		end)
	else
		nearbystations_raw = Space.GetBodies("SpaceStation")
	end

	-- determine distance to body
	local nearbystations_dist = {}
	for _,station in pairs(nearbystations_raw) do
		if station ~= body then
			local dist = body:DistanceTo(station)
			table.insert(nearbystations_dist, {station, dist})
		end
	end

	-- sort stations by distance to body (ascending)
	local nearbystations = {}
	table.sort(nearbystations_dist, function (a,b) return a[2] < b[2] end)
	for _,data in ipairs(nearbystations_dist) do
		table.insert(nearbystations, data[1].path)
	end

	return nearbystations
end

function sar_utils.findClosestPlanets ()
	-- Return dictionary of stations with the corresponding rocky planets they are closer
	-- to than any other station. Planets with a station are excluded.

	-- get rocky planets
	local rockyplanets = {}
	for _,path in pairs(Game.system:GetBodyPaths()) do
		local sbody = path:GetSystemBody()
		if sbody.superType == "ROCKY_PLANET" then
			table.insert(rockyplanets, Space.GetBody(sbody.index))
		end
	end

	-- get planets with stations and remove from planet list
	local ground_stations = utils.filter_array(Space.GetBodies("SpaceStation"), function (body) return body.type == 'STARPORT_SURFACE' end)
	for _,ground_station in pairs(ground_stations) do
		for i=#rockyplanets, 1, -1 do
			if rockyplanets[i] == Space.GetBody(ground_station.path:GetSystemBody().parent.index) then
				table.remove(rockyplanets, i)
				break
			end
		end
	end

	-- create dictionary of stations
	local closestplanets = {}
	local stations = Space.GetBodies("SpaceStation")
	for _,station in pairs(stations) do closestplanets[station] = {} end

	-- pick closest planets to stations
	for _,planet in pairs(rockyplanets) do
		local nearest_station = planet:FindNearestTo("SPACESTATION")
		table.insert(closestplanets[nearest_station], planet.path)
	end

	return closestplanets
end

function sar_utils.findNearbySystems (with_stations)
	-- Return list of systems within max_mission_dist distance and sorted by distance from player system (ascending).

	-- get systems (either inhabited or not - depending on variable with_stations)
	local nearbysystems_raw
	if with_stations == true then
		nearbysystems_raw = Game.system:GetNearbySystems(sar_config.max_mission_dist, function (s) return #s:GetStationPaths() > 0 end)
	else
		nearbysystems_raw = Game.system:GetNearbySystems(sar_config.max_mission_dist, function (s) return #s:GetStationPaths() == 0 end)
	end

	-- determine distance to player system
	local nearbysystems_dist = {}
	for _,system in pairs(nearbysystems_raw) do
		local dist = Game.system:DistanceTo(system)
		table.insert(nearbysystems_dist, {system, dist})
	end

	-- sort systems by distance to player system (ascending)
	local nearbysystems = {}
	table.sort(nearbysystems_dist, function (a,b) return a[2] < b[2] end)
	for _,data in ipairs(nearbysystems_dist) do
		table.insert(nearbysystems, data[1].path)
	end
	return nearbysystems
end

function sar_utils.randomPlanet (system)
	-- Return random planet located in the provided system.
	local planets = {}
	local paths = system:GetBodyPaths()
	for _,path in pairs(paths) do
		local sbody = path:GetSystemBody()
		local supertype = sbody.superType
		if supertype == "ROCKY_PLANET" or supertype == "GAS_GIANT" then
			table.insert(planets, path)
		end
	end
	if #planets > 0 then return planets[Engine.rand:Integer(1,#planets)] else return nil end
end

function sar_utils.randomLatLong (station)
	-- Provide a set of random latitude and longitude coordinates for ship placement that are:
	-- (a) random, within max_close_dist from starting base if base is provided, or
	-- (b) completely random.
	local lat, long, dist

	-- calc new lat/lon based on distance and bearing
	-- formulas taken from http://www.movable-type.co.uk/scripts/latlong.html
	if station then
		local old_lat, old_long = station:GetGroundPosition()
		local planet_radius = station.path:GetSystemBody().parent.radius / 1000
		local bearing = math.rad(Engine.rand:Number(0,360))
		dist = Engine.rand:Integer(1,sar_config.max_close_dist)  -- min distance is 1 km
		lat = math.asin(math.sin(old_lat) * math.cos(dist/planet_radius)
			+ math.cos(old_lat) * math.sin(dist/planet_radius) * math.cos(bearing))
		long = old_long + math.atan2(
			math.sin(bearing) * math.sin(dist/planet_radius) * math.cos(old_lat),
			math.cos(dist/planet_radius) - math.sin(old_lat) * math.sin(lat))
		dist = dist * 1000  -- convert to m for downstream consistency
	else
		lat = Engine.rand:Number(-90,90)
		lat = math.rad(lat)
		long = Engine.rand:Number(-180,180)
		long = math.rad(long)
	end
	return lat, long, dist
end

function sar_utils.crewPresent (ship)
	-- Check if any crew is present on the ship.
	if ship:CrewNumber() > 0 then
		return true
	else
		return false
	end
end

function sar_utils.passengersPresent (ship)
	-- Check if any passengers are present on the ship.
	return Passengers.CountOccupiedBerths(ship) > 0
end

function sar_utils.passengerSpace (ship)
	-- Check if the ship has space for passengers.
	return Passengers.CountFreeBerths(ship) > 0
end

function sar_utils.cargoPresent (ship, item)
	-- Check if this cargo item is present on the ship.
	return ship:GetComponent('CargoManager'):CountCommodity(item) > 0
end

function sar_utils.cargoSpace (ship)
	-- Check if the ship has space for additional cargo.
	return ship:GetComponent('CargoManager'):GetFreeSpace() > 0
end

function sar_utils.addCrew (ship, crew_member)
	-- Add a crew member to the supplied ship.
	if ship:CrewNumber() == ship.maxCrew then return end
	if not crew_member then
		crew_member = Character.New()
	end
	ship:Enroll(crew_member)
end

function sar_utils.removeCrew (ship)
	-- Remove a crew member from the supplied ship.
	if ship:CrewNumber() == 0 then return end
	local crew_member
	for member in ship:EachCrewMember() do  -- only way to get a single crew member?
		crew_member = member
		break
	end
	ship:Dismiss(crew_member)
	return crew_member
end

function sar_utils.addPassenger (ship, passenger)
	-- Add a passenger to the supplied ship.
	if not passengerSpace(ship) then return end
	Passengers.EmbarkPassenger(ship, passenger)
end

function sar_utils.removePassenger (ship, passenger)
	-- Remove a passenger from the supplied ship.
	if not passengersPresent(ship) then return end
	Passengers.DisembarkPassenger(ship, passenger)
end

function sar_utils.addCargo (ship, item)
	-- Add a ton of the supplied cargo item to the ship.
	ship:GetComponent('CargoManager'):AddCommodity(item, 1)
end

function sar_utils.removeCargo (ship, item)
	-- Remove a ton of the supplied cargo item from the ship.
	ship:GetComponent('CargoManager'):RemoveCommodity(item, 1)
end


return sar_utils
