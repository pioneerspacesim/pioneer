-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = require 'pigui'
local Engine = require 'Engine'
local Game = require 'Game'
local Vector2 = _G.Vector2

-- cache ui
local pionillium = ui.fonts.pionillium
local pionicons = ui.fonts.pionicons
local colors = ui.theme.colors
local icons = ui.theme.icons

local lc = require 'Lang'.GetResource("core")
local lui = require 'Lang'.GetResource("ui-core")

local reticuleCircleRadius = math.min(ui.screenWidth, ui.screenHeight) / 8
local reticuleCircleThickness = 2.0

local gameView = require 'pigui.views.game'

-- for modules
ui.reticuleCircleRadius = reticuleCircleRadius
ui.reticuleCircleThickness = reticuleCircleThickness

-- center of screen, set each frame by the handler
local center = nil

-- cache player each frame
local player = nil

-- this should go into HUD settings
local showNavigationalNumbers = true

-- display the pitch indicator on the right inside of the reticule circle
local function displayReticulePitch(pitch_degrees)
	local tick_length = 4
	local radius = reticuleCircleRadius - 1
	ui.lineOnClock(center, 3, tick_length, radius, colors.navigationalElements, 1)
	ui.lineOnClock(nil, 2.25, tick_length, radius, colors.navigationalElements, 1)
	ui.lineOnClock(nil, 3.75, tick_length, radius, colors.navigationalElements, 1)
	ui.lineOnClock(nil, 1.5, tick_length, radius, colors.navigationalElements, 1)
	ui.lineOnClock(nil, 4.5, tick_length, radius, colors.navigationalElements, 1)

	local xpitch = (pitch_degrees + 90) / 180
	local xpitch_h = 4.5 - xpitch * 3
	ui.lineOnClock(nil, xpitch_h, tick_length * 1.5, radius, colors.navigationalElements, 2)
end

-- display the horizon inside the reticule circle
local function displayReticuleHorizon(roll_degrees)
	-- offset inside the circle (px)
	local offset = 30
	-- width of the horizontal bar (px)
	local width = 10
	-- height of the horizontal bar (clock hours)
	local height_hrs = 0.1

	local hrs = roll_degrees / 360 * 12 + 3

	local radius = reticuleCircleRadius - offset
	-- left hook
	ui.lineOnClock(center, hrs, width, radius, colors.navigationalElements, 1)
	ui.addLine(ui.pointOnClock(nil, radius, hrs),
		ui.pointOnClock(nil, radius, hrs + height_hrs),
		colors.navigationalElements, 1)
	ui.lineOnClock(nil, -3, -width/2, radius, colors.navigationalElements, 1)
	-- right hook
	ui.lineOnClock(nil, hrs + 6, width, radius, colors.navigationalElements, 1)
	ui.addLine(ui.pointOnClock(nil, radius, hrs + 6),
		ui.pointOnClock(nil, radius, hrs + 6 - height_hrs),
		colors.navigationalElements, 1)
	ui.lineOnClock(nil, 3, -width/2, radius, colors.navigationalElements, 1)
end

-- display the compass at the top of the reticule circle
local function displayReticuleCompass(heading)
	-- labelled points on the compass
	local directions = {
		[0] = lc.COMPASS_N, [45] = lc.COMPASS_NE, [90] = lc.COMPASS_E, [135] = lc.COMPASS_SE,
		[180] = lc.COMPASS_S, [225] = lc.COMPASS_SW, [270] = lc.COMPASS_W, [315] = lc.COMPASS_NW
	}
	local function clamp(x)
		if x < 0 then
			return clamp(x + 360)
		elseif x >= 360 then
			return clamp(x - 360)
		else
			return x
		end
	end
	local left = math.floor(heading - 45)
	local right = left + 90

	ui.lineOnClock(center, 0, 3, reticuleCircleRadius, colors.reticuleCircle, 1)

	local function stroke(d, p, multiple, height, thickness)
		if d % multiple == 0 then
			ui.lineOnClock(nil, 2.8 * p - 1.4, -height, reticuleCircleRadius, colors.reticuleCircle, thickness)
		end
	end

	for d=left,right do
		local p = (d - left) / 90
		stroke(d, p, 15, 3, 1)
		stroke(d, p, 45, 4, 1)
		stroke(d, p, 90, 4, 2)
		for k,v in pairs(directions) do
			if clamp(k) == clamp(d) then
				local a = ui.pointOnClock(nil, reticuleCircleRadius + 8, 3 * p - 1.5)
				ui.addStyledText(a, ui.anchor.center, ui.anchor.bottom, v, colors.navigationalElements, pionillium.tiny, "")
			end
		end
	end
end

-- display the delta-v gauges on the left side of the reticule circle
local function displayReticuleDeltaV()
	-- ratio is 1.0 for full, 0.0 for empty
	local function gauge(ratio, radius, color, thickness)
		if ratio < 0 then
			ratio = 0
		end
		-- clamp to reduce flickering
		if ratio > 0 and ratio < 0.01 then
			ratio = 0.01
		end
		if ratio > 1 then
			ratio = 1
		end
		ui.pathArcTo(center, radius + thickness / 2, ui.pi_2 + ui.pi_4, ui.pi_2 + ui.pi_4 + ui.pi_2 * ratio, 64)
		ui.pathStroke(color, false, thickness)
	end

	local offset = 3
	local thickness = 5

	local deltav_max = player:GetMaxDeltaV()
	local deltav_remaining = player:GetRemainingDeltaV()
	local dvr = deltav_remaining / deltav_max
	local deltav_maneuver = player:GetManeuverVelocity():length()
	local dvm = deltav_maneuver / deltav_max
	local deltav_current = player:GetCurrentDeltaV()
	local dvc = deltav_current / deltav_max

	-- draw full gauge in background color
	gauge(1.0, reticuleCircleRadius + offset, colors.deltaVTotal, thickness)
	-- draw remaining from bottom
	if dvr > 0 then
	  gauge(dvr, reticuleCircleRadius + offset, colors.deltaVRemaining, thickness)
	end
	-- draw maneuver thinner inside remaining
	if dvm > 0 then
	  gauge(dvm, reticuleCircleRadius + offset + thickness / 4, colors.deltaVManeuver, thickness / 2)
	end
	-- draw current beside the others
	if dvc > 0 then
	  gauge(dvc, reticuleCircleRadius + offset + thickness, colors.deltaVCurrent, thickness)
	end

	local dvr_text, dvr_unit = ui.Format.Speed(deltav_remaining)
	local uiPos = ui.pointOnClock(center, reticuleCircleRadius + 5, 7)
	ui.addFancyText(uiPos, ui.anchor.right, ui.anchor.top, {
		{ text=math.floor(dvr*100), color=colors.reticuleCircle,     font=pionillium.small, tooltip=lui.HUD_DELTA_V_PERCENT },
		{ text='% ',                color=colors.reticuleCircleDark, font=pionillium.tiny,  tooltip=lui.HUD_DELTA_V_PERCENT },
		{ text=dvr_text,            color=colors.reticuleCircle,     font=pionillium.small, tooltip=lui.HUD_DELTA_V },
		{ text=dvr_unit,            color=colors.reticuleCircleDark, font=pionillium.tiny,  tooltip=lui.HUD_DELTA_V }
	}, colors.lightBlackBackground)
end

-- if the ratio of current distance to brake distance is greater than this,
-- display the gauge in green (or whatever the theme's colour is)
-- to tell the user to flip and start braking now
local brakeNowRatio = 0.93

-- display the velocity vs. braking distance gauges on the right side of the reticule circle
local function displayReticuleBrakeGauge(ratio, ratio_secondary)
	local function gauge(ratio, radius, color, thickness)
		if ratio < 0 then
			ratio = 0
		end
		-- clamp to reduce flickering
		if ratio > 0 and ratio < 0.01 then
			ratio = 0.01
		end
		if ratio > 1 then
			ratio = 1
		end
		ui.pathArcTo(center, radius + thickness / 2, ui.pi_4, - ui.pi_4 + ui.pi_2 * (1 - ratio), 64)
		ui.pathStroke(color, false, thickness)
	end
	local thickness = 5
	local offset = 3

	if ratio <= 1 then
		gauge(1, reticuleCircleRadius + offset, colors.brakeBackground, thickness)
		local color
		if ratio > brakeNowRatio then
			color = colors.brakeNow
		else
			color = colors.brakePrimary
		end
		gauge(ratio_secondary, reticuleCircleRadius + offset, colors.brakeSecondary, thickness)
		gauge(ratio, reticuleCircleRadius + offset, color, thickness)
	else
		gauge(1, reticuleCircleRadius + offset, colors.brakeOvershoot, thickness)
		gauge(2 - math.min(ratio, 2), reticuleCircleRadius + offset, colors.brakePrimary, thickness)
	end
end

-- display heading, pitch and roll around the reticule circle
local function displayReticulePitchHorizonCompass()
	local heading, pitch, roll = Game.player:GetHeadingPitchRoll("planet")
	local pitch_degrees = (pitch / ui.twoPi * 360)
	local heading_degrees = (heading / ui.twoPi * 360)
	local roll_degrees = (roll / ui.twoPi * 360);
	local size = 12

	if showNavigationalNumbers then
		local uiPos = ui.pointOnClock(center, reticuleCircleRadius + 5, 4.7)
		ui.addStyledText(uiPos, ui.anchor.left, ui.anchor.top, math.floor(pitch_degrees + 0.5) .. "°", colors.reticuleCircle, pionillium.small, lui.HUD_CURRENT_PITCH)

		uiPos = ui.pointOnClock(center, reticuleCircleRadius + 15, 1.3)
		ui.addStyledText(uiPos, ui.anchor.left, ui.anchor.bottom, math.floor(heading_degrees + 0.5) .. "°", colors.reticuleCircle, pionillium.small, lui.HUD_CURRENT_HEADING)

		uiPos = ui.pointOnClock(center, reticuleCircleRadius + 5, 6)
		ui.addStyledText(uiPos, ui.anchor.center, ui.anchor.top, math.floor(roll_degrees + 0.5) .. "°", colors.reticuleCircle, pionillium.small, lui.HUD_CURRENT_ROLL)
	end

	displayReticulePitch(pitch_degrees)
	displayReticuleHorizon(roll_degrees)
	displayReticuleCompass(heading_degrees)
end

local reticuleTarget = "frame"

local lastNavTarget = nil
local lastCombatTarget = nil

local function updateReticuleTarget(frame, navTarget, combatTarget)
	if lastNavTarget ~= navTarget then
		reticuleTarget = "navTarget"
	end
	lastNavTarget = navTarget

	if lastCombatTarget ~= combatTarget then
		reticuleTarget = "combatTarget"
	end
	lastCombatTarget = combatTarget

	if reticuleTarget == "navTarget" then
		if not navTarget then
			reticuleTarget = combatTarget and "combatTarget" or "frame"
		end
	end

	if reticuleTarget == "combatTarget" then
		if not combatTarget then
			reticuleTarget = navTarget and "navTarget" or "frame"
		end
	end

	if not reticuleTarget then
		reticuleTarget = "frame"
	end

	if reticuleTarget == "frame" then
		if not frame then
			reticuleTarget = nil
		end
	end
end

-- show frame / target switch buttons if anything is targetted
local function displayDetailButtons(radius, navTarget, combatTarget)
	local uiPos = ui.pointOnClock(center, radius, 3.6)
	local mouse_position = ui.getMousePos()
	local size = 24
	if combatTarget or navTarget then
		local color = reticuleTarget == "frame" and colors.reticuleCircle or colors.reticuleCircleDark
		ui.addIcon(uiPos, icons.display_frame, color, Vector2(size, size), ui.anchor.left, ui.anchor.bottom, lui.HUD_SHOW_FRAME)
		if ui.isMouseClicked(0) and (mouse_position - (uiPos + Vector2(size/2, -size/2))):length() < size/2 then
			reticuleTarget = "frame"
		end
		uiPos.x = uiPos.x + size
	end
	if navTarget then
		local color = reticuleTarget == "navTarget" and colors.reticuleCircle or colors.reticuleCircleDark
		ui.addIcon(uiPos, icons.display_navtarget, color, Vector2(size, size), ui.anchor.left, ui.anchor.bottom, lui.HUD_SHOW_NAV_TARGET)
		if ui.isMouseClicked(0) and (mouse_position - (uiPos + Vector2(size/2, -size/2))):length() < size/2 then
			reticuleTarget = "navTarget"
		end
		uiPos.x = uiPos.x + size
	end
	if combatTarget then
		local color = reticuleTarget == "combatTarget" and colors.reticuleCircle or colors.reticuleCircleDark
		ui.addIcon(uiPos, icons.display_combattarget, color, Vector2(size, size), ui.anchor.left, ui.anchor.bottom, lui.HUD_SHOW_COMBAT_TARGET)
		if ui.isMouseClicked(0) and (mouse_position - (uiPos + Vector2(size/2, -size/2))):length() < size/2 then
			reticuleTarget = "combatTarget"
		end
	end
end

local function displayDetailData(target, radius, combatTarget, navTarget, colorLight, colorDark)
	local velocity = player:GetVelocityRelTo(target)
	local position = player:GetPositionRelTo(target)

	local uiPos = ui.pointOnClock(center, radius, 2.46)
	-- label of target
	local tooltip = reticuleTarget == "combatTarget" and lui.HUD_CURRENT_COMBAT_TARGET or (reticuleTarget == "navTarget" and lui.HUD_CURRENT_NAV_TARGET or lui.HUD_CURRENT_FRAME)
	local nameSize = ui.addStyledText(uiPos, ui.anchor.left, ui.anchor.baseline, target.label, colorDark, pionillium.medium, tooltip, colors.lightBlackBackground)
	if ui.isMouseHoveringRect(uiPos - Vector2(0, pionillium.medium.offset), uiPos + nameSize - Vector2(0, pionillium.medium.offset)) and ui.isMouseClicked(1) and ui.noModifierHeld() then
		ui.openDefaultRadialMenu(target)
	end
	-- current distance, relative speed
	uiPos = ui.pointOnClock(center, radius, 2.75)
	-- currently unused: local distance, distance_unit = ui.Format.Distance(player:DistanceTo(target))
	local approach_speed = position:dot(velocity) / position:length()

	local speed, speed_unit = ui.Format.Speed(approach_speed)

	ui.addFancyText(uiPos, ui.anchor.left, ui.anchor.baseline, {
		{ text=speed,      color=colorLight, font=pionillium.medium, tooltip=lui.HUD_SPEED_OF_APPROACH_TO_TARGET },
		{ text=speed_unit, color=colorDark,  font=pionillium.small,  tooltip=lui.HUD_SPEED_OF_APPROACH_TO_TARGET }
	}, colors.lightBlackBackground)

	-- current brake distance
	local brake_distance = player:GetDistanceToZeroV(velocity:length(),"forward")
	local brake_distance_retro = player:GetDistanceToZeroV(velocity:length(),"reverse")
	local altitude = player:GetAltitudeRelTo(target)
	local ratio = brake_distance / altitude
	local ratio_retro = brake_distance_retro / altitude

	speed, speed_unit = ui.Format.Speed(velocity:length())

	uiPos = ui.pointOnClock(center, radius, 3)
	local distance,unit = ui.Format.Distance(brake_distance)
	ui.addFancyText(uiPos, ui.anchor.left, ui.anchor.baseline, {
		{ text="~" .. distance, color=colorDark, font=pionillium.medium, tooltip=lui.HUD_BRAKE_DISTANCE_MAIN_THRUSTERS },
		{ text=unit,            color=colorDark, font=pionillium.small,  tooltip=lui.HUD_BRAKE_DISTANCE_MAIN_THRUSTERS }
	}, colors.lightBlackBackground)

	-- current altitude
	uiPos = ui.pointOnClock(center, radius, 3.25)
	local altitude, altitude_unit = ui.Format.Distance(altitude)
	ui.addFancyText(uiPos, ui.anchor.left, ui.anchor.baseline, {
		{ text=altitude,      color=colorLight, font=pionillium.medium, tooltip=lui.HUD_DISTANCE_TO_SURFACE_OF_TARGET },
		{ text=altitude_unit, color=colorDark,  font=pionillium.small,  tooltip=lui.HUD_DISTANCE_TO_SURFACE_OF_TARGET },
		{ text=" " .. speed,  color=colorLight, font=pionillium.medium, tooltip=lui.HUD_SPEED_RELATIVE_TO_TARGET },
		{ text=speed_unit,    color=colorDark,  font=pionillium.small,  tooltip=lui.HUD_SPEED_RELATIVE_TO_TARGET }
	}, colors.lightBlackBackground)

	-- current speed of approach
	if approach_speed < 0 then
		displayReticuleBrakeGauge(ratio, ratio_retro)
	end

	displayDetailButtons(radius, navTarget, combatTarget)
end

-- display data relative to frame left of the reticule circle
local function displayFrameData(frame, radius)
	local velocity = player:GetVelocityRelTo(frame)
	local position = player:GetPositionRelTo(frame)
	local altitude = player:GetAltitudeRelTo(frame)
	local brake_distance = player:GetDistanceToZeroV(velocity:length(),"forward")
	local altitude, altitude_unit = ui.Format.Distance(altitude)
	local approach_speed = position:dot(velocity) / position:length()
	local speed, speed_unit = ui.Format.Speed(approach_speed)
	local uiPos = ui.pointOnClock(center, radius, -2.46)
	-- label of frame
	ui.addStyledText(uiPos, ui.anchor.right, ui.anchor.baseline, frame.label, colors.frame, pionillium.medium, lui.HUD_CURRENT_FRAME, colors.lightBlackBackground)
	-- speed of approach of frame
	uiPos = ui.pointOnClock(center, radius, -2.75)
	ui.addFancyText(uiPos, ui.anchor.right, ui.anchor.baseline, {
		{ text=speed,      color=colors.frame,     font=pionillium.medium, tooltip=lui.HUD_SPEED_OF_APPROACH_TO_FRAME },
		{ text=speed_unit, color=colors.frameDark, font=pionillium.small,  tooltip=lui.HUD_SPEED_OF_APPROACH_TO_FRAME }
	}, colors.lightBlackBackground)
	-- brake distance
	uiPos = ui.pointOnClock(center, radius, -3)
	local distance,unit = ui.Format.Distance(brake_distance)
	ui.addFancyText(uiPos, ui.anchor.right, ui.anchor.baseline, {
		{ text="~" .. distance, color=colors.frame,     font=pionillium.medium, tooltip=lui.HUD_BRAKE_DISTANCE_MAIN_THRUSTERS },
		{ text=unit,            color=colors.frameDark, font=pionillium.small,  tooltip=lui.HUD_BRAKE_DISTANCE_MAIN_THRUSTERS }
	}, colors.lightBlackBackground)


	-- altitude above frame
	speed, speed_unit = ui.Format.Speed(velocity:length())
	uiPos = ui.pointOnClock(center, radius, -3.25)
	ui.addFancyText(uiPos, ui.anchor.right, ui.anchor.baseline, {
		{ text=speed,           color=colors.frame,     font=pionillium.medium, tooltip=lui.HUD_SPEED_RELATIVE_TO_TARGET },
		{ text=speed_unit,      color=colors.frameDark, font=pionillium.small,  tooltip=lui.HUD_SPEED_RELATIVE_TO_TARGET },
		{ text=' ' .. altitude, color=colors.frame,     font=pionillium.medium, tooltip=lui.HUD_DISTANCE_TO_SURFACE_OF_FRAME },
		{ text=altitude_unit,   color=colors.frameDark, font=pionillium.small,  tooltip=lui.HUD_DISTANCE_TO_SURFACE_OF_FRAME }
	}, colors.lightBlackBackground)

end

-- display current maneuver data below the reticule circle
local function displayManeuverData(radius)
	local maneuverVelocity = player:GetManeuverVelocity()
	local maneuverSpeed = maneuverVelocity:length()
	if maneuverSpeed > 0 and not (player:IsDocked() or player:IsLanded()) then
		local onscreen,position,direction = Engine.WorldSpaceToScreenSpace(maneuverVelocity)
		gameView.displayIndicator(onscreen, position, direction, icons.bullseye, colors.maneuver, true, lui.HUD_INDICATOR_MANEUVER_PROGRADE)
		local uiPos = ui.pointOnClock(center, radius, 6)
		local speed, speed_unit = ui.Format.Speed(maneuverSpeed)
		local duration = ui.Format.Duration(player:GetManeuverTime() - Game.time)
		local acceleration = player:GetAcceleration("forward")
		local burn_duration = maneuverSpeed / acceleration
		local burn_time = ui.Format.Duration(burn_duration)
		ui.addFancyText(uiPos, ui.anchor.center, ui.anchor.top, {
			{ text=duration,           color=colors.maneuver,     font=pionillium.medium, tooltip=lui.HUD_DURATION_UNTIL_MANEUVER_BURN },
			{ text="  " .. speed,      color=colors.maneuver,     font=pionillium.medium, tooltip=lui.HUD_DELTA_V_OF_MANEUVER_BURN },
			{ text=speed_unit,         color=colors.maneuverDark, font=pionillium.small,  tooltip=lui.HUD_DELTA_V_OF_MANEUVER_BURN },
			{ text="  ~" .. burn_time, color=colors.maneuver,     font=pionillium.medium, tooltip=lui.HUD_DURATION_OF_MANEUVER_BURN }
		}, colors.lightBlackBackground)
	end
end


local function displaySetSpeed(radius)
	local setSpeed = player:GetSetSpeed()
	if setSpeed ~= nil then
		local distance, unit = ui.Format.Speed(setSpeed)
		local uiPos = ui.pointOnClock(center, radius, 4.0)
		local target = player:GetSetSpeedTarget()
		if target then
			local color = colors.reticuleCircle
			local colorDark = colors.reticuleCircleDark
			ui.addFancyText(uiPos, ui.anchor.left, ui.anchor.top, {
				{ text=icons.autopilot_set_speed, color=color,     font=pionicons.medium,  tooltip="set speed" },
				{ text=distance,                  color=color,     font=pionillium.medium, tooltip="set speed" },
				{ text=unit,                      color=colorDark, font=pionillium.small,  tooltip="set speed" },
				{ text=' ' .. target.label,       color=color,     font=pionillium.medium, tooltip="set speed" }
			}, colors.lightBlackBackground)
		end
	end
end

local function displayAlertMarker()
	local alert = player:GetAlertState()
	local iconsize = Vector2(24, 24)
	if alert then
		local uiPos = ui.pointOnClock(center, reticuleCircleRadius * 1.2 , 2)
		if alert == "ship-firing" then
			ui.addIcon(uiPos, icons.alert2, colors.alertRed, iconsize, ui.anchor.center, ui.anchor.center, lc.LASER_FIRE_DETECTED)
		elseif alert == "ship-nearby" then
			ui.addIcon(uiPos, icons.alert1, colors.alertYellow, iconsize, ui.anchor.center, ui.anchor.center, lc.SHIP_DETECTED_NEARBY)
		end
	end
end

local function displayReticule()
	-- reticule circle
	ui.addCircle(center, reticuleCircleRadius, colors.reticuleCircle, ui.circleSegments(reticuleCircleRadius), reticuleCircleThickness)

	local frame = player.frameBody
	local navTarget = player:GetNavTarget()
	local combatTarget = player:GetCombatTarget()
	local radius = reticuleCircleRadius * 1.2

	updateReticuleTarget(frame, navTarget, combatTarget)

	if reticuleTarget == "frame" then
		displayDetailData(frame, radius, combatTarget, navTarget, colors.frame, colors.frameDark)
	elseif reticuleTarget == "navTarget" then
		displayDetailData(navTarget, radius, combatTarget, navTarget, colors.navTarget, colors.navTargetDark)
	elseif reticuleTarget == "combatTarget" then
		displayDetailData(combatTarget, radius, combatTarget, navTarget, colors.combatTarget, colors.combatTargetDark)
	end

	displaySetSpeed(radius)
	displayManeuverData(radius)
	displayReticulePitchHorizonCompass()
	displayReticuleDeltaV()
	displayAlertMarker()

	if frame and reticuleTarget ~= "frame" then
		displayFrameData(frame, radius)
	end
end

gameView.registerModule("reticule", {
	showInHyperspace = false,
	draw = function(dT)
		player = gameView.player
		center = gameView.center
		colors = ui.theme.colors
		displayReticule()
	end
})
