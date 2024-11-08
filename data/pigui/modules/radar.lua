-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Game = require 'Game'
local utils = require 'utils'
local Event = require 'Event'
local Input = require 'Input'

local Lang = require 'Lang'
local lc = Lang.GetResource("core");
local lui = Lang.GetResource("ui-core");

local ui = require 'pigui'

local player = nil
local pionillium = ui.fonts.pionillium
local colors = ui.theme.colors
local icons = ui.theme.icons

local MAX_RADAR_SIZE = 1000000000
local MIN_RADAR_SIZE = 1000
local DEFAULT_RADAR_SIZE = 10000

local shouldDisplay2DRadar
local current_radar_size = DEFAULT_RADAR_SIZE
local manual_zoom = false
local blobSize = 6.0

local keys = {
	radar_toggle_mode = Input.GetActionBinding('BindRadarToggleMode'),
	radar_reset = Input.GetActionBinding('BindRadarZoomReset'),
	-- TODO: Convert to Axis?
	radar_zoom_in = Input.GetActionBinding('BindRadarZoomIn'),
	radar_zoom_out = Input.GetActionBinding('BindRadarZoomOut'),
}

local function getColorFor(item)
	local body = item.body
	if body == player:GetNavTarget() then
		return colors.radarNavTarget
	elseif body == player:GetCombatTarget() then
		return colors.radarCombatTarget
	elseif body:IsShip() then
		return colors.radarShip
	elseif body:IsHyperspaceCloud() then
		return colors.radarCloud
	elseif body:IsMissile() then
		return colors.radarMissile
	elseif body:IsStation() then
		return colors.radarStation
	elseif body:IsCargoContainer() then
		return colors.radarCargo
	end
	return colors.radarUnknown
end

-- display the 2D radar
local function display2DRadar(cntr, size)
	local targets = ui.getTargetsNearby(current_radar_size)
	local halfsize = size * 0.5
	local thirdsize = size * 0.3
	local twothirdsize = size * 0.7
	local fgColor = colors.uiPrimary
	local bgColor = colors.uiBackground:opacity(0.54)
	local lineThickness = 1.5

	local function line(x,y)
		-- Uncomment to extend the radial line all the way to the outer circle
		-- ui.addLine(cntr + Vector2(x, y) * halfsize, center + Vector2(x,y) * size, colors.uiPrimaryDark, lineThickness)
		ui.addLine(cntr + Vector2(x, y) * thirdsize, cntr + Vector2(x,y) * twothirdsize, fgColor, lineThickness)
	end

	-- radar background and border
	ui.addCircleFilled(cntr, size, bgColor, ui.circleSegments(size), 1)
	ui.addCircle(cntr, size, fgColor, ui.circleSegments(size), lineThickness * 2)

	-- inner circles
	-- Uncomment to add an additional circle dividing the 2D radar display
	--	ui.addCircle(cntr, halfsize, fgColor, ui.circleSegments(halfsize), lineThickness)
	ui.addCircle(cntr, thirdsize, fgColor, ui.circleSegments(thirdsize), lineThickness)
	ui.addCircle(cntr, twothirdsize, fgColor, ui.circleSegments(twothirdsize), lineThickness)
	local l = ui.oneOverSqrtTwo
	-- cross-lines
	line(-l, l)
	line(-l, -l)
	line(l, -l)
	line(l, l)

	-- Draw target markers
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
				ui.addIcon(position, icons.square, color, Vector2(12, 12), ui.anchor.center, ui.anchor.center)
			elseif v.body == combatTarget then
				local color = Color(colors.combatTarget.r, colors.combatTarget.g, colors.combatTarget.b, alpha)
				ui.addIcon(position, icons.square, color, Vector2(12, 12), ui.anchor.center, ui.anchor.center)
			else
				local color = getColorFor(v)
				ui.addCircleFilled(position, 2, color, 4, 1)
			end
			local mouse_position = ui.getMousePos()
			if (mouse_position - position):length() < 4 then
				table.insert(tooltip, v.label)
			end
		end
	end
	if #tooltip > 0 then
		ui.setTooltip(table.concat(tooltip, "\n"))
	end
	-- local distance = ui.Format.Distance(current_radar_size)
	-- local textcenter = cntr + Vector2((halfsize + twothirdsize) * 0.5, size)
	-- local textsize = ui.addStyledText(textcenter, ui.anchor.left, ui.anchor.bottom, distance, colors.frame, pionillium.small, lui.HUD_RADAR_DISTANCE, colors.lightBlackBackground)
end

-- Return tooltip for target
local function drawTarget(target, scale, center, color)
	local pos = target.rel_position
	local basePos = Vector2(pos.x * scale.x + center.x, pos.z * scale.y + center.y)
	local blobPos = Vector2(basePos.x, basePos.y - pos.y * scale.y);
	local blobHalfSize = Vector2(blobSize / 2)
	local tooltip = {}

	ui.addLine(basePos, blobPos, color:shade(0.1), 2)
	ui.addRectFilled(blobPos - blobHalfSize, blobPos + blobHalfSize, color, 0, 0)
	local mouse_position = ui.getMousePos()
	if (mouse_position - blobPos):length() < 4 then
		table.insert(tooltip, target.label)
	end
	return tooltip
end

local radar = require 'PiGui.Modules.RadarWidget'()
--local currentZoomDist = MIN_RADAR_SIZE
radar.minZoom = MIN_RADAR_SIZE
radar.maxZoom = MAX_RADAR_SIZE

local function display3DRadar(center, size)
	local targets = ui.getTargetsNearby(MAX_RADAR_SIZE)
	local tooltip = {}

	local combatTarget = player:GetCombatTarget()
	local navTarget = player:GetNavTarget()
	local maxBodyDist = 0.0
	local maxShipDist = 0.0
	local maxCargoDist = 0.0

	radar.size = size
	radar.zoom = current_radar_size or DEFAULT_RADAR_SIZE
	local radius = radar.radius
	local scale = radar.radius / radar.zoom
	ui.setCursorPos(center - size / 2.0)

	-- draw targets below the plane
	for k, v in pairs(targets) do
		-- collect some values for zoom updates later
		maxBodyDist = math.max(maxBodyDist, v.distance)
		-- only snap to ships if they're less than 50km away (arbitrary constant based on crime range)
		if v.body:IsShip() and v.distance < 50000 then maxShipDist = math.max(maxShipDist, v.distance) end

		-- only snap to cargo containers if they're less than 25km away (arbitrary)
		if v.body:IsCargoContainer() and v.distance < 25000 then maxCargoDist = math.max(maxCargoDist, v.distance) end

		if v.distance < current_radar_size and v.rel_position.y < 0.0 then
			local color = (v.body == navTarget and colors.navTarget) or (v.body == combatTarget and colors.combatTarget) or getColorFor(v)
			table.append(tooltip, drawTarget(v, scale, center, color))
		end
	end

	ui.withStyleColors({
		FrameBg = colors.radarBackground,
		FrameBgActive = colors.radarFrame,
	}, function()
		-- draw the radar plane itself
		radar:Draw()
	end)


	-- draw targets above the plane
	for k, v in pairs(targets) do
		if v.distance < current_radar_size and v.rel_position.y >= 0.0 then
			local color = (v.body == navTarget and colors.navTarget) or (v.body == combatTarget and colors.combatTarget) or getColorFor(v)
			table.append(tooltip, drawTarget(v, scale, center, color))
		end
	end

	if #tooltip > 0 then
		ui.setTooltip(table.concat(tooltip, "\n"))
	end

	-- handle automatic radar zoom based on player surroundings
	if not manual_zoom then
		local maxDist = maxBodyDist
		if combatTarget then
			maxDist = combatTarget:GetPositionRelTo(player):length() * 1.4
		elseif maxShipDist > 0 then
			maxDist = maxShipDist * 1.4
		elseif maxCargoDist > 0 then
		maxDist = maxCargoDist * 1.4
		elseif navTarget then
			local dist = navTarget:GetPositionRelTo(player):length()
			maxDist = dist > MAX_RADAR_SIZE and maxBodyDist or dist * 1.4
		end
		current_radar_size = math.clamp(radar.zoom + (maxDist - radar.zoom) * 0.03,
			MIN_RADAR_SIZE, MAX_RADAR_SIZE)
	end

	-- local distance = ui.Format.Distance(current_radar_size)
	-- local textwidth = ui.calcTextSize(distance).x
	-- local textpos = center + Vector2(textwidth / -2, size.y * 0.42)
	-- local textsize = ui.addStyledText(textpos, ui.anchor.left, ui.anchor.bottom, distance, colors.frame, pionillium.small, lui.HUD_RADAR_DISTANCE, colors.lightBlackBackground)
end

local click_on_radar = false
-- display either the 3D or the 2D radar, show a popup on right click to select
local function displayRadar()
	if ui.optionsWindow.isOpen or Game.CurrentView() ~= "world" then return end
	player = Game.player

	-- only display if there actually *is* a radar installed
	local equipped_radar = player:GetComponent("EquipSet"):GetInstalledOfType("sensor.radar")
	if #equipped_radar > 0 then

		local size = ui.reticuleCircleRadius * 0.66
		local center = Vector2(ui.screenWidth / 2, ui.screenHeight - size - 4)
		local zoom = 0
		local toggle_radar = false

		-- Handle keyboard
		-- TODO: Convert to axis?
		if keys.radar_zoom_in:IsJustActive() then
			zoom = 1
		elseif keys.radar_zoom_out:IsJustActive() then
			zoom = -1
		end
		if keys.radar_reset:IsJustActive() then
			zoom = 0
			manual_zoom = false
			current_radar_size = DEFAULT_RADAR_SIZE
		end
		if keys.radar_toggle_mode:IsJustActive() then
			toggle_radar = true
		end

		-- Handle mouse if it is in the radar area
		local mp = ui.getMousePos()
		local mouse_dist = shouldDisplay2DRadar and size or size * 1.8
		if (mp - center):length() > mouse_dist then
			click_on_radar = false
		end
		if (mp - center):length() < mouse_dist then
			if ui.isMouseClicked(1) then
				click_on_radar = true
			end
			if not toggle_radar and click_on_radar and ui.isMouseReleased(1) then
				ui.openPopup("radarselector")
			end
			-- TODO: figure out how to "capture" the mouse wheel to prevent
			-- the game engine from using it to also zoom the viewport
			if zoom == 0 then
				zoom = ui.getMouseWheel()
			end
		end
		if zoom > 0 then
			-- Zoom in (decrease scanned area)
			if not manual_zoom then
				current_radar_size = 10 ^ math.floor(math.log(current_radar_size, 10))
			else
				current_radar_size = math.max(current_radar_size / 10, MIN_RADAR_SIZE)
			end
			manual_zoom = true
		elseif zoom < 0 then
			-- Zoom out (increase scanned area)
			if not manual_zoom then
				current_radar_size = 10 ^ math.ceil(math.log(current_radar_size, 10))
			else
				current_radar_size = math.min(current_radar_size * 10, MAX_RADAR_SIZE)
			end
			manual_zoom = true
		end
		ui.popup("radarselector", function()
			if ui.selectable(lui.HUD_2D_RADAR, shouldDisplay2DRadar, {}) then
				Event.Queue('onChangeMFD', 'radar')
			end
			if ui.selectable(lui.HUD_3D_RADAR, not shouldDisplay2DRadar, {}) then
				Event.Queue('onChangeMFD', 'scanner')
			end
		end)

		if toggle_radar then
			shouldDisplay2DRadar = not shouldDisplay2DRadar
			Event.Queue('onChangeMFD', shouldDisplay2DRadar and 'radar' or 'scanner')
		end
		-- Draw the actual radar
		if shouldDisplay2DRadar then
			display2DRadar(center, size)
		else
			display3DRadar(center, Vector2(ui.reticuleCircleRadius * 1.8, size * 2))
		end
		-- Draw the range indicator
		local distance = ui.Format.Distance(current_radar_size)
		local textpos = Vector2(center.x + size, center.y + size)
		local textsize = ui.addStyledText(textpos, ui.anchor.right, ui.anchor.bottom, distance, colors.frame, pionillium.small, lui.HUD_RADAR_DISTANCE, colors.lightBlackBackground)
		-- Draw the radar mode in bottom-left corner
		-- TODO: use an icon?
		local mode = manual_zoom and '[M]' or '[A]'
		textpos = Vector2(center.x - size, center.y + size)
		ui.addStyledText(textpos, ui.anchor.left, ui.anchor.bottom, mode, colors.alertRed, pionillium.small, lui.HUD_RADAR_DISTANCE, colors.lightBlackBackground)
	end
end

Event.Register('onChangeMFD', function(selected)
	shouldDisplay2DRadar = selected == "radar";
end)

-- reset radar to default at game end
Event.Register("onGameEnd", function() shouldDisplay2DRadar = false end)

-- save/load preference
require 'Serializer':Register("PiguiRadar",
	function () return { shouldDisplay2DRadar = shouldDisplay2DRadar } end,
	function (data) shouldDisplay2DRadar = data.shouldDisplay2DRadar end)

ui.registerModule("game", {
	id = "game-view-radar-module",
	draw = displayRadar,
	debugReload = function()
		print("Radar module reloading..")
		package.reimport()
	end
})

return {}
