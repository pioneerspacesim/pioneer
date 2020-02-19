-- Copyright � 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Game = require 'Game'

local ui = require 'pigui'

local alreadyAlertedTemp = false
local alreadyAlertedFuel = false
local alreadyAlertedPres = false

local function alarm ()
	--check hull temperature
	local t = Game.player:GetHullTemperature()
	if t and t > 0.8 and not alreadyAlertedTemp then
		ui.playSfx("alarm_emer", 1.0, 1.0)
		alreadyAlertedTemp = true;
	end
	if t < 0.8 and alreadyAlertedTemp then
		alreadyAlertedTemp = false;
	end

	--check fuel level
	local remainingFuel = Game.player:GetRemainingDeltaV()
	local currentSpeed = Game.player:GetCurrentDeltaV()
	local maxDv = Game.player:GetMaxDeltaV()
	local warningRatio = remainingFuel - currentSpeed
	if warningRatio < (maxDv / 10) and not alreadyAlertedFuel then
		ui.playSfx("fuel_low", 1.0, 1.0)
		alreadyAlertedFuel = true
	end
	if warningRatio > (maxDv / 10) and alreadyAlertedFuel then
		alreadyAlertedFuel = false
	end

	--check atmospheric pressure
	local frame = Game.player.frameBody
	if frame then
		local pressure = frame:GetAtmosphericState()
		if pressure and pressure > 9 and not alreadyAlertedPres then
			ui.playSfx("alarm_generic1", 1.0, 1.0)
			alreadyAlertedPres = true
		end
		if not pressure or pressure < 9 and alreadyAlertedPres then
			alreadyAlertedPres = false
		end
	end
end

ui.registerModule("game", alarm)

return {}

