-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = require 'pigui'
local Engine = require 'Engine'
local Game = require 'Game'
local Vector2 = _G.Vector2
local Vector3 = _G.Vector3

-- cache ui
local pionillium = ui.fonts.pionillium
local pionicons = ui.fonts.pionicons
local colors = ui.theme.colors
local icons = ui.theme.icons

local Lang = require 'Lang'
local lc = Lang.GetResource("core")
local lui = Lang.GetResource("ui-core")

-- center of screen, set each frame by the handler
local center = nil

-- cache player each frame
local player = nil

local gameView = require 'pigui.views.game'

local reticuleCircleRadius

-- display the HUD markers in space for ship forward, backward, left, right, up and down
local function displayDirectionalMarkers()
	local function displayDirectionalMarker(ship_space, icon, showDirection, angle)
		local screen = Engine.ShipSpaceToScreenSpace(ship_space)
		local coord = Vector2(screen.x, screen.y)
		if screen.z <= 1 then
			ui.addIcon(coord, icon, colors.reticuleCircle, Vector2(32, 32), ui.anchor.center, ui.anchor.center, nil, angle)
		end
		return showDirection and (coord - center):length() > reticuleCircleRadius
	end
	local function angle(forward, adjust)
		local aux2 = Vector2(forward.x, forward.y)
		if forward.z >= 1 then
			return aux2:angle() + adjust - ui.pi
		else
			return aux2:angle() + adjust
		end
	end
	local aux = Vector3(0,0,-1)
	local forward = Engine.ShipSpaceToScreenSpace(aux - Vector3(center))
	local showDirection = displayDirectionalMarker(aux, icons.forward, true)
	showDirection = displayDirectionalMarker(aux(0, 0, 1), icons.backward, showDirection)
	showDirection = displayDirectionalMarker(aux(0, 1, 0), icons.up, showDirection, angle(forward, ui.pi))
	showDirection = displayDirectionalMarker(aux(0, -1, 0), icons.down, showDirection, angle(forward, 0))
	showDirection = displayDirectionalMarker(aux(1, 0, 0), icons.right, showDirection)
	showDirection = displayDirectionalMarker(aux(-1, 0, 0), icons.left, showDirection)

	if showDirection then
		ui.addIcon(center, icons.direction_forward, colors.reticuleCircle, Vector2(32, 32), ui.anchor.center, ui.anchor.center, nil, angle(forward, 0))
	end
end

-- show the larger indicator "in-space" around something and maybe the small indicator inside the reticule circle
local function displayIndicator(onscreen, position, direction, icon, color, showIndicator, tooltip)
	local size = Vector2(32, 32) -- size of full icon
	local indicatorSize = Vector2(16, 16) -- size of small indicator inside the reticule circle
	local dir = Vector2(direction.x, direction.y) * reticuleCircleRadius * 0.90
	local indicator = center + dir
	if onscreen then
		ui.addIcon(position, icon, color, size, ui.anchor.center, ui.anchor.center)
		if tooltip then
			local mouse_position = ui.getMousePos()
			if (mouse_position - position):length() < indicatorSize:length() then -- not size on purpose, most icons are much smaller
				ui.setTooltip(tooltip)
			end
		end
	end
	-- only show small indicator if the large icon is outside the reticule radius
	if showIndicator and (center - position):length() > reticuleCircleRadius * 1.2 then
		ui.addIcon(indicator, icon, color, indicatorSize, ui.anchor.center, ui.anchor.center)
	end
end

gameView.displayIndicator = displayIndicator

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
	if frameVelocity:length() > 1 and frame ~= navTarget then
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
	if navVelocity:length() > 1 then
		onscreen,position,direction = Engine.WorldSpaceToScreenSpace(navVelocity)
		displayIndicator(onscreen, position, direction, icons.prograde, colors.navTarget, true, lui.HUD_INDICATOR_NAV_TARGET_PROGRADE)
		onscreen,position,direction = Engine.WorldSpaceToScreenSpace(-navVelocity)
		displayIndicator(onscreen, position, direction, icons.retrograde, colors.navTarget, false, lui.HUD_INDICATOR_NAV_TARGET_RETROGRADE)
	end
end

-- display the new direction indicator if the player is actively moving the ship with the mouse (RMB)
local function displayMouseMoveIndicator()
	-- if mouse is held, show new direction indicator
	if player:IsMouseActive() then
		local direction = player:GetMouseDirection()
		local screen = Engine.CameraSpaceToScreenSpace(direction)
		local screen2 = Vector2(screen.x, screen.y)
		ui.addIcon(screen2, icons.mouse_move_direction, colors.mouseMovementDirection, Vector2(32, 32), ui.anchor.center, ui.anchor.center)
	end
end

gameView.registerModule("indicators", {
	showInHyperspace = false,
	draw = function (self, dT)
		player = gameView.player
		center = gameView.center
		colors = ui.theme.colors
		reticuleCircleRadius = ui.reticuleCircleRadius

		local frame = player.frameBody
		local navTarget = player:GetNavTarget()
		local combatTarget = player:GetCombatTarget()

		if navTarget then
			displayNavTargetIndicator(navTarget)
		end
		if combatTarget then
			displayCombatTargetIndicator(combatTarget)
		end

		displayMouseMoveIndicator()
		displayDirectionalMarkers()

		if frame then
			displayFrameIndicators(frame, navTarget)
		end
	end
})
