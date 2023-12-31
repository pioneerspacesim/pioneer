-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
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
		if screen.z <= 0 then
			ui.addIcon(coord, icon, colors.reticuleCircle, Vector2(32, 32), ui.anchor.center, ui.anchor.center, nil, angle)
		end
		return showDirection and (coord - center):length() > reticuleCircleRadius
	end
	local function angle(forward, adjust)
		local aux2 = Vector2(forward.x, forward.y)
		if forward.z >= 0 then
			return aux2:angle() + adjust - ui.pi
		else
			return aux2:angle() + adjust
		end
	end
	local aux = Vector3(0,0,-1)
	local forward = Engine.ShipSpaceToScreenSpace(aux) - Vector3(center)
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

local function isTargetObscured(navTarget)

	local frameBody = player.frameBody

	-- checks if the nav target is  behind (on the other side of) the player's framebody
	
	if frameBody and frameBody.path:GetSystemBody().radius > 0 and frameBody ~= navTarget then

		local starport = navTarget.type == "STARPORT_SURFACE" and navTarget or
			navTarget:IsShip() and navTarget:IsDocked() and navTarget:GetDockedWith().type == "STARPORT_SURFACE" and navTarget:GetDockedWith()

		local isTargetOnFrameBody = starport and starport.path:GetSystemBody().parent.path == frameBody.path
			or navTarget:IsShip() and navTarget:IsLanded() and navTarget.frameBody == frameBody

		--local isPlayerOnFrameBody = player:IsLanded() or player:IsDocked() and player:GetDockedWith().type == "STARPORT_SURFACE"

		local navWrtPlayer = navTarget:GetPositionRelTo(player)
		--avoiding calculation of length - no sqrt() 
		local targetDistSqr = navWrtPlayer:lengthSqr()

		--below 10km distance visibility assumed
		if targetDistSqr < 100000000 then
			--print("Too close")
			return false
		end

		local frameWrtPlayer = frameBody:GetPositionRelTo(player)
		local navWrtFrame = navWrtPlayer - frameWrtPlayer

		--ground ports are straightforward - if player is above the zero-horizon from the station's POV, the station is visible
		if isTargetOnFrameBody then
			--print("isTargetOnFrameBody")
			if navWrtFrame:dot(navWrtPlayer) > 0 then return true end

		--player parked another trivial case
		elseif player:IsLanded() or player:IsDocked() and player:GetDockedWith().type == "STARPORT_SURFACE" then
			--print("player on gournd")
			if frameWrtPlayer:dot(navWrtPlayer) > 0 then return true end

		-- generic case both angles have to be acute
		elseif navWrtPlayer:dot(frameWrtPlayer) > 0 and navWrtPlayer:dot(navWrtFrame) > 0 then
			--print("generic case")
			local areaSqr = navWrtPlayer:cross(frameWrtPlayer):lengthSqr()
			-- these will be used to check if the distance between the planet's centre and the line that connects the player and the target
			-- is larger than the planet's radius
			local pointLineDistSqr = areaSqr / targetDistSqr
			local frameRadius = frameBody.path:GetSystemBody().radius

			if pointLineDistSqr < frameRadius^2 then
				return true
			end
		end
	end

	--print("BINGO2")
	return false
end

gameView.displayIndicator = displayIndicator

-- display the indicator pointing at the combat target
local function displayCombatTargetIndicator(combatTarget)
	local pos = combatTarget:GetPositionRelTo(player)
	local vel = -combatTarget:GetVelocityRelTo(player)
	local onscreen,position,direction = Engine.ProjectRelPosition(pos)


	local targetIcon = isTargetObscured(combatTarget) and icons.square_dashed or icons.square
	displayIndicator(onscreen, position, direction, targetIcon, colors.combatTarget, true)
	onscreen,position,direction = Engine.ProjectRelDirection(vel)
	displayIndicator(onscreen, position, direction, icons.prograde, colors.combatTarget, true, lui.HUD_INDICATOR_COMBAT_TARGET_PROGRADE)
	onscreen,position,direction = Engine.ProjectRelDirection(-vel)
	displayIndicator(onscreen, position, direction, icons.retrograde, colors.combatTarget, false, lui.HUD_INDICATOR_COMBAT_TARGET_RETROGRADE)
end

-- display indicators relative to frame
local function displayFrameIndicators(frame, navTarget)
	local frameVelocity = -frame:GetVelocityRelTo(player)
	if frameVelocity:length() > 1 and frame ~= navTarget then
		local onscreen,position,direction = Engine.ProjectRelDirection(frameVelocity)
		displayIndicator(onscreen, position, direction, icons.prograde, colors.frame, true, lui.HUD_INDICATOR_FRAME_PROGRADE)
		onscreen,position,direction = Engine.ProjectRelDirection(-frameVelocity)
		displayIndicator(onscreen, position, direction, icons.retrograde, colors.frame, false, lui.HUD_INDICATOR_FRAME_RETROGRADE)
	end

	local awayFromFrame = -frame:GetPositionRelTo(player)
	local onscreen,position,direction = Engine.ProjectRelDirection(awayFromFrame)
	displayIndicator(onscreen, position, direction, icons.frame_away, colors.frame, false, lui.HUD_INDICATOR_AWAY_FROM_FRAME)
end


-- display the indicator pointing at the nav target, and pro- and retrograde
local function displayNavTargetIndicator(navTarget)
	local onscreen,position,direction = Engine.GetTargetIndicatorScreenPosition(navTarget)
	local frameBody = player.frameBody


	local targetIcon = isTargetObscured(navTarget) and icons.square_dashed or icons.square
	displayIndicator(onscreen, position, direction, targetIcon, colors.navTargetDark, true)
	
	local navVelocity = -navTarget:GetVelocityRelTo(player)
	if navVelocity:length() > 1 then
		onscreen,position,direction = Engine.ProjectRelDirection(navVelocity)
		displayIndicator(onscreen, position, direction, icons.prograde, colors.navTarget, true, lui.HUD_INDICATOR_NAV_TARGET_PROGRADE)
		onscreen,position,direction = Engine.ProjectRelDirection(-navVelocity)
		displayIndicator(onscreen, position, direction, icons.retrograde, colors.navTarget, false, lui.HUD_INDICATOR_NAV_TARGET_RETROGRADE)
	end
end

-- Returns the minimum distance (in meters) from the target that the given navtunnel frame should display at
local function squareDist(scalingFactor, num)
	return math.pow(scalingFactor, num - 1) * num * 10.0 -- first square at ten meters
end

-- Given a navtunnel frame's distance from the target and the distance of the target from the camera,
-- return the screen height of the corresponding navtunnel frame
local function squareHeight(distance, distToDest)
	return distance * (ui.screenHeight / distToDest)
end

local function displayNavTunnels(navTarget)
	local _, position, _, behindCamera = Engine.GetTargetIndicatorScreenPosition(navTarget)
	if behindCamera then return end

	local targetDist = player:GetPositionRelTo(navTarget):length()
	local screenDiff = position - Vector2(ui.screenWidth / 2, ui.screenHeight / 2)

	-- exponential distribution of navtunnel frames
	-- the closer to 1.0, the more frames are drawn for a large distance
	local scalingFactor = 1.6
	local size = Vector2(0.9, ui.screenHeight / ui.screenWidth)
	-- TODO: a lot of iterations are wasted on very tiny frames, could be improved
	for i = 1, 100 do
		local dist = squareDist(scalingFactor, i)
		if (dist > targetDist) then break end

		local scrHeight = squareHeight(dist, targetDist)
		if scrHeight > 10.0 then
			-- as the frame gets further away from the target, bring it closer to the center of the screen
			local rectCenter = position - screenDiff * (dist / targetDist)
			ui.addRect(rectCenter - size * scrHeight, rectCenter + size * scrHeight, colors.navTargetDark, 0, 0, 1.0)
		end
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
			if Engine.GetDisplayNavTunnels() then
				displayNavTunnels(navTarget)
			end
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
