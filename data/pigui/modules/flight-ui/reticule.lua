-- Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = require 'pigui'
local Engine = require 'Engine'
local Event = require 'Event'
local Game = require 'Game'
local Vector2 = _G.Vector2
local bindManager = require 'bind-manager'

-- cache ui
local pionillium = ui.fonts.pionillium
local pionicons = ui.fonts.pionicons
local colors = ui.theme.colors
local icons = ui.theme.icons

local lc = require 'Lang'.GetResource("core")
local lui = require 'Lang'.GetResource("ui-core")

local reticuleCircleRadius = math.min(ui.screenWidth, ui.screenHeight) / 8
local reticuleCircleThickness = 2.0

local flightAssistIconSize = ui.theme.styles.SmallButtonSize.x
-- first we calculate the padding so that it does not turn out to be fractional
-- precise padding is needed so that the radial menu is the same for both menus
local flightAssistButtonPadding = math.ceil((32 / 18 * flightAssistIconSize - flightAssistIconSize) / 2)
local flightAssistButtonSize = flightAssistIconSize + flightAssistButtonPadding * 2

local frameSwitchStyle = ui.Style:clone({
	colors = {},
	vars = {
		ItemSpacing = Vector2(0, 0),
		FrameRounding = ui.rescaleUI(6)
	}
})

local frameDetailStyle = ui.Style:clone({
	colors = {
		Text = colors.frame
	},
	vars = {
		ItemSpacing = ui.rescaleUI(Vector2(8, 3))
	}
})

local gameView = require 'pigui.views.game'

-- for modules
ui.reticuleCircleRadius = reticuleCircleRadius
ui.reticuleCircleThickness = reticuleCircleThickness

-- center of screen, set each frame by the handler
local center = nil

-- cache player each frame
local player = Game.player ---@type Player

-- active target information to display on the flight reticle
local reticuleTarget = "frame"

-- this should go into HUD settings
local showNavigationalNumbers = true

-- to interact with actions and axes in the input system
local bindings = {
	assistRadial = bindManager.registerAction('BindFlightAssistRadial'),
	fixheadingRadial = bindManager.registerAction('BindFixheadingRadial'),
	targetRadial = bindManager.registerAction('BindTargetRadial'),
}

local function textShadowedRight(label)
	ui.addCursorPos(Vector2(-ui.calcTextSize(label).x, 0))
	ui.textShadowed(label)
end

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

	ui.withFont(pionillium.reticle_sm, function()
		local uiPos = ui.pointOnClock(center, reticuleCircleRadius + thickness * 2 + ui.getTextLineHeight(), 7.5)

		local iconSize = pionillium.reticle_md.size + ui.theme.styles.FramePadding.y * 2

		ui.setCursorPos(uiPos - Vector2(iconSize, ui.getTextLineHeight() * 0.5))
		textShadowedRight(ui.Format.Speed(deltav_remaining))

		ui.withFont(pionicons.medium.name, iconSize, function()
			ui.addTextShadowed(uiPos - Vector2(iconSize, iconSize * 0.5), colors.font, ui.get_icon_glyph(icons.deltav))
			ui.setItemTooltip(lui.HUD_DELTA_V)
		end)

		uiPos = ui.pointOnClock(center, reticuleCircleRadius + thickness * 2, 10.5)
		ui.setCursorPos(uiPos - Vector2(ui.getTextLineHeight() * 0.5))

		textShadowedRight(ui.Format.Number(dvr * 100, 2) .. "%")
		ui.setItemTooltip(lui.HUD_DELTA_V_PERCENT)
	end)
end

-- if the ratio of current distance to brake distance is greater than this,
-- display the gauge in green (or whatever the theme's colour is)
-- to tell the user to flip and start braking now
local brakeNowRatio = 0.93

-- display the velocity vs. braking distance gauges on the right side of the reticule circle
local function displayReticuleBrakeGauge(ratio_primary, ratio_secondary)
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

	if ratio_primary <= 1 then
		gauge(1, reticuleCircleRadius + offset, colors.brakeBackground, thickness)
		local color
		if ratio_primary > brakeNowRatio then
			color = colors.brakeNow
		else
			color = colors.brakePrimary
		end
		gauge(ratio_secondary, reticuleCircleRadius + offset, colors.brakeSecondary, thickness)
		gauge(ratio_primary, reticuleCircleRadius + offset, color, thickness)
	else
		gauge(1, reticuleCircleRadius + offset, colors.brakeOvershoot, thickness)
		gauge(2 - math.min(ratio_primary, 2), reticuleCircleRadius + offset, colors.brakePrimary, thickness)
	end
end

-- display heading, pitch and roll around the reticule circle
local function displayReticulePitchHorizonCompass()
	local heading, pitch, roll = Game.player:GetHeadingPitchRoll("planet")
	local pitch_degrees = (pitch / ui.twoPi * 360)
	local heading_degrees = (heading / ui.twoPi * 360)
	local roll_degrees = (roll / ui.twoPi * 360);

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

local function makeTargetData(target, inv_approach)
	local velocity = player:GetVelocityRelTo(target)
	local position = player:GetPositionRelTo(target)

	local approach_speed = math.trunc(position:normalized():dot(inv_approach and velocity or -velocity))

	return {
		ship_speed = velocity:length(),
		-- positive values indicate closure unless inv_approach is true
		-- flush negative zero to positive zero
		approach_speed = approach_speed == -0.0 and 0 or approach_speed,
		brake_distance = player:GetDistanceToZeroV(velocity:length(), "forward"),
		brake_distance_retro = player:GetDistanceToZeroV(velocity:length(), "reverse"),
		distance = player:GetAltitudeRelTo(target)
	}
end

local function displayFrameData(frame, radius, tooltip)

	local is_station = frame:isa("SpaceStation")

	local data = makeTargetData(frame, not is_station)
	local uiPos = ui.pointOnClock(center, radius, 9)

	local show_orbit = not is_station and not player.frameRotating

	ui.setCursorPos(uiPos - Vector2(0, pionillium.heading.size * (show_orbit and 4 or 3)))

	-- NOTE: because this is a right-to-left group, hover tests etc. will not work on it!
	ui.group(function()
		ui.withFont(pionillium.heading, function()
			textShadowedRight(frame.label)
		end)

		if ui.isItemClicked(1) then
			ui.openDefaultRadialMenu("game", frame, ui.getMousePos())
		end

		ui.setItemTooltip(tooltip)

		frameDetailStyle:withStyle(function()
			ui.withFont(pionillium.reticle_md, function()

				textShadowedRight(ui.Format.Speed(data.ship_speed) .. " " .. ui.get_icon_glyph(icons.prograde_thin))
				ui.setItemTooltip(lui.HUD_SPEED_RELATIVE_TO_TARGET)

				textShadowedRight(ui.Format.Speed(data.approach_speed) .. " " .. ui.get_icon_glyph(icons.normal_thin))
				ui.setItemTooltip(lui.HUD_SPEED_OF_APPROACH_TO_FRAME)

				ui.spacing()
				local dist_icon = is_station and icons.radial_out_thin or icons.altitude

				textShadowedRight(ui.Format.Distance(data.distance) .. " ".. ui.get_icon_glyph(dist_icon))
				ui.setItemTooltip(lui.HUD_DISTANCE_TO_SURFACE_OF_TARGET)

				textShadowedRight("~" .. ui.Format.Distance(data.brake_distance) .. " " .. ui.get_icon_glyph(icons.radial_in_thin))
				ui.setItemTooltip(lui.HUD_BRAKE_DISTANCE_MAIN_THRUSTERS)

				if show_orbit then

					local orbit = player:GetOrbit()

					ui.spacing()

					textShadowedRight(ui.Format.Distance(orbit.apogee) .. " " .. ui.get_icon_glyph(icons.apoapsis))
					ui.setItemTooltip(lui.HUD_ORBIT_APOAPSIS)

					textShadowedRight(ui.Format.Distance(orbit.perigee) .. " " .. ui.get_icon_glyph(icons.periapsis))
					ui.setItemTooltip(lui.HUD_ORBIT_PERIAPSIS)

				end

			end)
		end)
	end)

end

local function displayFrameLabel(label, radius, tooltip)
	local uiPos = ui.pointOnClock(center, radius, 9)

	ui.withFont(pionillium.heading, function()
		local sz = ui.calcTextSize(label)

		ui.setCursorPos(uiPos - Vector2(sz.x, sz.y * 0.5))
		ui.textShadowed(label)
	end)

	ui.setItemTooltip(tooltip)
end

local function displayTargetSwitcher(targets)
	local size = Vector2(22)
	local padding = Vector2(0)

	frameSwitchStyle:withStyle(function()
		if targets.navTarget and targets.combatTarget then
			ui.addCursorPos(Vector2(-ui.getTextLineHeight() * 0.25, 0))

			ui.withStyleColors({ Text = reticuleTarget == "navTarget" and colors.font or colors.reticuleCircleDark }, function()
				if ui.iconButton("navTarget", icons.navtarget, lui.HUD_SHOW_NAV_TARGET, ui.theme.buttonColors.transparent, size, padding) then
					reticuleTarget = "navTarget"
				end
			end)

			ui.sameLine(0, 0)

			ui.withStyleColors({ Text = reticuleTarget == "combatTarget" and colors.font or colors.reticuleCircleDark }, function()
				if ui.iconButton("combatTarget", icons.combattarget, lui.HUD_SHOW_COMBAT_TARGET, ui.theme.buttonColors.transparent, size, padding) then
					reticuleTarget = "combatTarget"
				end
			end)
		end
	end)
end

local function displayDetailData(target, data, radius, tooltip)
	local targets = {
		combatTarget = player:GetCombatTarget(),
		navTarget = player:GetNavTarget(),
	}

	local is_planet = not (target:isa("Ship") or target:isa("SpaceStation"))

	local uiPos = ui.pointOnClock(center, radius, 3)
	ui.setCursorPos(uiPos - Vector2(0, pionillium.heading.size * 3))

	ui.group(function()
		ui.withFont(pionillium.heading, function()
			local color = target == targets.combatTarget and colors.combatTarget
				or target == targets.navTarget and colors.navTarget
				or colors.font

			ui.withStyleColors({ Text = color }, function()
				ui.textShadowed(target.label)
			end)
		end)

		if ui.isItemClicked(1) or bindings.targetRadial.action:IsJustActive() then
			local action_binding = bindings.targetRadial.action:IsActive() and bindings.targetRadial.action
			ui.openDefaultRadialMenu("game", target, ui.getMousePos(), action_binding)
		end

		ui.setItemTooltip(tooltip)

		frameDetailStyle:withStyle(function()

			ui.withFont(pionillium.reticle_md, function()
				ui.textShadowed(ui.get_icon_glyph(icons.prograde_thin) .. " " .. ui.Format.Speed(data.ship_speed))
				ui.setItemTooltip(lui.HUD_SPEED_RELATIVE_TO_TARGET)

				ui.textShadowed(ui.get_icon_glyph(icons.antinormal_thin) .. " " .. ui.Format.Speed(data.approach_speed))
				ui.setItemTooltip(lui.HUD_SPEED_OF_APPROACH_TO_FRAME)

				ui.spacing()

				local dist_icon = is_planet and icons.altitude or icons.radial_out_thin

				ui.textShadowed(ui.get_icon_glyph(dist_icon) .. " " .. ui.Format.Distance(data.distance))
				ui.setItemTooltip(lui.HUD_DISTANCE_TO_SURFACE_OF_TARGET)

				ui.textShadowed(ui.get_icon_glyph(icons.radial_in_thin) .. " ~" .. ui.Format.Distance(data.brake_distance))
				ui.setItemTooltip(lui.HUD_BRAKE_DISTANCE_MAIN_THRUSTERS)
			end)

		end)

		ui.spacing()

		displayTargetSwitcher(targets)

	end)

end

-- display current maneuver data below the reticule circle
local function displayManeuverData(radius)
	local maneuverVelocity = player:GetManeuverVelocity()
	local maneuverSpeed = maneuverVelocity:length()
	if maneuverSpeed > 0 and not (player:IsDocked() or player:IsLanded()) then
		local onscreen,position,direction = Engine.ProjectRelDirection(maneuverVelocity)
		gameView.displayIndicator(onscreen, position, direction, icons.bullseye, colors.maneuver, true, lui.HUD_INDICATOR_MANEUVER_PROGRADE)
		local uiPos = ui.pointOnClock(center, radius, 6)
		local speed, speed_unit = ui.Format.SpeedUnit(maneuverSpeed)
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

local aicommand_info = {
	CMD_DOCK = { icon = icons.autopilot_dock, tooltip = lui.HUD_BUTTON_AUTOPILOT_DOCKING },
	CMD_FLYTO = { icon = icons.autopilot_fly_to, tooltip = lui.HUD_BUTTON_AUTOPILOT_FLYING_TO_TARGET },
	CMD_FORMATION = { icon = icons.autopilot_fly_to, tooltip = lui.HUD_BUTTON_AUTOPILOT_FLYING_TO_TARGET },
	CMD_FLYAROUND = { icon = icons.autopilot_medium_orbit, tooltip = lui.HUD_BUTTON_AUTOPILOT_ENTERING_ORBIT },
}

local flightstate_info = {
	CONTROL_MANUAL = { icon = icons.empty, tooltip = lui.HUD_BUTTON_MANUAL_CONTROL },
	-- "CONTROL_AUTOPILOT" - depends on the current command
	-- "CONTROL_FIXSPEED" - depends on the cruise mode
	CONTROL_FIXHEADING_FORWARD = { icon = icons.prograde_thin, tooltip = lui.HUD_BUTTON_FIX_PROGRADE },
	CONTROL_FIXHEADING_BACKWARD = { icon = icons.retrograde_thin , tooltip = lui.HUD_BUTTON_FIX_RETROGRADE },
	CONTROL_FIXHEADING_NORMAL = { icon = icons.normal_thin, tooltip = lui.HUD_BUTTON_FIX_NORMAL },
	CONTROL_FIXHEADING_ANTINORMAL = { icon = icons.antinormal_thin, tooltip = lui.HUD_BUTTON_FIX_ANTINORMAL },
	CONTROL_FIXHEADING_RADIALLY_INWARD = { icon = icons.radial_in_thin, tooltip = lui.HUD_BUTTON_FIX_RADIAL_IN },
	CONTROL_FIXHEADING_RADIALLY_OUTWARD = { icon = icons.radial_out_thin, tooltip = lui.HUD_BUTTON_FIX_RADIAL_OUT },
	-- "CONTROL_FIXHEADING_KILLROT" uses the same icon as rotation damping on
	CONTROL_FIXHEADING_KILLROT = { icon = icons.rotation_damping_on , tooltip = lui.HUD_BUTTON_KILL_ROTATION }
}

local radial_menu_actions_orbital = {
	{icon = ui.theme.icons.normal_thin,
	 tooltip=lc.HEADING_LOCK_NORMAL,
	 action=function(_) Game.player:SetFlightControlState("CONTROL_FIXHEADING_NORMAL") end},
	{icon = ui.theme.icons.radial_out_thin,
	 tooltip=lc.HEADING_LOCK_RADIALLY_OUTWARD,
	 action=function(_) Game.player:SetFlightControlState("CONTROL_FIXHEADING_RADIALLY_OUTWARD") end},
	{icon = ui.theme.icons.retrograde_thin,
	 tooltip=lc.HEADING_LOCK_BACKWARD,
	 action=function(_) Game.player:SetFlightControlState("CONTROL_FIXHEADING_BACKWARD") end},
	{icon = ui.theme.icons.antinormal_thin,
	 tooltip=lc.HEADING_LOCK_ANTINORMAL,
	 action=function(_) Game.player:SetFlightControlState("CONTROL_FIXHEADING_ANTINORMAL") end},
	{icon = ui.theme.icons.radial_in_thin,
	 tooltip=lc.HEADING_LOCK_RADIALLY_INWARD,
	 action=function(_) Game.player:SetFlightControlState("CONTROL_FIXHEADING_RADIALLY_INWARD") end},
	{icon = ui.theme.icons.prograde_thin,
	 tooltip=lc.HEADING_LOCK_FORWARD,
	 action=function(_) Game.player:SetFlightControlState("CONTROL_FIXHEADING_FORWARD") end},
}

local color_active = colors.white
local color_disable = colors.white:opacity(0.3)
local color_inactive = colors.lightBlackBackground

local flight_assist_buttons = {

	cruise_forward = {
		icon = icons.circ_cruise_fwd,
		tooltip = lui.HUD_BUTTON_CRUISE_FORWARD,
		color = color_active,
		action = function(_)
			Game.player:SetCruiseDirection("CRUISE_FWD")
			Game.player:SetFlightControlState("CONTROL_FIXSPEED")
		end
	},

	cruise_up = {
		icon = icons.circ_cruise_up,
		tooltip = lui.HUD_BUTTON_CRUISE_UP,
		color = color_active,
		action = function(_)
			Game.player:SetCruiseDirection("CRUISE_UP")
			Game.player:SetFlightControlState("CONTROL_FIXSPEED")
		end
	},

	disable_cruise = {
		icon = icons.circ_clear_flwtarget,
		tooltip = lui.HUD_BUTTON_DISABLE_CRUISE,
		color = color_disable,
		action = function(_) Game.player:SetFlightControlState("CONTROL_MANUAL") end
	},

	disable_follow = {
		icon = icons.circ_clear_flwtarget,
		tooltip = lui.HUD_BUTTON_DISABLE_FOLLOW_MODE,
		color = color_disable,
		action = function(_) Game.player:SetFollowTarget(nil) end
	}

}

-- pos is the center of the flight assist button
local function flightAssistButton(pos)
	local icon_size = flightAssistIconSize
	local icon_padding = flightAssistButtonPadding
	local button_size = flightAssistButtonSize
	local tooltip = ""
	local back_color = colors.lightBlueBackground:opacity(0.3)
	-- info:
	local flightstate = player:GetFlightState()
	local flightcontrolstate = player:GetFlightControlState()
	local followtarget = player:GetFollowTarget()
	local navtarget = player:GetNavTarget()
	local cmbtarget = player:GetCombatTarget()
	local frmtarget = player.frameBody
	local followmode = player:GetFollowMode()
	local cruisedir = player:GetCruiseDirection()

	local main_icon = nil -- must be overriden
	if flightcontrolstate == "CONTROL_AUTOPILOT" then
		local cmd = player:GetCurrentAICommand()
		local ci = aicommand_info[cmd]
		main_icon = icons.backward
		tooltip = "UNKNOWN AI COMMAND?"
		if ci then
			main_icon = ci.icon
			tooltip = ci.tooltip
		end
	elseif flightcontrolstate == "CONTROL_FIXSPEED" then
		main_icon = cruisedir == "CRUISE_FWD" and icons.cruise_fwd or icons.cruise_up
		tooltip = cruisedir == "CRUISE_FWD" and lui.HUD_BUTTON_CRUISE_FORWARD or lui.HUD_BUTTON_CRUISE_UP
	else
		main_icon = flightstate_info[flightcontrolstate].icon
		tooltip = flightstate_info[flightcontrolstate].tooltip
	end

	-- background
	ui.addIcon(pos, icons.follow_fill, back_color, Vector2(button_size, button_size), ui.anchor.center, ui.anchor.center, tooltip)
	ui.addIcon(pos, icons.follow_edge, back_color, Vector2(button_size, button_size), ui.anchor.center, ui.anchor.center, tooltip)
	-- edge icon
	if followtarget then
		local follow_icon = followmode == "FOLLOW_POS" and icons.follow_pos or icons.follow_ori
		ui.addIcon(pos, follow_icon, colors.white, Vector2(button_size, button_size), ui.anchor.center, ui.anchor.center, tooltip)
	end

	-- main icon
	ui.addIcon(pos, main_icon, colors.white, Vector2(icon_size, icon_size), ui.anchor.center, ui.anchor.center, tooltip)

	-- radial menu
	if ui.isMouseClicked(0) and ui.isWindowHovered() and (ui.getMousePos() - pos):length() < button_size / 2.0
		or bindings.assistRadial.action:IsJustActive() then

		local icon_left = { icon = icons.backward, tooltip = "NO_ACTION", action = function(_) end, color = color_inactive }
		local icon_right = { icon = icons.backward, tooltip = "NO_ACTION", action = function(_) end, color = color_inactive }
		local icon_up = { icon = icons.backward, tooltip = "NO_ACTION", action = function(_) end, color = color_inactive }
		local icon_down = { icon = icons.backward, tooltip = "NO_ACTION", action = function(_) end, color = color_inactive }

		-- up/down icons
		if flightcontrolstate == "CONTROL_MANUAL" or flightcontrolstate == "CONTROL_FIXSPEED" then
			icon_up = flight_assist_buttons.cruise_forward
			icon_down = flight_assist_buttons.cruise_up
			if flightcontrolstate == "CONTROL_FIXSPEED" then
				if cruisedir == "CRUISE_FWD" then
					icon_up = flight_assist_buttons.disable_cruise
				else
					icon_down = flight_assist_buttons.disable_cruise
				end
			end
		else
			local forward = cruisedir == "CRUISE_FWD"
			local fix_icon = forward and icon_up or icon_down
			local man_icon = forward and icon_down or icon_up
			fix_icon.icon = forward and icons.circ_cruise_fwd or icons.circ_cruise_up
			fix_icon.tooltip = lui.HUD_BUTTON_CRUISE_FORWARD
			fix_icon.color = color_active
			fix_icon.action = function(_) player:SetFlightControlState("CONTROL_FIXSPEED") end

			man_icon.icon = icons.circ_clear_flwtarget
			man_icon.tooltip = flightcontrolstate == "CONTROL_AUTOPILOT" and
				lui.HUD_BUTTON_DISABLE_AUTOPILOT or lui.HUD_BUTTON_DISABLE_FIXHEADING
			man_icon.color = color_disable
			man_icon.action = function(_) player:SetFlightControlState("CONTROL_MANUAL") end
		end

		-- side icons
		local settarget = followtarget

		if not followtarget and reticuleTarget then
			if     reticuleTarget == "frame"        then settarget = frmtarget
			elseif reticuleTarget == "navTarget"    then settarget = navtarget
			elseif reticuleTarget == "combatTarget" then settarget = cmbtarget
			end
		end

		if settarget then

			local settargetname = settarget:GetLabel()
			-- ori / pos does only works for dynamic bodies and orbitals
			local can_follow = not settarget.type or settarget.type == "STARPORT_ORBITAL"

			if can_follow then

				local is_follow_pos = followtarget and followmode == "FOLLOW_POS"
				local is_follow_ori = followtarget and followmode == "FOLLOW_ORI"
				local distance = player:DistanceTo(settarget)

				if is_follow_ori then
					icon_left = flight_assist_buttons.disable_follow
				elseif distance > player:GetMaxFollowDistance('FOLLOW_ORI') then
					icon_left.icon = icons.follow_ori
					icon_left.tooltip = string.interp(lui.HUD_FOLLOW_ORIENTATION_NOT_AVAILABLE_TOO_FAR, { targetname = settargetname })
				else
					icon_left.icon = icons.follow_ori
					icon_left.color = color_active
					icon_left.tooltip = string.interp(lui.HUD_FOLLOW_ORIENTATION, { targetname = settargetname })
					icon_left.action = function(_)
						if flightcontrolstate ~= "CONTROL_MANUAL" then
							player:SetFlightControlState("CONTROL_FIXSPEED")
						end
						player:SetFollowTarget(settarget)
						player:SetFollowMode("FOLLOW_ORI")
					end
				end

				if is_follow_pos then
					icon_right = flight_assist_buttons.disable_follow
				elseif distance > player:GetMaxFollowDistance('FOLLOW_POS') then
					icon_right.icon = icons.follow_pos
					icon_right.tooltip = string.interp(lui.HUD_FOLLOW_POSITION_NOT_AVAILABLE_TOO_FAR, { targetname = settargetname })
				else
					icon_right.icon = icons.follow_pos
					icon_right.color = color_active
					icon_right.tooltip = string.interp(lui.HUD_FOLLOW_POSITION, { targetname = settargetname })
					icon_right.action = function(_)
						player:SetFlightControlState("CONTROL_FIXSPEED")
						player:SetFollowTarget(settarget)
						player:SetFollowMode("FOLLOW_POS")
						-- switch to cruise mode automatically if follow position was enabled from manual mode
						if flightcontrolstate == "CONTROL_MANUAL" then
							player:SetCruiseDirection("CRUISE_FWD")
						end
					end
				end
			else
				icon_left.tooltip = string.interp( lui.HUD_FOLLOW_ORIENTATION_NOT_AVAILABLE, { targetname = settargetname })
				icon_right.tooltip = string.interp( lui.HUD_FOLLOW_POSITION_NOT_AVAILABLE, { targetname = settargetname })
			end
		end

		-- consider keyboard / joystick activation
		local action_binding = bindings.assistRadial.action:IsActive() and bindings.assistRadial.action

		-- create quad menu
		local my_quad_menu = { icon_right, icon_down, icon_left, icon_up }
		ui.openRadialMenu("game", nil, 0, button_size, my_quad_menu, 0, pos, action_binding)
	elseif ui.isMouseClicked(1) and ui.isWindowHovered() and (ui.getMousePos() - pos):length() < button_size / 2.0
		or bindings.fixheadingRadial.action:IsJustActive() then
		local frame = player.frameBody
		if frame then
			local action_binding = bindings.fixheadingRadial.action:IsActive() and bindings.fixheadingRadial.action
			ui.openRadialMenu("game", frame, 1, icon_size, radial_menu_actions_orbital, icon_padding, pos, action_binding)
		end
	end

	-- keyboard
	if (flightstate == "FLYING" and ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f5)) then
		local newState = "CONTROL_MANUAL"
		if ui.ctrlHeld() and flightcontrolstate == "CONTROL_FIXSPEED" then
			newState = "CONTROL_FIXHEADING_FORWARD"
		elseif flightcontrolstate == "CONTROL_MANUAL" then
			newState = "CONTROL_FIXSPEED"
		end
		Game.player:SetFlightControlState(newState)
		ui.playBoinkNoise()
	end

	-- mouse wheel
	local wheel = ui.getMouseWheel()
	if wheel ~= 0 and (ui.getMousePos() - pos):length() < button_size / 2.0 then
		local delta = wheel
		if ui.shiftHeld() then
			delta = delta * 10
		end
		Game.player:ChangeCruiseSpeed(delta)
	end
end

local function displayFlightAssist(radius)
	local follow_target = player:GetFollowTarget()
	local assist_button_radius = flightAssistButtonSize / 2
	local uiPos = ui.pointOnClock(center, radius + assist_button_radius * 2, 4.1)
	local padding = 3.0
	local speed_limit_size = assist_button_radius

	flightAssistButton(Vector2(uiPos.x, uiPos.y))

	local cruise_speed = player:GetCruiseSpeed()
	local speed_limit = player:GetSpeedLimit()

	if speed_limit then
		local cur_speed = player:GetVelocity():length()
		local hit_speed_limit = math.abs(speed_limit) - math.abs(cruise_speed or cur_speed) < 0.001

		ui.setCursorPos(uiPos - Vector2(assist_button_radius + speed_limit_size + padding, speed_limit_size * 0.5))

		ui.withStyleColors({ Text = hit_speed_limit and colors.reticuleCircle or colors.reticuleCircleDark }, function()
			ui.withFont(pionicons.large.name, speed_limit_size, function()
				ui.textShadowed(ui.get_icon_glyph(icons.speed_limiter))
			end)
		end)
		ui.setItemTooltip(lui.HUD_SPEED_LIMITER_ACTIVE)

	end

	uiPos.x = uiPos.x + assist_button_radius + padding + ui.getItemSpacing().x * 0.5

	local cruise_speed_lbl = cruise_speed and ui.Format.Speed(cruise_speed)

	ui.withFont(pionillium.reticle_md, function()
		if cruise_speed then

			local sz = ui.calcTextSize(cruise_speed_lbl, pionillium.body)
			local follow_sz = follow_target and ui.calcTextSize(follow_target.label)
			local spacing = ui.getItemSpacing().y

			if follow_target then

				ui.withStyleColors({ Text = colors.fontDim }, function()
					ui.setCursorPos(uiPos - Vector2(math.min(0, -0.5 * (sz.x - follow_sz.x)), follow_sz.y + spacing * 0.5))

					ui.textShadowed(follow_target.label)
					ui.setItemTooltip(lui.HUD_FOLLOW_TARGET)
				end)

			end

			ui.withFont(pionillium.body, function()
				ui.setCursorPos(uiPos + Vector2(0, follow_target and spacing * 0.5 or sz.y * -0.5))

				ui.textShadowed(cruise_speed_lbl)
				ui.setItemTooltip(lui.HUD_CRUISE_SPEED)
			end)


		elseif follow_target then

			ui.withStyleColors({ Text = colors.fontDim }, function()
				ui.setCursorPos(uiPos - Vector2(0, ui.calcTextSize(follow_target.label).y * 0.5))

				ui.textShadowed(follow_target.label)
				ui.setItemTooltip(lui.HUD_FOLLOW_TARGET)
			end)

		end
	end)
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
	local frameLabel = player.frameLabel
	local navTarget = player:GetNavTarget()
	local combatTarget = player:GetCombatTarget()
	local radius = reticuleCircleRadius * 1.2

	if reticuleTarget == "frame" and (navTarget or combatTarget) then
		reticuleTarget = (combatTarget and "combatTarget") or (navTarget and "navTarget") --[[@as string]]
	end

	if frame then
		displayFrameData(frame, radius, lui.HUD_CURRENT_FRAME)
	else
		displayFrameLabel(frameLabel, radius, lui.HUD_CURRENT_FRAME)
	end

	local targetData = nil

	if reticuleTarget == "navTarget" and navTarget then
		targetData = makeTargetData(navTarget)
		displayDetailData(navTarget, targetData, radius, lui.HUD_CURRENT_NAV_TARGET)
	elseif reticuleTarget == "combatTarget" and combatTarget then
		targetData = makeTargetData(combatTarget)
		displayDetailData(combatTarget, targetData, radius, lui.HUD_CURRENT_COMBAT_TARGET)
	end

	if not targetData and frame then
		targetData = makeTargetData(frame)
	end

	-- current speed of approach
	if targetData and targetData.approach_speed > 0 and player:GetFlightState() == "FLYING" then
		displayReticuleBrakeGauge(
			targetData.brake_distance / targetData.distance,
			targetData.brake_distance_retro / targetData.distance)
	end

	displayFlightAssist(radius)
	displayManeuverData(radius)
	displayReticulePitchHorizonCompass()
	displayReticuleDeltaV()
	displayAlertMarker()
end

gameView.registerModule("reticule", {
	showInHyperspace = false,
	draw = function(_)
		center = gameView.center
		colors = ui.theme.colors
		displayReticule()
	end,
	debugReload = function()
		package.reimport()
		package.reimport('pigui.modules.flight-ui.body-icons')
	end
})

Event.Register("onGameStart", function()
	player = Game.player
end)

ui.Events.Register("onPlayerTargetChanged", function(target)
	if target then
		reticuleTarget =
			(target == player:GetCombatTarget() and "combatTarget") or
			(target == player:GetNavTarget() and "navTarget") or
			"frame"
	else
		reticuleTarget =
			(player:GetCombatTarget() and "combatTarget") or
			(player:GetNavTarget() and "navTarget") or
			"frame"
	end
end)
