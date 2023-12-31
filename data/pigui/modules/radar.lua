-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Game = require 'Game'
local utils = require 'utils'
local Event = require 'Event'

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

local shouldDisplay2DRadar
local current_radar_size = 10000
local blobSize = 6.0

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
	return colors.grey
end

-- display the 2D radar
local function display2DRadar(cntr, size)
	local targets = ui.getTargetsNearby(current_radar_size)
	local halfsize = size * 0.5
	local thirdsize = size * 0.3
	local twothirdsize = size * 0.7

	local function line(x,y)
		-- ui.addLine(cntr + Vector2(x, y) * halfsize, cntr + Vector2(x,y) * size, colors.reticuleCircle, ui.reticuleCircleThickness)
		ui.addLine(cntr + Vector2(x, y) * thirdsize, cntr + Vector2(x,y) * twothirdsize, colors.reticuleCircle, ui.reticuleCircleThickness)
	end
	ui.addCircleFilled(cntr, size, colors.lightBlueBackground, ui.circleSegments(size), 1)
	ui.addCircle(cntr, size, colors.reticuleCircle, ui.circleSegments(size), ui.reticuleCircleThickness)
	--	ui.addCircle(cntr, halfsize, colors.reticuleCircle, ui.circleSegments(halfsize), ui.reticuleCircleThickness)
	ui.addCircle(cntr, thirdsize, colors.reticuleCircle, ui.circleSegments(thirdsize), ui.reticuleCircleThickness)
	ui.addCircle(cntr, twothirdsize, colors.reticuleCircle, ui.circleSegments(twothirdsize), ui.reticuleCircleThickness)
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
	local distance = ui.Format.Distance(current_radar_size)
	local textcenter = cntr + Vector2((halfsize + twothirdsize) * 0.5, size)
	local textsize = ui.addStyledText(textcenter, ui.anchor.left, ui.anchor.bottom, distance, colors.frame, pionillium.small, lui.HUD_RADAR_DISTANCE, colors.lightBlackBackground)
end

local function drawTarget(target, scale, center, color)
	local pos = target.rel_position
	local basePos = Vector2(pos.x * scale.x + center.x, pos.z * scale.y + center.y)
	local blobPos = Vector2(basePos.x, basePos.y - pos.y * scale.y);
	local blobHalfSize = Vector2(blobSize / 2)

	ui.addLine(basePos, blobPos, color:shade(0.1), 2)
	ui.addRectFilled(blobPos - blobHalfSize, blobPos + blobHalfSize, color, 0, 0)
end

local radar = require 'PiGui.Modules.RadarWidget'()
local currentZoomDist = MIN_RADAR_SIZE
radar.minZoom = MIN_RADAR_SIZE
radar.maxZoom = MAX_RADAR_SIZE

local function display3DRadar(center, size)
	local targets = ui.getTargetsNearby(MAX_RADAR_SIZE)

	local combatTarget = player:GetCombatTarget()
	local navTarget = player:GetNavTarget()
	local maxBodyDist = 0.0
	local maxShipDist = 0.0

	radar.size = size
	radar.zoom = currentZoomDist
	local radius = radar.radius
	local scale = radar.radius / currentZoomDist
	ui.setCursorPos(center - size / 2.0)

	-- draw targets below the plane
	for k, v in pairs(targets) do
		-- collect some values for zoom updates later
		maxBodyDist = math.max(maxBodyDist, v.distance)
		-- only snap to ships if they're less than 50km away (arbitrary constant based on crime range)
		if v.body:IsShip() and v.distance < 50000 then maxShipDist = math.max(maxShipDist, v.distance) end

		if v.distance < currentZoomDist and v.rel_position.y < 0.0 then
			local color = (v.body == navTarget and colors.navTarget) or (v.body == combatTarget and colors.combatTarget) or getColorFor(v)
			drawTarget(v, scale, center, color)
		end
	end

	-- draw the radar plane itself
	radar:Draw()

	-- draw targets above the plane
	for k, v in pairs(targets) do
		if v.distance < currentZoomDist and v.rel_position.y >= 0.0 then
			local color = (v.body == navTarget and colors.navTarget) or (v.body == combatTarget and colors.combatTarget) or getColorFor(v)
			drawTarget(v, scale, center, color)
		end
	end

	-- handle automatic radar zoom based on player surroundings
	local maxDist = maxBodyDist
	if combatTarget then
		maxDist = combatTarget:GetPositionRelTo(player):length() * 1.4
	elseif maxShipDist > 0 then
		maxDist = maxShipDist * 1.4
	elseif navTarget then
		local dist = navTarget:GetPositionRelTo(player):length()
		maxDist = dist > MAX_RADAR_SIZE and maxBodyDist or dist * 1.4
	end

	currentZoomDist = math.clamp(currentZoomDist + (maxDist - currentZoomDist) * 0.03,
		MIN_RADAR_SIZE, MAX_RADAR_SIZE)
end

local click_on_radar = false
-- display either the 3D or the 2D radar, show a popup on right click to select
local function displayRadar()
	if ui.optionsWindow.isOpen or Game.CurrentView() ~= "world" then return end
	player = Game.player
	local equipped_radar = player:GetEquip("radar")
	-- only display if there actually *is* a radar installed
	if #equipped_radar > 0 then

		local size = ui.reticuleCircleRadius * 0.66
		local cntr = Vector2(ui.screenWidth / 2, ui.screenHeight - size - 4)

		local mp = ui.getMousePos()
		if (mp - cntr):length() > size then
			click_on_radar = false
		end
		if (mp - cntr):length() < size then
			if ui.isMouseClicked(1) then
				click_on_radar = true
			end
			if click_on_radar and ui.isMouseReleased(1) then
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
				Event.Queue('onChangeMFD', 'radar')
			end
			if ui.selectable(lui.HUD_3D_RADAR, not shouldDisplay2DRadar, {}) then
				Event.Queue('onChangeMFD', 'scanner')
			end
		end)
		if shouldDisplay2DRadar then
			display2DRadar(cntr, size)
		else
			display3DRadar(cntr, Vector2(ui.reticuleCircleRadius * 1.8, size * 2))
		end
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

ui.registerModule("game", displayRadar)

return {}
