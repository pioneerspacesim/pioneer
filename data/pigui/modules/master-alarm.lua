-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local gameView = require 'pigui.views.game'

local ui = require 'pigui'
local pionillium = ui.fonts.pionillium
local colors = ui.theme.colors
local icons = ui.theme.icons

local lui = require 'Lang'.GetResource("ui-core")

local Vector2 = _G.Vector2

local alreadyAlertedTemp = false
local alreadyAlertedFuel = false
local alreadyAlertedPres = false
local alreadyAlertedDescent = false
local alreadyAlertedImpact = false

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
		local pressure = frame:GetAtmosphericState(Game.player)
		if pressure and pressure > 9 and not alreadyAlertedPres then
			ui.playSfx("alarm_generic1", 1.0, 1.0)
			alreadyAlertedPres = true
		end
		if not pressure or pressure < 9 and alreadyAlertedPres then
			alreadyAlertedPres = false
		end
	end

	--check descent speed
	if frame and not frame:IsDynamic() and frame.superType ~= 'STARPORT' then --we only need to check descent speed if we are approaching a celestial body
		local velocity = Game.player:GetVelocityRelTo(frame)
		local position = Game.player:GetPositionRelTo(frame)
		local altitude = Game.player:GetAltitudeRelTo(frame)
		local approach_speed = position:dot(velocity) / position:length() --perpendicular to horizon
		local recover_distance = Game.player:GetDistanceToZeroV(approach_speed,"forward") --braking distance basically

		local reticuleCircleRadius = math.min(ui.screenWidth, ui.screenHeight) / 8 -- required for icon placement
		local uiTextPos = Vector2(ui.screenWidth / 2, ui.screenHeight / 3 - 10) --sits just above HUD circle
		local uiPos = ui.pointOnClock(gameView.center, reticuleCircleRadius * 1.2 , 10) --left side of the reticule, above frame info
		local iconSize = Vector2(24, 24)

		local max_accel = Game.player:GetAcceleration("forward") --max acceleration axis of the ship, which is always forward with all Pioneer ships
		local up_accel = Game.player:GetAcceleration("up") --up acceleration

		local response_time_factor = 2 --Gives reponse time to player (seconds). Presence of atmosphere makes this time longer.
		--Higher the value, earlier the warning. Too high = annoying, too low = death, probably.

		local evasion_factor = 20
		--The lower the evasion_factor value, the higher altitude the descent rate alarm changes mode. Depends on planet radius.
		--6500000 / 20 = 325000
		--For example, above Earth, alarm changes mode at 325km altitude
		--This should be about where the planet/star becomes the player's horizon instead of a distant orb in space.

		local surface_gravity = frame.path:GetSystemBody().gravity --surface gravity of the body we need to avoid
		local body_radius = frame.path:GetSystemBody().radius --radius of the body we need to avoid

		--required to calculate periapsis radius
		local player_mass = Game.player.staticMass + Game.player.fuelMassLeft
		local eccentricity_vector = ((velocity:length()^2 - surface_gravity*player_mass/position:length())*position-position:dot(velocity)*velocity)/surface_gravity*player_mass
		local eccentricity = eccentricity_vector:length()
		local semi_major_axis = 1 / ((2 / position:length()) - (velocity:length()^2 / surface_gravity*player_mass))
		local periapsis_radius = semi_major_axis * (1 - eccentricity)

		--with the following formula, alert triggers if
		--approach speed is over 25 m/s
		--periapsis is below body surface (ignoring atmosphere and terrain)
		--if the approach speed keeps increasing under planetary gravity at max possible rate (V1^2 = V0^2 + 2*a*h; V1: final speed, V0: initial speed, a:acceleration(gravity), h:altitude), player has less than "response_time_factor" to start boosting upwards to avoid a crash (atmo ignored)
		--player is closer to the body than the altitude calculated via evasion_factor
		if approach_speed < -25 and periapsis_radius < body_radius and (altitude - recover_distance) / ((approach_speed^2 + 2*surface_gravity*recover_distance)^(1/2)) < response_time_factor and body_radius/altitude > evasion_factor and Game.player:GetCurrentAICommand() ~= "CMD_DOCK" then
			alreadyAlertedImpact = false
			if Game.CurrentView() == "world" then
				ui.addStyledText(uiTextPos, ui.anchor.center, ui.anchor.top, lui.HUD_WARNING_DESCENT_RATE, colors.alertRed, pionillium.large, nil, colors.lightBlackBackground)
			end
			if not alreadyAlertedDescent then
				-- FIXME: alarm_descent is *REALLY* loud. Needs to be properly mixed.
				ui.playSfx("alarm_descent", 0.2, 0.2)
				alreadyAlertedDescent = true
			end

		--with the following formula, alert triggers if
		--braking distance is longer than distance to the body (player can not decelerate before hitting the body)
		--player is still far enough away from the body
		elseif approach_speed < -25 and recover_distance > altitude and periapsis_radius < body_radius and body_radius/altitude <= evasion_factor and (periapsis_radius + (1/2)*max_accel*(altitude / -approach_speed)^2) >= body_radius then
			alreadyAlertedDescent = false
			if Game.CurrentView() == "world" then
				ui.addIcon(uiPos, icons.impact_warning, colors.alertYellow, iconSize, ui.anchor.center, ui.anchor.center, lui.HUD_WARNING_IMPACT)
			end
			if not alreadyAlertedImpact then
				ui.playSfx("impact_chime", 1.0, 1.0)
				alreadyAlertedImpact = true
			end

		--with the following formula, alert triggers if
		--player ship's acceleration rate would not allow them to avoid a collision by simply accelerating sideways
		--exact calculations require complex integrals, this alert is accurate enough but just a tiny bit on the pessimistic side for extra safety measures
		elseif approach_speed < -25 and recover_distance > altitude and periapsis_radius < body_radius and body_radius/altitude <= evasion_factor and (periapsis_radius + (1/2)*max_accel*(altitude / -approach_speed)^2) < body_radius and Game.CurrentView() == "world" then
			ui.addIcon(uiPos, icons.impact_warning, colors.alertRed, iconSize, ui.anchor.center, ui.anchor.center, lui.HUD_WARNING_IMPACT_IMMINENT)
		else -- clean up warning status so we can warn the player the next time they are in danger
			alreadyAlertedImpact = false
			alreadyAlertedDescent = false
		end
	end

end

ui.registerModule("game", alarm)

return {}
