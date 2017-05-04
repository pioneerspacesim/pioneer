local Engine = import('Engine')
local Game = import('Game')
local ui = import('pigui')
local Vector = import('Vector')
local Color = import('Color')
local Lang = import("Lang")
local lc = Lang.GetResource("core");
local lui = Lang.GetResource("ui-core");
local utils = import("utils")
local Event = import("Event")

-- cache ui
local pionillium = ui.fonts.pionillium
local colors = ui.theme.colors
local icons = ui.theme.icons

local reticuleCircleRadius = math.min(ui.screenWidth, ui.screenHeight) / 13
local reticuleCircleThickness = 2.0

-- settings
local ASTEROID_RADIUS = 1500000 -- rocky planets smaller than this (in meters) are considered an asteroid, not a planet
local IN_SPACE_INDICATOR_SHIP_MAX_DISTANCE = 1000000 -- ships farther away than this don't show up on as in-space indicators
-- center of screen, set each frame by the handler
local center = nil

-- this should go into HUD settings
local showNavigationalNumbers = true

-- cache player each frame
local player = nil

-- display the pitch indicator on the right inside of the reticule circle
local function displayReticulePitch(pitch_degrees)
	local function pitchline(hrs, length, color, thickness)
		local a = ui.pointOnClock(center, reticuleCircleRadius - 1 - length, hrs)
		local b = ui.pointOnClock(center, reticuleCircleRadius - 1, hrs)
		ui.addLine(a, b, color, thickness)
	end
	local tick_length = 2
	pitchline(3, tick_length * 2, colors.reticuleCircle, 1)
	pitchline(2.25, tick_length, colors.reticuleCircle, 1)
	pitchline(3.75, tick_length, colors.reticuleCircle, 1)
	pitchline(1.5, tick_length * 2, colors.reticuleCircle, 1)
	pitchline(4.5, tick_length * 2, colors.reticuleCircle, 1)
	local xpitch = (pitch_degrees + 90) / 180
	local xpitch_h = 4.5 - xpitch * 3
	pitchline(xpitch_h, tick_length * 3, colors.navigationalElements, 2)
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
	-- left hook
	ui.addLine(ui.pointOnClock(center, reticuleCircleRadius - offset, hrs),
						 ui.pointOnClock(center, reticuleCircleRadius - offset - width, hrs),
						 colors.navigationalElements, 1)
	ui.addLine(ui.pointOnClock(center, reticuleCircleRadius - offset, hrs),
						 ui.pointOnClock(center, reticuleCircleRadius - offset, hrs + height_hrs),
						 colors.navigationalElements, 1)
	ui.addLine(ui.pointOnClock(center, reticuleCircleRadius - offset, -3),
						 ui.pointOnClock(center, reticuleCircleRadius - offset + width/2, -3),
						 colors.navigationalElements, 1)
	-- right hook
	ui.addLine(ui.pointOnClock(center, reticuleCircleRadius - offset, hrs + 6),
						 ui.pointOnClock(center, reticuleCircleRadius - offset - width, hrs + 6),
						 colors.navigationalElements, 1)
	ui.addLine(ui.pointOnClock(center, reticuleCircleRadius - offset, hrs + 6),
						 ui.pointOnClock(center, reticuleCircleRadius - offset, hrs + 6 - height_hrs),
						 colors.navigationalElements, 1)
	ui.addLine(ui.pointOnClock(center, reticuleCircleRadius - offset, 3),
						 ui.pointOnClock(center, reticuleCircleRadius - offset + width/2, 3),
						 colors.navigationalElements, 1)
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

	ui.addLine(ui.pointOnClock(center, reticuleCircleRadius, 0),
						 ui.pointOnClock(center, reticuleCircleRadius - 3, 0),
						 colors.reticuleCircle, 1)

	local function stroke(d, p, multiple, height, thickness)
		if d % multiple == 0 then
			local a = ui.pointOnClock(center, reticuleCircleRadius, 2.8 * p - 1.4)
			local b = ui.pointOnClock(center, reticuleCircleRadius + height, 2.8 * p - 1.4)
			ui.addLine(a, b, colors.reticuleCircle, thickness)
		end
	end

	for d=left,right do
		local p = (d - left) / 90
		stroke(d, p, 15, 3, 1)
		stroke(d, p, 45, 4, 1)
		stroke(d, p, 90, 4, 2)
		for k,v in pairs(directions) do
			if clamp(k) == clamp(d) then
				local a = ui.pointOnClock(center, reticuleCircleRadius + 8, 3 * p - 1.5)
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
	local deltav_maneuver = player:GetManeuverVelocity():magnitude()
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
										{ text=dvr_unit,            color=colors.reticuleCircleDark, font=pionillium.tiny,  tooltip=lui.HUD_DELTA_V }},
									colors.lightBlackBackground)
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


-- display the HUD markers in space for ship forward, backward, left, right, up and down
local function displayDirectionalMarkers()
	local function displayDirectionalMarker(ship_space, icon, showDirection, angle)
		local screen = Engine.ShipSpaceToScreenSpace(ship_space)
		if screen.z <= 1 then
			ui.addIcon(screen, icon, colors.reticuleCircle, 32, ui.anchor.center, ui.anchor.center, nil, angle)
		end
		return showDirection and (screen - center):magnitude() > reticuleCircleRadius
	end
	local function angle(forward, adjust)
		if forward.z >= 1 then
			return forward:angle() + adjust - ui.pi
		else
			return forward:angle() + adjust
		end
  end
	local forward = Engine.ShipSpaceToScreenSpace(Vector(0,0,-1)) - center
	local showDirection = displayDirectionalMarker(Vector(0,0,-1), icons.forward, true)
	showDirection = displayDirectionalMarker(Vector(0,0,1), icons.backward, showDirection)
	showDirection = displayDirectionalMarker(Vector(0,1,0), icons.up, showDirection, angle(forward, ui.pi))
	showDirection = displayDirectionalMarker(Vector(0,-1,0), icons.down, showDirection, angle(forward, 0))
	showDirection = displayDirectionalMarker(Vector(1,0,0), icons.right, showDirection)
	showDirection = displayDirectionalMarker(Vector(-1,0,0), icons.left, showDirection)

	if showDirection then
		ui.addIcon(center, icons.direction_forward, colors.reticuleCircle, 32, ui.anchor.center, ui.anchor.center, nil, angle(forward, 0))
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

-- show the larger indicator "in-space" around something and maybe the small indicator inside the reticule circle
local function displayIndicator(onscreen, position, direction, icon, color, showIndicator, tooltip)
	local size = 32 -- size of full icon
	local indicatorSize = 16 -- size of small indicator inside the reticule circle
	local dir = direction * reticuleCircleRadius * 0.90
	local indicator = center + dir
	if onscreen then
		ui.addIcon(position, icon, color, size, ui.anchor.center, ui.anchor.center)
		if tooltip then
			local mouse_position = ui.getMousePos()
			if (mouse_position - position):magnitude() < indicatorSize then -- not size on purpose, most icons are much smaller
				ui.setTooltip(tooltip)
			end
		end
	end
	-- only show small indicator if the large icon is outside the reticule radius
	if showIndicator and (center - position):magnitude() > reticuleCircleRadius * 1.2 then
		ui.addIcon(indicator, icon, color, indicatorSize, ui.anchor.center, ui.anchor.center)
	end
end

local reticuleTarget = "frame"

-- show frame / target switch buttons if anything is targetted
local function displayDetailButtons(radius, navTarget, combatTarget)
	local uiPos = ui.pointOnClock(center, radius, 4.2)
	local mouse_position = ui.getMousePos()
	local size = 24
	if combatTarget or navTarget then
		local color = reticuleTarget == "frame" and colors.reticuleCircle or colors.reticuleCircleDark
		ui.addIcon(uiPos, icons.display_frame, color, size, ui.anchor.left, ui.anchor.bottom, lui.HUD_SHOW_FRAME)
		if ui.isMouseClicked(0) and (mouse_position - (uiPos + Vector(size/2, -size/2))):magnitude() < size/2 then
			reticuleTarget = "frame"
		end
		uiPos = uiPos + Vector(size,0)
	end
	if navTarget then
		local color = reticuleTarget == "navTarget" and colors.reticuleCircle or colors.reticuleCircleDark
		ui.addIcon(uiPos, icons.display_navtarget, color, size, ui.anchor.left, ui.anchor.bottom, lui.HUD_SHOW_NAV_TARGET)
		if ui.isMouseClicked(0) and (mouse_position - (uiPos + Vector(size/2, -size/2))):magnitude() < size/2 then
			reticuleTarget = "navTarget"
		end
		uiPos = uiPos + Vector(size,0)
	end
	if combatTarget then
		local color = reticuleTarget == "combatTarget" and colors.reticuleCircle or colors.reticuleCircleDark
		ui.addIcon(uiPos, icons.display_combattarget, color, size, ui.anchor.left, ui.anchor.bottom, lui.HUD_SHOW_COMBAT_TARGET)
		if ui.isMouseClicked(0) and (mouse_position - (uiPos + Vector(size/2, -size/2))):magnitude() < size/2 then
			reticuleTarget = "combatTarget"
		end
	end
end

local function displayDetailData(target, radius, combatTarget, navTarget, colorLight, colorDark)
	local velocity = player:GetVelocityRelTo(target)
	local position = player:GetPositionRelTo(target)

	local uiPos = ui.pointOnClock(center, radius, 2)
	-- label of target
	local tooltip = reticuleTarget == "combatTarget" and lui.HUD_CURRENT_COMBAT_TARGET or (reticuleTarget == "navTarget" and lui.HUD_CURRENT_NAV_TARGET or lui.HUD_CURRENT_FRAME)
	ui.addStyledText(uiPos, ui.anchor.left, ui.anchor.baseline, target.label, colorDark, pionillium.medium, tooltip, colors.lightBlackBackground)

	-- current distance, relative speed
	uiPos = ui.pointOnClock(center, radius, 2.5)
	-- currently unused: local distance, distance_unit = ui.Format.Distance(player:DistanceTo(target))
	local approach_speed = position:dot(velocity) / position:magnitude()
	local speed, speed_unit = ui.Format.Speed(approach_speed)

	ui.addFancyText(uiPos, ui.anchor.left, ui.anchor.baseline, {
										{ text=speed,      color=colorLight, font=pionillium.medium, tooltip=lui.HUD_SPEED_OF_APPROACH_TO_TARGET },
										{ text=speed_unit, color=colorDark,  font=pionillium.small,  tooltip=lui.HUD_SPEED_OF_APPROACH_TO_TARGET }},
									colors.lightBlackBackground)

	-- current brake distance
	local brake_distance = player:GetDistanceToZeroV(velocity:magnitude(),"forward")
	local brake_distance_retro = player:GetDistanceToZeroV(velocity:magnitude(),"reverse")
	local altitude = player:GetAltitudeRelTo(target)
	local ratio = brake_distance / altitude
	local ratio_retro = brake_distance_retro / altitude
	speed, speed_unit = ui.Format.Speed(velocity:magnitude())

	-- speed
	uiPos = ui.pointOnClock(center, radius, 3.5)
	local distance,unit = ui.Format.Distance(brake_distance)
	ui.addFancyText(uiPos, ui.anchor.left, ui.anchor.baseline, {
										{ text="~" .. distance, color=colorDark, font=pionillium.medium, tooltip=lui.HUD_BRAKE_DISTANCE_MAIN_THRUSTERS },
										{ text=unit,            color=colorDark, font=pionillium.small,  tooltip=lui.HUD_BRAKE_DISTANCE_MAIN_THRUSTERS }},
									colors.lightBlackBackground)

	-- current altitude
	uiPos = ui.pointOnClock(center, radius, 3)
	local altitude, altitude_unit = ui.Format.Distance(altitude)
	ui.addFancyText(uiPos, ui.anchor.left, ui.anchor.baseline, {
										{ text=altitude,      color=colorLight, font=pionillium.medium, tooltip=lui.HUD_DISTANCE_TO_SURFACE_OF_TARGET },
										{ text=altitude_unit, color=colorDark,  font=pionillium.small,  tooltip=lui.HUD_DISTANCE_TO_SURFACE_OF_TARGET },
										{ text=" " .. speed,  color=colorLight, font=pionillium.medium, tooltip=lui.HUD_SPEED_RELATIVE_TO_TARGET },
										{ text=speed_unit,    color=colorDark,  font=pionillium.small,  tooltip=lui.HUD_SPEED_RELATIVE_TO_TARGET }},
									colors.lightBlackBackground)

	-- current speed of approach
	if approach_speed < 0 then
		displayReticuleBrakeGauge(ratio, ratio_retro)
	end

	displayDetailButtons(radius, navTarget, combatTarget)
end

-- display the indicator pointing at the combat target
local function displayCombatTargetIndicator(combatTarget)
	local pos = combatTarget:GetPositionRelTo(player)
	local vel = -combatTarget:GetVelocityRelTo(player)
	local onscreen,position,direction = Engine.WorldSpaceToScreenSpace(pos)
	displayIndicator(onscreen, position, direction, icons.square, colors.combatTarget, true)
	onscreen,position,direction = Engine.WorldSpaceToScreenSpace(vel)
	displayIndicator(onscreen, position, direction, icons.prograde, colors.combatTarget, true, lui.HUD_INDICATOR_COMBAT_TARGET_PROGRADE)
	onscreen,position,direction = Engine.WorldSpaceToScreenSpace(-vel)
	displayIndicator(onscreen, position, direction, icons.retrograde, colors.combatTarget, false, lui.HUD_INDICATOR_COMBAT_TARGET_RETROGRADE)
end

-- display indicators relative to frame
local function displayFrameIndicators(frame, navTarget)
	local frameVelocity = -frame:GetVelocityRelTo(player)
	if frameVelocity:magnitude() > 1 and frame ~= navTarget then
		local onscreen,position,direction = Engine.WorldSpaceToScreenSpace(frameVelocity)
		displayIndicator(onscreen, position, direction, icons.prograde, colors.frame, true, lui.HUD_INDICATOR_FRAME_PROGRADE)
		onscreen,position,direction = Engine.WorldSpaceToScreenSpace(-frameVelocity)
		displayIndicator(onscreen, position, direction, icons.retrograde, colors.frame, false, lui.HUD_INDICATOR_FRAME_RETROGRADE)
	end

	local awayFromFrame = player:GetPositionRelTo(frame) * 1.01
	local onscreen,position,direction = Engine.WorldSpaceToScreenSpace(awayFromFrame)
	displayIndicator(onscreen, position, direction, icons.frame_away, colors.frame, false, lui.HUD_INDICATOR_AWAY_FROM_FRAME)
end

-- display the indicator pointing at the nav target, and pro- and retrograde
local function displayNavTargetIndicator(navTarget)
	local onscreen,position,direction = navTarget:GetTargetIndicatorScreenPosition()
	displayIndicator(onscreen, position, direction, icons.square, colors.navTarget, true)
	local navVelocity = -navTarget:GetVelocityRelTo(Game.player)
	if navVelocity:magnitude() > 1 then
		onscreen,position,direction = Engine.WorldSpaceToScreenSpace(navVelocity)
		displayIndicator(onscreen, position, direction, icons.prograde, colors.navTarget, true, lui.HUD_INDICATOR_NAV_TARGET_PROGRADE)
		onscreen,position,direction = Engine.WorldSpaceToScreenSpace(-navVelocity)
		displayIndicator(onscreen, position, direction, icons.retrograde, colors.navTarget, false, lui.HUD_INDICATOR_NAV_TARGET_RETROGRADE)
	end
end

-- display data relative to frame left of the reticule circle
local function displayFrameData(frame, radius)
	local velocity = player:GetVelocityRelTo(frame)
	local position = player:GetPositionRelTo(frame)
	local altitude = player:GetAltitudeRelTo(frame)
	local altitude, altitude_unit = ui.Format.Distance(altitude)
	local approach_speed = position:dot(velocity) / position:magnitude()
	local speed, speed_unit = ui.Format.Speed(approach_speed)
	local uiPos = ui.pointOnClock(center, radius, -2)
	-- label of frame
	ui.addStyledText(uiPos, ui.anchor.right, ui.anchor.baseline, frame.label, colors.frame, pionillium.medium, lui.HUD_CURRENT_FRAME, colors.lightBlackBackground)
	-- altitude above frame
	uiPos = ui.pointOnClock(center, radius, -3)
	ui.addFancyText(uiPos, ui.anchor.right, ui.anchor.baseline, {
										{ text=altitude,      color=colors.frame,     font=pionillium.medium, tooltip=lui.HUD_DISTANCE_TO_SURFACE_OF_FRAME },
										{ text=altitude_unit, color=colors.frameDark, font=pionillium.small,  tooltip=lui.HUD_DISTANCE_TO_SURFACE_OF_FRAME }},
									colors.lightBlackBackground)

	-- speed of approach of frame
	uiPos = ui.pointOnClock(center, radius, -2.5)
	ui.addFancyText(uiPos, ui.anchor.right, ui.anchor.baseline, {
										{ text=speed,      color=colors.frame,     font=pionillium.medium, tooltip=lui.HUD_SPEED_OF_APPROACH_TO_FRAME },
										{ text=speed_unit, color=colors.frameDark, font=pionillium.small,  tooltip=lui.HUD_SPEED_OF_APPROACH_TO_FRAME }},
									colors.lightBlackBackground)
end

-- display current maneuver data below the reticule circle
local function displayManeuverData(radius)
	local maneuverVelocity = player:GetManeuverVelocity()
	local maneuverSpeed = maneuverVelocity:magnitude()
	if maneuverSpeed > 0 and not (player:IsDocked() or player:IsLanded()) then
		local onscreen,position,direction = Engine.WorldSpaceToScreenSpace(maneuverVelocity)
		displayIndicator(onscreen, position, direction, icons.bullseye, colors.maneuver, true, lui.HUD_INDICATOR_MANEUVER_PROGRADE)
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
											{ text="  ~" .. burn_time, color=colors.maneuver,     font=pionillium.medium, tooltip=lui.HUD_DURATION_OF_MANEUVER_BURN }},
										colors.lightBlackBackground)
	end
end

-- display the new direction indicator if the player is actively moving the ship with the mouse (RMB)
local function displayMouseMoveIndicator()
	-- if mouse is held, show new direction indicator
	if player:IsMouseActive() then
		local direction = player:GetMouseDirection()
		local screen = Engine.CameraSpaceToScreenSpace(direction)
		ui.addIcon(screen, icons.mouse_move_direction, colors.mouseMovementDirection, 32, ui.anchor.center, ui.anchor.center)
	end
end

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

local function displayReticule()
	-- reticule circle
	ui.addCircle(center, reticuleCircleRadius, colors.reticuleCircle, ui.circleSegments(reticuleCircleRadius), reticuleCircleThickness)

	local frame = player.frameBody
	local navTarget = player:GetNavTarget()
	local combatTarget = player:GetCombatTarget()
	local radius = reticuleCircleRadius * 1.2

	updateReticuleTarget(frame, navTarget, combatTarget)

	if navTarget then
		displayNavTargetIndicator(navTarget)
	end
	if combatTarget then
		displayCombatTargetIndicator(combatTarget)
	end

	if reticuleTarget == "frame" then
		displayDetailData(frame, radius, combatTarget, navTarget, colors.frame, colors.frameDark)
	elseif reticuleTarget == "navTarget" then
		displayDetailData(navTarget, radius, combatTarget, navTarget, colors.navTarget, colors.navTargetDark)
	elseif reticuleTarget == "combatTarget" then
		displayDetailData(combatTarget, radius, combatTarget, navTarget, colors.combatTarget, colors.combatTargetDark)
	end

	displayManeuverData(radius)
	displayMouseMoveIndicator()
	displayReticulePitchHorizonCompass()
	displayReticuleDeltaV()
	displayDirectionalMarkers()

	if frame then
		displayFrameIndicators(frame, navTarget)
		if reticuleTarget ~= "frame" then
			displayFrameData(frame, radius)
		end
	end
end

local function displayHyperspace()
	-- TODO implement :)
end

local MAX_RADAR_SIZE = 1000000000
local MIN_RADAR_SIZE = 1000

local shouldDisplay2DRadar = false
local current_radar_size = 10000

-- display the 2D radar
local function display2DRadar(cntr, size)
	local targets = ui.getTargetsNearby(current_radar_size)
	local halfsize = size * 0.5
	local thirdsize = size * 0.3
	local twothirdsize = size * 0.7

	local function line(x,y)
		-- ui.addLine(cntr + Vector(x, y) * halfsize, cntr + Vector(x,y) * size, colors.reticuleCircle, reticuleCircleThickness)
		ui.addLine(cntr + Vector(x, y) * thirdsize, cntr + Vector(x,y) * twothirdsize, colors.reticuleCircle, reticuleCircleThickness)
	end
	ui.addCircleFilled(cntr, size, colors.lightBlueBackground, ui.circleSegments(size), 1)
	ui.addCircle(cntr, size, colors.reticuleCircle, ui.circleSegments(size), reticuleCircleThickness)
	--	ui.addCircle(cntr, halfsize, colors.reticuleCircle, ui.circleSegments(halfsize), reticuleCircleThickness)
	ui.addCircle(cntr, thirdsize, colors.reticuleCircle, ui.circleSegments(thirdsize), reticuleCircleThickness)
	ui.addCircle(cntr, twothirdsize, colors.reticuleCircle, ui.circleSegments(twothirdsize), reticuleCircleThickness)
	local l = ui.oneOverSqrtTwo
	line(-l, l)
	line(-l, -l)
	line(l, -l)
	line(l, l)
	local combatTarget = player:GetCombatTarget()
	local navTarget = player:GetNavTarget()
	local tooltip = {}
	for k,v in pairs(targets) do
		if v.distance < current_radar_size then
			local halfRadarSize = current_radar_size / 2
			local alpha = 255
			if v.distance > halfRadarSize then
				alpha = 255 * (1 - (v.distance - halfRadarSize) / halfRadarSize)
			end
			local position = cntr + v.aep * size * 2
			if v.body == navTarget then
				local color = Color(colors.navTarget.r, colors.navTarget.g, colors.navTarget.b, alpha)
				ui.addIcon(position, icons.square, color, 12, ui.anchor.center, ui.anchor.center)
			elseif v.body == combatTarget then
				local color = Color(colors.combatTarget.r, colors.combatTarget.g, colors.combatTarget.b, alpha)
				ui.addIcon(position, icons.square, color, 12, ui.anchor.center, ui.anchor.center)
			else
				local color = Color(colors.reticuleCircle.r, colors.reticuleCircle.g, colors.reticuleCircle.b, alpha)
				ui.addCircleFilled(position, 2, color, 4, 1)
			end
			local mouse_position = ui.getMousePos()
			if (mouse_position - position):magnitude() < 4 then
				table.insert(tooltip, v.label)
			end
		end
	end
	if #tooltip > 0 then
		ui.setTooltip(table.concat(tooltip, "\n"))
	end
	local d, d_u = ui.Format.Distance(current_radar_size)
	local distance = d .. ' ' .. d_u
	local textcenter = cntr + Vector(0, size + 2)
	local textsize = ui.addStyledText(textcenter, ui.anchor.center, ui.anchor.top, distance, colors.frame, pionillium.small, lui.HUD_RADAR_DISTANCE, colors.lightBlackBackground)
end

-- display either the 3D or the 2D radar, show a popup on right click to select
local function displayRadar()
	local cntr = Vector(ui.screenWidth / 2, ui.screenHeight - reticuleCircleRadius - 15)
	local size = reticuleCircleRadius

	local mp = ui.getMousePos()
	if (Vector(mp.x,mp.y) - cntr):magnitude() < size then
		if ui.isMouseReleased(1) then
			ui.openPopup("radarselector")
		end
		local wheel = ui.getMouseWheel()
		if wheel > 0 then
			current_radar_size = math.max(current_radar_size / 10, MIN_RADAR_SIZE)
		elseif wheel < 0 then
			current_radar_size = math.min(current_radar_size * 10, MAX_RADAR_SIZE)
		end
	end
	ui.popup("radarselector", function()
						 if ui.selectable(lui.HUD_2D_RADAR, shouldDisplay2DRadar, {}) then
							 shouldDisplay2DRadar = true
							 Game.SetRadarVisible(false)
						 end
						 if ui.selectable(lui.HUD_3D_RADAR, not shouldDisplay2DRadar, {}) then
							 shouldDisplay2DRadar = false
							 Game.SetRadarVisible(true)
						 end
	end)
	if shouldDisplay2DRadar then
		display2DRadar(cntr, size)
	end
end

local function getBodyIcon(body)
	local st = body.superType
	local t = body.type
	if st == "STARPORT" then
		if t == "STARPORT_ORBITAL" then
			return icons.spacestation
		elseif body.type == "STARPORT_SURFACE" then
			return icons.starport
		end
	elseif st == "GAS_GIANT" then
		return icons.gas_giant
	elseif st == "STAR" then
		return icons.sun
	elseif st == "ROCKY_PLANET" then
		if body:IsMoon() then
			return icons.moon
		else
			local sb = body:GetSystemBody()
			if sb.radius < ASTEROID_RADIUS then
				return icons.asteroid_hollow
			else
				return icons.rocky_planet
			end
		end
	elseif body:IsShip() then
		local shipClass = body:GetShipClass()
		if icons[shipClass] then
			return icons[shipClass]
		else
			print("data/pigui/game.lua: getBodyIcon unknown ship class " .. (shipClass and shipClass or "nil"))
			return icons.ship -- TODO: better icon
		end
	elseif body:IsHyperspaceCloud() then
		return icons.hyperspace -- TODO: better icon
	elseif body:IsMissile() then
		return icons.bullseye -- TODO: better icon
	elseif body:IsCargoContainer() then
		return icons.rocky_planet
	else
		print("data/pigui/game.lua: getBodyIcon not sure how to process body, supertype: " .. (st and st or "nil") .. ", type: " .. (t and t or "nil"))
		utils.print_r(body)
		return icons.ship
	end
end

local function setTarget(body)
	if body:IsShip() then
		player:SetCombatTarget(body)
	else
		player:SetNavTarget(body)
	end
end

local function callModules(mode)
	for k,v in pairs(ui.getModules(mode)) do
		v()
	end
end

local function displayOnScreenObjects()
	local should_show_label = ui.shouldShowLabels()
	local iconsize = 18
	local label_offset = 14 -- enough so that the target rectangle fits
	local collapse = iconsize
	local bodies = ui.getProjectedBodies()
	local navTarget = player:GetNavTarget()
	local combatTarget = player:GetCombatTarget()
	local onscreen = {}
	-- go through all objects, collecting those that are shown near each other into single objects
	for k,v in pairs(bodies) do
		if v.onscreen then
			local it = v
			it.label = k
			local itk = k
			local itbody = it.body
			local itsc = it.screenCoordinates
			local inserted = false
			-- if body is ship and distance is too great, ignore it
			if not itbody:IsShip() or itbody:DistanceTo(player) < IN_SPACE_INDICATOR_SHIP_MAX_DISTANCE then
				-- never collapse combat target
				if itbody ~= combatTarget then
					for k,v in pairs(onscreen) do
						local mbsc = v.mainBody.screenCoordinates
						if (mbsc - itsc):magnitude() < collapse then
							if itbody == navTarget or (itbody:IsMoreImportantThan(v.mainBody.body) and v.mainBody.body ~= navTarget) then
								-- if the navtarget is one of the collapsed ones, it's the most important
								table.insert(v.others, v.mainBody)
								v.mainBody = it
								v.hasNavTarget = true
								v.multiple = true
							else
								-- otherwise, just add it as a further body
								table.insert(v.others, it)
								v.multiple = true
							end
							inserted = true
							break
						end
					end
				end
				if not inserted then
					-- not collapsed anywhere, just insert it normally
					table.insert(onscreen, { mainBody = it, others = {}, hasCombatTarget = itbody == combatTarget, hasNavTarget = itbody == navTarget, multiple = false })
				end
			end
		end
	end
	-- now display each group
	for k,v in pairs(onscreen) do
		table.sort(v.others, function(a,b) return a.body:IsMoreImportantThan(b.body) end)
		local coords = v.mainBody.screenCoordinates
		ui.addIcon(coords, getBodyIcon(v.mainBody.body), colors.frame, iconsize, ui.anchor.center, ui.anchor.center)
		if should_show_label then
			local multipleText = ""
			if v.multiple then
				-- show a + after the name to indicate it is actually multiple objects
				multipleText = "+"
			end
			ui.addStyledText(coords + Vector(label_offset, 0), ui.anchor.left, ui.anchor.center, v.mainBody.label .. multipleText , colors.frame, pionillium.small)
		end
		local mp = ui.getMousePos()
		-- click handler
		if (Vector(mp.x,mp.y) - coords):magnitude() < iconsize then
			if ui.isMouseReleased(0) then
				if v.hasNavTarget and ui.ctrlHeld() then
					-- if ctrl-clicked and has nav target, unset nav target
					player:SetNavTarget(nil)
				elseif v.hasCombatTarget and ui.ctrlHeld() then
					-- if ctlr-clicked and has combat target, unset combat target
					player:SetCombatTarget(nil)
				else
					if v.multiple then
						-- clicked on group, show popup
						ui.openPopup("navtarget" .. v.mainBody.label)
					else
						-- clicked on single, just set navtarget/combatTarget
						setTarget(v.mainBody.body)
					end
				end
			end
		end
		-- popup content
		ui.popup("navtarget" .. v.mainBody.label, function()
							 local size = Vector(16,16)
							 ui.icon(getBodyIcon(v.mainBody.body), size, colors.frame)
							 ui.sameLine()
							 if ui.selectable(v.mainBody.label, v.mainBody.body == navTarget, {}) then
								 player:SetNavTarget(v.mainBody.body)
							 end
							 for k,v in pairs(v.others) do
								 ui.icon(getBodyIcon(v.body), size, colors.frame)
								 ui.sameLine()
								 if ui.selectable(v.label, v.body == navTarget, {}) then
									 player:SetNavTarget(v.body)
								 end
							 end
		end)
	end
end

Event.Register('onGameStart', function()
								 shouldDisplay2DRadar = false
end)

ui.registerHandler(
	'game',
	function(delta_t)
		-- delta_t is ignored for now
		player = Game.player
		colors = ui.theme.colors -- if the theme changes
		icons = ui.theme.icons -- if the theme changes
		ui.setNextWindowPos(Vector(0, 0), "Always")
		ui.setNextWindowSize(Vector(ui.screenWidth, ui.screenHeight), "Always")
		ui.withStyleColors({ ["WindowBg"] = colors.transparent }, function()
				ui.window("HUD", {"NoTitleBar", "NoResize", "NoMove", "NoInputs", "NoSavedSettings", "NoFocusOnAppearing", "NoBringToFrontOnFocus"}, function()
										center = Vector(ui.screenWidth / 2, ui.screenHeight / 2)
										if Game.CurrentView() == "world" and ui.shouldDrawUI() then
											if Game.InHyperspace() then
												displayHyperspace()
												callModules("hyperspace")
											else
												displayOnScreenObjects()
												displayReticule()
												displayRadar()
												callModules("game")
											end
										else
											if ui.shouldDrawUI() then
												callModules("game")
											end
										end
				end)
		end)
end)

-- todo: f8 low thrust, f5 flight control, f10 equipment ?
-- todo: on hyperspace target changed while counting down -> abort
