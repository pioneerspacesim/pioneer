-- Copyright © 2008-2022 Pioneer Developers. See AUTHORS.txt for details
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

return MissionUtils
