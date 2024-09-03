-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game   = require "Game"
local Engine = require "Engine"
local utils  = require "utils"

local AU = 149598000000
local AU_sqrt = math.sqrt(AU)

local Days = 24*60*60

local MissionUtils = {
	AU = AU,
	Days = Days
}

---@class MissionUtils.Calculator
---@field New fun(): MissionUtils.Calculator
local Calculator = utils.class("MissionUtils.Calculator")

MissionUtils.Calculator = Calculator

-- Default starting parameters
Calculator.baseDuration = 4 * Days
Calculator.baseReward = 0

Calculator.inSystemTime = 1 * Days
Calculator.travelTimeAU = 1.0
Calculator.typHyperTime = 7 * Days
Calculator.typHyperDist = 15

Calculator.inSystemReward = 50
Calculator.hyperspaceReward = 500

-- Setup per-mission parameters in the mission calculator
function Calculator:SetParams(args)
	self.baseDuration = args.baseDuration
	self.baseReward = args.baseReward

	self.inSystemTime = args.inSystemTime
	self.travelTimeAU = args.travelTimeAU
	self.typHyperTime = args.hyperspaceTime
	self.typHyperDist = args.hyperspaceDist

	self.inSystemReward = args.inSystemReward
	self.hyperspaceReward = args.hyperspaceReward
end

-- Calculate the expected time for a transfer between the given station and the destination
---@param station SpaceStation
---@param location SystemPath
---@param urgency number scalar between 0..1 determining how urgent the mission is
---@param random number? optional scalar between 0..1 determining the random deviation of the result
function Calculator:CalcTravelTime(station, location, urgency, random)
	local distDur

	if station.path:IsSameSystem(location) then
		local dist = math.sqrt(station:DistanceTo(location:GetSystemBody().body))
		distDur = math.max((dist / AU_sqrt) * self.travelTimeAU, self.inSystemTime)
	else
		distDur = (location:DistanceTo(Game.system) / self.typHyperDist) * self.typHyperTime
	end

	local dur = self.baseDuration + distDur
		* (1.5 - urgency)
		* (random and Engine.rand:Number(1.0 - random, 1.0 + random) or 1.0)

	return dur
end

-- Calculate the adjusted reward for a mission between the given station and the destination
---@param station SpaceStation
---@param location SystemPath
---@param urgency number scalar between 0..1 determining how urgent the mission is
---@param difficulty number? optional scalar between 0..1 determining the difficulty involved in the mission
---@param random number? optional scalar between 0..1 determining the random deviation of the result
function Calculator:CalcReward(station, location, urgency, difficulty, random)
	difficulty = difficulty or 0

	local distReward

	if station.path:IsSameSystem(location) then
		local dist = math.sqrt(station:DistanceTo(location:GetSystemBody().body))
		distReward = math.max(dist / 15000, self.inSystemReward)
	else
		distReward = (location:DistanceTo(Game.system) / self.typHyperDist) * self.hyperspaceReward
	end

	local reward = self.baseReward + distReward
		* (0.5 + urgency)
		* (1 + difficulty)
		* (random and Engine.rand:Number(1.0 - random, 1.0 + random) or 1.0)

	return reward
end

-- Calculate the distance between the station and the given system path
---@param station SpaceStation
---@param location SystemPath
---@return number distance in meters or lightyears
---@return boolean isLocal whether the distance is measured in meters or ly
function Calculator:CalcDistance(station, location)
	if station.path:IsSameSystem(location) then
		return station:DistanceTo(location:GetSystemBody().body), true
	else
		return location:DistanceTo(Game.system), false
	end
end

--=============================================================================


--
-- Function: GetNearbyStationPaths
--
-- Gets a list of stations in nearby systems that match some criteria.
--
-- Example:
--
-- >  local orbital_ports = MissionUtils.GetNearbyStationPaths(Game.system,
-- >      30, nil, function (station) return station.type == 'STARPORT_ORBITAL' end, true)
-- >
-- >  for i = 1, #orbital_ports do
-- >      local path = orbital_ports[i]
-- >      print(path, ' -- ', path:GetSystemBody().name, ' in system ', path:GetStarSystem().name)
-- >  end
--
-- Parameters:
--
--   range_ly        Range limit for nearby systems to search.
--   system_filter   [optional] function, taking a StarSystem object, used to filter systems.
--   station_filter  [optional] function, taking a SystemBody object, used to filter stations.
--   include_local   [optional] if this is true, then stations in the origin system will be included.--
--
-- Author: John Bartholomew
--
---@param system StarSystem
---@param range_ly number
---@param system_filter? fun(s: StarSystem): boolean
---@param station_filter? fun(s: SystemBody): boolean
---@param include_local? boolean
function MissionUtils.GetNearbyStationPaths(system, range_ly, system_filter, station_filter, include_local)
	local full_system_filter ---@type fun(sys: StarSystem): boolean

	if system_filter then
		full_system_filter = function(sys) return (sys.numberOfStations > 0) and system_filter(sys) end
	else
		full_system_filter = function(sys) return (sys.numberOfStations > 0) end
	end

	local function filter_and_add_stations(output_table, sys)
		local station_paths = sys:GetStationPaths()
		for j = 1, #station_paths do
			local station_path = station_paths[j]
			local station = station_path:GetSystemBody()
			if station_filter == nil or station_filter(station) then
				table.insert(output_table, station_path)
			end
		end
	end

	local nearby_systems = Game.system:GetNearbySystems(range_ly, full_system_filter)
	local nearby_stations = {}

	if include_local == true then
		filter_and_add_stations(nearby_stations, system)
	end
	for i = 1, #nearby_systems do
		filter_and_add_stations(nearby_stations, nearby_systems[i])
	end

	return nearby_stations
end

-- Function: TravelTimeLocal
--
-- Returns a standardized travel time to a local target
--
-- Example:
--
-- > travel_time = MissionUtils.TravelTimeLocal(distance)
--
-- Parameters:
--
--   distance - the distance to the target in meters
--
-- Returns:
--
--   the travel time in seconds
--
function MissionUtils.TravelTimeLocal(distance)
	return distance/AU * 2*Days
end

--
-- Function: TravelTime
--
-- Returns a standardized hyperspace travel time
--
-- Example:
--
-- > travel_time = MissionUtils.TravelTime(distance, location)
--
-- Parameters:
--
--   distance - the distance to the target system in light years
--
--   location - optional, the location in the target system
--              must be a station or planet
--
-- Returns:
--
--   the travel time in seconds
--
function MissionUtils.TravelTime(distance, location)
	local ltt

	if location then
		local sbody = location:GetSystemBody()
		-- find the primary planet orbiting the star or gravpoint to calculate the local travel time
		while sbody.parent and sbody.parent.superType ~= 'STAR' do sbody = sbody.parent end
		ltt = MissionUtils.TravelTimeLocal((sbody.periapsis+sbody.apoapsis)/2)
	else
		ltt = MissionUtils.TravelTimeLocal(AU)
	end

	return distance * 1.75*Days + ltt
end

return MissionUtils
