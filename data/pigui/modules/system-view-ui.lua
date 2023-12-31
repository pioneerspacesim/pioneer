-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local Engine = require 'Engine'
local Event = require 'Event'
local Lang = require 'Lang'
local ui = require 'pigui'
local Format = require 'Format'
local SpaceStation = require 'SpaceStation'
local Constants = _G.Constants

local Vector2 = _G.Vector2
local lc = Lang.GetResource("core")
local luc = Lang.GetResource("ui-core")
local layout = require 'pigui.libs.window-layout'
local Sidebar = require 'pigui.libs.sidebar'

local player = nil
local colors = ui.theme.colors
local icons = ui.theme.icons
local styles = ui.theme.styles

local systemView = Game and Game.systemView -- for hot-reload

local indicatorSize = Vector2(30, 30)
local bodyIconSize = Vector2(18, 18)

local selectedObject -- object, centered in SystemView

local hudfont = ui.fonts.pionillium.small
local hudfont_highlight = ui.fonts.pionillium.medium
local detailfont = ui.fonts.pionillium.medium
local winfont = ui.fonts.pionillium.medlarge

local atlasfont = ui.fonts.pionillium.medium
local atlasfont_highlight = ui.fonts.pionillium.medlarge

local atlas_line_length = ui.rescaleUI(24)
local atlas_label_offset = ui.rescaleUI(Vector2(12, -8))

--load enums Projectable::types and Projectable::bases in one table "Projectable"
local Projectable = {}
for _, key in pairs(Constants.ProjectableTypes) do Projectable[key] = Engine.GetEnumValue("ProjectableTypes", key) end
for _, key in pairs(Constants.ProjectableBases) do Projectable[key] = Engine.GetEnumValue("ProjectableBases", key) end

-- all colors, used in this module
local svColor = {
	COMBAT_TARGET = colors.combatTarget,
	FONT = colors.font,
	GRID = colors.systemMapGrid,
	GRID_LEG = colors.systemMapGridLeg,
	LAGRANGE = colors.systemMapLagrangePoint,
	NAV_TARGET = colors.navTarget,
	OBJECT = colors.systemMapObject,
	PLANNER = colors.systemMapPlanner,
	PLANNER_ORBIT = colors.systemMapPlannerOrbit,
	PLAYER = colors.systemMapPlayer,
	PLAYER_ORBIT = colors.systemMapPlayerOrbit,
	SELECTED = ui.theme.styleColors.gray_200,
	SELECTED_SHIP_ORBIT = colors.systemMapSelectedShipOrbit,
	SHIP = colors.systemMapShip,
	SHIP_ORBIT = colors.systemMapShipOrbit,
	SYSTEMBODY = colors.systemMapSystemBody,
	SYSTEMBODY_ICON = colors.systemMapSystemBodyIcon,
	SYSTEMBODY_ORBIT = colors.systemMapSystemBodyOrbit,
	UNKNOWN = colors.unknown
}

-- button states
local function loop3items(a, b, c) return a, { [a] = b, [b] = c, [c] = a } end
local colorset = ui.theme.buttonColors

local buttonState = {
	SHIPS_OFF     = { icon = icons.ships_no_orbits,    state = colorset.transparent },
	SHIPS_ON      = { icon = icons.ships_no_orbits,    state = colorset.semi_transparent },
	SHIPS_ORBITS  = { icon = icons.ships_with_orbits },
	LAG_OFF       = { icon = icons.lagrange_no_text,   state = colorset.transparent },
	LAG_ICON      = { icon = icons.lagrange_no_text,   state = colorset.semi_transparent },
	LAG_ICONTEXT  = { icon = icons.lagrange_with_text },
	GRID_OFF      = { icon = icons.toggle_grid,        state = colorset.transparent },
	GRID_ON       = { icon = icons.toggle_grid,        state = colorset.semi_transparent },
	GRID_AND_LEGS = { icon = icons.toggle_grid },
	[true]        = {                                  state = colorset.default },
	[false]       = {                                  state = colorset.transparent },
	DISABLED      = {                                  state = colorset.semi_transparent }
}

local ship_drawing,  nextShipDrawings = loop3items("SHIPS_OFF", "SHIPS_ON", "SHIPS_ORBITS")
local show_lagrange, nextShowLagrange = loop3items("LAG_OFF", "LAG_ICON", "LAG_ICONTEXT")
local show_grid,     nextShowGrid     = loop3items("GRID_OFF", "GRID_ON", "GRID_AND_LEGS")

local onGameStart = function ()
	--connect to class SystemView
	systemView = Game.systemView
	--export several colors to class SystemView (only those which mentioned in the enum SystemViewColorIndex)
	for _, key in pairs(Constants.SystemViewColorIndex) do
		systemView:SetColor(key, svColor[key])
	end
	-- update visibility states
	systemView:SetVisibility(ship_drawing)
	systemView:SetVisibility(show_lagrange)
	systemView:SetVisibility(show_grid)
end

local onEnterSystem = function (ship)
	if ship == Game.player then
		Game.systemView:SetVisibility("RESET_VIEW");
	end
end

local function textIcon(icon, tooltip)
	ui.icon(icon, Vector2(ui.getTextLineHeight()), svColor.FONT, tooltip)
	ui.sameLine()
end

local function showDvLine(leftIcon, resetIcon, rightIcon, key, Formatter, leftTooltip, resetTooltip, rightTooltip)
	local wheel = function()
		if ui.isItemHovered() then
			local w = ui.getMouseWheel()
			if w ~= 0 then
				systemView:TransferPlannerAdd(key, w * 10)
			end
		end
	end
	local id =  "##" .. key

	local press = ui.mainMenuButton(leftIcon, leftTooltip..id)
	if press or (key ~= "factor" and ui.isItemActive()) then
		systemView:TransferPlannerAdd(key, -10)
	end
	wheel()
	ui.sameLine()
	if ui.mainMenuButton(resetIcon, resetTooltip..id) then
		systemView:TransferPlannerReset(key)
	end
	wheel()
	ui.sameLine()
	press = ui.mainMenuButton(rightIcon, rightTooltip..id)
	if press or (key ~= "factor" and ui.isItemActive()) then
		systemView:TransferPlannerAdd(key, 10)
	end
	wheel()
	ui.sameLine()
	local speed, speed_unit = Formatter(systemView:TransferPlannerGet(key))
	ui.text(speed .. " " .. speed_unit)
	return 0
end

local time_selected_button_icon = icons.time_center

local function timeButton(icon, tooltip, factor)
	if ui.mainMenuButton(icon, tooltip) then
		time_selected_button_icon = icon
	end
	local active = ui.isItemActive()
	if active then
		systemView:AccelerateTime(factor)
	end
	ui.sameLine()
	return active
end

-- all windows in this view
local Windows = {
	systemName = layout.NewWindow("SystemMapSystemName"),
	objectInfo = layout.NewWindow("SystemMapObjectIngo"),
	edgeButtons = layout.NewWindow("SystemMapEdgeButtons"),
	orbitPlanner = layout.NewWindow("SystemMapOrbitPlanner"),
	timeButtons = layout.NewWindow("SystemMapTimeButtons"),
	unexplored = layout.NewWindow("SystemMapUnexplored")
}

Windows.systemName.style_colors["WindowBg"] = colors.transparent

local systemViewLayout = layout.New(Windows)
systemViewLayout.mainFont = winfont

local leftSidebar = Sidebar.New("##SidebarL", "left")

local systemOverviewWidget = require 'pigui.modules.system-overview-window'.New()
systemOverviewWidget.visible = true

function systemOverviewWidget:onBodySelected(sBody)
	systemView:SetSelectedObject(Projectable.OBJECT, Projectable.SYSTEMBODY, sBody)
end

function systemOverviewWidget:onBodyDoubleClicked(sBody)
	systemView:ViewSelectedObject()
end

table.insert(leftSidebar.modules, {
	priority = 1,
	side = "left",
	showInHyperspace = false,
	icon = icons.system_overview,
	tooltip = luc.TOGGLE_OVERVIEW_WINDOW,
	exclusive = true,

	drawTitle = function()
		if Windows.unexplored.visible then
			ui.text(lc.UNEXPLORED_SYSTEM_NO_SYSTEM_VIEW)
		else
			systemOverviewWidget:displaySidebarTitle(systemView:GetSystem())
		end
	end,

	drawBody = function()
		local system = systemView:GetSystem() ---@type StarSystem
		if not system or Windows.unexplored.visible then
			return
		end

		systemOverviewWidget:displaySearch()

		systemOverviewWidget.size.y = math.max(ui.getContentRegion().y, ui.screenHeight / 1.6)

		local root = system.rootSystemBody
		local selected = { [systemView:GetSelectedObject().ref or 0] = true }

		systemOverviewWidget:display(Game.system, root, selected)
	end
})

local systemEconView = require 'pigui.modules.system-econ-view'.New()

table.insert(leftSidebar.modules, {
	priority = 2,
	side = "left",
	showInHyperspace = false,
	icon = icons.money,
	tooltip = luc.ECONOMY_TRADE,
	exclusive = true,

	drawTitle = function()
		ui.text(luc.ECONOMY_TRADE)
	end,

	drawBody = function()
		local selected = systemView:GetSelectedObject().ref ---@type SystemBody?
		local docked = Game.player:GetDockedWith()
		local current

		if docked then
			if not selected or not selected.isStation then
				selected = docked:GetSystemBody()
			else
				current = docked:GetSystemBody()
			end
		end

		if not selected or not selected.isStation then
			systemEconView:drawSystemComparison(systemView:GetSystem())
		else
			systemEconView:drawStationComparison(selected, current)
		end
	end
})

local function drawWindowControlButton(window, icon, tooltip)
	local isWindowActive = true
	if window.ShouldShow then isWindowActive = window:ShouldShow() end

	-- tristate: invisible, inactive, visible
	local state = (isWindowActive or not window.visible) and buttonState[window.visible].state or buttonState['DISABLED'].state
	if ui.mainMenuButton(icon, tooltip, state) then
		window.visible = not window.visible
	end
end

function Windows.edgeButtons.Show()
	local isOrrery = systemView:GetDisplayMode() == "Orrery"
	-- view control buttons
	if ui.mainMenuButton(icons.reset_view, luc.RESET_ORIENTATION_AND_ZOOM) then
		systemView:SetVisibility("RESET_VIEW")
	end
	ui.mainMenuButton(icons.rotate_view, luc.ROTATE_VIEW)
	systemView:SetRotateMode(ui.isItemActive())
	ui.mainMenuButton(icons.search_lens, luc.ZOOM)
	systemView:SetZoomMode(ui.isItemActive())

	if isOrrery and ui.mainMenuButton(icons.system_overview, luc.HUD_BUTTON_SWITCH_TO_SYSTEM_OVERVIEW) then
		systemView:SetDisplayMode('Atlas')
	end
	if not isOrrery and ui.mainMenuButton(icons.system_map, luc.HUD_BUTTON_SWITCH_TO_SYSTEM_MAP) then
		systemView:SetDisplayMode('Orrery')
	end
	ui.newLine()
	-- visibility control buttons
	if isOrrery then
		if ui.mainMenuButton(buttonState[ship_drawing].icon, lc.SHIPS_DISPLAY_MODE_TOGGLE, buttonState[ship_drawing].state) then
			ship_drawing = nextShipDrawings[ship_drawing]
			systemView:SetVisibility(ship_drawing)
		end
		if ui.mainMenuButton(buttonState[show_lagrange].icon, lc.L4L5_DISPLAY_MODE_TOGGLE, buttonState[show_lagrange].state) then
			show_lagrange = nextShowLagrange[show_lagrange]
			systemView:SetVisibility(show_lagrange)
		end
		if ui.mainMenuButton(buttonState[show_grid].icon, lc.GRID_DISPLAY_MODE_TOGGLE, buttonState[show_grid].state) then
			show_grid = nextShowGrid[show_grid]
			systemView:SetVisibility(show_grid)
		end
		ui.newLine()
	end

	drawWindowControlButton(Windows.objectInfo, icons.info, lc.OBJECT_INFO)
	if isOrrery then
		drawWindowControlButton(Windows.orbitPlanner, icons.semi_major_axis, lc.ORBIT_PLANNER)
	end
end

function Windows.orbitPlanner.ShouldShow()
	return systemView:GetDisplayMode() == 'Orrery'
end

function Windows.timeButtons.ShouldShow()
	return systemView:GetDisplayMode() == 'Orrery'
end

function Windows.orbitPlanner.Show()
	textIcon(icons.semi_major_axis)
	ui.text(lc.ORBIT_PLANNER)
	ui.separator()
	showDvLine(icons.decrease, icons.delta, icons.increase, "factor", function(i) return i, "x" end, luc.DECREASE, lc.PLANNER_RESET_FACTOR, luc.INCREASE)
	showDvLine(icons.decrease, icons.clock, icons.increase, "starttime",
		function(_)
			local now = Game.time
			local start = systemView:GetOrbitPlannerStartTime()
			if start then
				return ui.Format.Duration(math.floor(start - now)), ""
			else
				return lc.NOW, ""
			end
		end,
		luc.DECREASE, lc.PLANNER_RESET_START, luc.INCREASE)
	showDvLine(icons.decrease, icons.orbit_prograde, icons.increase, "prograde", ui.Format.SpeedUnit, luc.DECREASE, lc.PLANNER_RESET_PROGRADE, luc.INCREASE)
	showDvLine(icons.decrease, icons.orbit_normal, icons.increase, "normal", ui.Format.SpeedUnit, luc.DECREASE, lc.PLANNER_RESET_NORMAL, luc.INCREASE)
	showDvLine(icons.decrease, icons.orbit_radial, icons.increase, "radial", ui.Format.SpeedUnit, luc.DECREASE, lc.PLANNER_RESET_RADIAL, luc.INCREASE)
end

function Windows.timeButtons.Show()
	local t = systemView:GetOrbitPlannerTime()
	ui.text(t and ui.Format.Datetime(t) or lc.NOW)
	local r = false
	r = timeButton(icons.time_backward_100x, "-10,000,000x",-10000000) or r
	r = timeButton(icons.time_backward_10x, "-100,000x", -100000) or r
	r = timeButton(icons.time_backward_1x, "-1,000x", -1000) or r
	r = timeButton(icons.time_center, lc.NOW, nil) or r
	r = timeButton(icons.time_forward_1x, "1,000x", 1000) or r
	r = timeButton(icons.time_forward_10x, "100,000x", 100000) or r
	r = timeButton(icons.time_forward_100x, "10,000,000x", 10000000) or r
	if not r then
		if time_selected_button_icon == icons.time_center then
			systemView:AccelerateTime(nil)
		else
			systemView:AccelerateTime(0.0)
		end
	end
end

local _getBodyIcon = require 'pigui.modules.flight-ui.body-icons'
local function getBodyIcon(obj, forWorld, isOrrery)
	if obj.type == Projectable.APOAPSIS then return icons.apoapsis
	elseif obj.type == Projectable.PERIAPSIS then return icons.periapsis
	elseif obj.type == Projectable.L4 then return icons.lagrange_marker
	elseif obj.type == Projectable.L5 then return icons.lagrange_marker
	elseif obj.base == Projectable.PLAYER or obj.base == Projectable.PLANNER then
		local shipClass = obj.ref:GetShipClass()
		if icons[shipClass] then
			return icons[shipClass]
		else
			return icons.ship
		end
	elseif forWorld and not isOrrery and obj.ref.superType ~= "STARPORT" and obj.ref.type ~= "PLANET_ASTEROID" then
		return icons.empty
	else
		return _getBodyIcon(obj.ref, forWorld)
	end
end

local function getLabel(obj)
	if obj.type == Projectable.OBJECT then
		if obj.base == Projectable.SYSTEMBODY then return obj.ref.name
		elseif obj.base == Projectable.PLANNER then return ""
		else return obj.ref:GetLabel() end
	elseif obj.type == Projectable.L4 and show_lagrange == "LAG_ICONTEXT" then return "L4"
	elseif obj.type == Projectable.L5 and show_lagrange == "LAG_ICONTEXT" then return "L5"
	else return ""
	end
end

local function getColor(obj)
	if obj.type == Projectable.OBJECT then
		if obj.base == Projectable.SYSTEMBODY then return svColor.SYSTEMBODY_ICON
		elseif obj.base == Projectable.SHIP then return svColor.SHIP
		elseif obj.base == Projectable.PLAYER then return svColor.PLAYER
		elseif obj.base == Projectable.PLANNER then return svColor.PLANNER
		else return svColor.OBJECT
		end
	elseif obj.type == Projectable.APOAPSIS or obj.type == Projectable.PERIAPSIS then
		if obj.base == Projectable.SYSTEMBODY then return svColor.SYSTEMBODY_ORBIT
		elseif obj.base == Projectable.SHIP then
			if obj.ref == selectedObject then return svColor.SELECTED_SHIP_ORBIT
			else return svColor.SHIP_ORBIT
			end
		elseif obj.base == Projectable.PLAYER then return svColor.PLAYER_ORBIT
		elseif obj.base == Projectable.PLANNER then return svColor.PLANNER_ORBIT
		else return svColor.UNKNOWN -- unknown base
		end
	elseif obj.type == Projectable.L4 or obj.type == Projectable.L5 then return svColor.LAGRANGE
	else return svColor.UNKNOWN
	end
end

function Windows.systemName.Show()
	local path = Game.sectorView:GetSelectedSystemPath()
	ui.text(ui.Format.SystemPath(path))
end

local function drawGroupIcons(coords, icon, color, iconSize, group, isSelected)
	-- indicators
	local stackedSize = indicatorSize
	local stackStep = Vector2(10, 10)
	if isSelected then
		ui.addIcon(coords, icons.square, svColor.SELECTED, stackedSize, ui.anchor.center, ui.anchor.center)
		stackedSize = stackedSize + stackStep
	end
	if group.hasPlayer then
		ui.addIcon(coords, icons.square, svColor.PLAYER, stackedSize, ui.anchor.center, ui.anchor.center)
		stackedSize = stackedSize + stackStep
	end
	if group.hasNavTarget then
		ui.addIcon(coords, icons.square, svColor.NAV_TARGET, stackedSize, ui.anchor.center, ui.anchor.center)
		stackedSize = stackedSize + stackStep
	end
	if group.hasCombatTarget then
		ui.addIcon(coords, icons.square, svColor.COMBAT_TARGET, stackedSize, ui.anchor.center, ui.anchor.center)
		stackedSize = stackedSize + stackStep
	end
	if group.hasPlanner then
		ui.addIcon(coords, icons.square, svColor.PLANNER, stackedSize, ui.anchor.center, ui.anchor.center)
		stackedSize = stackedSize + stackStep
	end

	ui.addIcon(coords, icon, color, iconSize, ui.anchor.center, ui.anchor.center)
end

-- handle positioning and drawing a label for the given object in Atlas mode
local function drawAtlasBodyLabel(label, screenSize, mainCoords, isHovered, isSelected)
	-- Larger font for hovered bodies, slight emphasis on the selected body
	local font = isHovered and atlasfont_highlight or atlasfont
	local fontColor = isSelected and colors.systemAtlasLabelActive or colors.systemAtlasLabel
	local lineColor = isSelected and colors.systemAtlasLineActive or colors.systemAtlasLine

	local textSize = ui.calcTextSize(label, font)
	-- lineOffset is half the screen-size radius of the body
	local lineOffsetSize = math.max(screenSize * 0.66, bodyIconSize.x * 0.5) -- most icons use about 60% of the actual radius
	-- lineLength is how long to draw the "pointer" line between the label and the edge of the body
	local lineLength = (atlas_line_length / math.max(systemView:GetZoom(), 1.0)) * (isHovered and 1.0 or 0.6)

	local lineStartPos = mainCoords + Vector2(lineOffsetSize, -lineOffsetSize * 0.667)
	local lineEndPos = lineStartPos + Vector2(lineLength, -lineLength)
	local underlinePos = lineEndPos + Vector2(textSize.x + atlas_label_offset.x * 2, 0)

	-- draw a background behind the label, then an indicator line
	if isHovered then
		ui.addRectFilled(lineEndPos - Vector2(0, -atlas_label_offset.y + textSize.y), underlinePos, colors.lightBlackBackground, 4, 0)
		ui.addLine(lineStartPos, lineEndPos, lineColor, 2)
		ui.addLine(lineEndPos, underlinePos, lineColor, 3)
	end

	-- draw the label and it's shadow for clarity
	local labelPos = (isHovered and lineEndPos or lineStartPos) + atlas_label_offset
	local shadowPos = labelPos + Vector2(2, 1)

	ui.addStyledText(shadowPos, ui.anchor.left, ui.anchor.baseline, label, colors.black, font)
	ui.addStyledText(labelPos, ui.anchor.left, ui.anchor.baseline, label, fontColor, font)
end

Windows.unexplored.visible = false
function Windows.unexplored.Show()
	ui.text(lc.UNEXPLORED_SYSTEM_NO_SYSTEM_VIEW)
end

local function checkSurfPorts(sbody)
	local children = sbody.children
	if #children == 0 then return nil end
	local ports = {}
	local has_indicators
	local navTarget = Game.player:GetNavTarget()
	local selected = systemView:GetSelectedObject().ref
	for _, child in ipairs(children) do
		if child.type == "STARPORT_SURFACE" then
			table.insert(ports, child)
			local body = child.physicsBody
			has_indicators = has_indicators or body and body == navTarget or child == selected
		end
	end
	if #ports == 0 then return nil end
	return ports, has_indicators
end

local function displaySurfPorts(center, radius, ports, mouseover)
	local nports = #ports
	if nports == 0 then return nil end
	local icon_size = bodyIconSize.x
	-- pretty rough estimate
	local min_rad = icon_size * nports / ui.twoPi
	local pixPerUnit = systemView:AtlasViewPixelPerUnit()
	local gap = systemView:AtlasViewPlanetGap(radius / pixPerUnit) * pixPerUnit
	local icons_radius = math.max(icon_size, min_rad, radius + icon_size / 2)
	local can_show_ring = icons_radius + icon_size / 2 < radius + gap or nports < 3

	local start_ang = math.deg2rad(225)
	local step_ang = ui.twoPi / nports
	local hover_i = 0
	local hover_center
	local select_i = 0
	local select_center
	local navtarget_in_ring = false
	for i = 1, nports do
		local ang = start_ang + step_ang * (i - 1)
		local p_center = center + Vector2(math.cos(ang), math.sin(ang)) * icons_radius
		-- HACK we pretend to be a group, to use drawGroupIcons
		local body = ports[i].physicsBody
		local group = { hasNavTarget = body and body == Game.player:GetNavTarget() }
		local isSelected = ports[i] == systemView:GetSelectedObject().ref
		if can_show_ring then
			drawGroupIcons(p_center, _getBodyIcon(ports[i]), svColor.SYSTEMBODY_ICON, bodyIconSize, group, isSelected)
			-- hover mouse
			if (ui.getMousePos() - p_center):length() < icon_size / 2 then
				hover_i = i
				hover_center = p_center
			end
		else
			navtarget_in_ring = navtarget_in_ring or group.hasNavTarget
		end
		if isSelected then
			select_i = i
			select_center = p_center
		end
	end
	if not can_show_ring then
		local p_center = center + Vector2(math.cos(start_ang), math.sin(start_ang)) * icons_radius
		local group = { hasNavTarget = navtarget_in_ring }
		drawGroupIcons(p_center, icons.plus, svColor.SYSTEMBODY_ICON, bodyIconSize, group, select_i ~= 0)
		return nil
	end
	if hover_i ~= 0 then
		return ports[hover_i], hover_center
	elseif select_i ~= 0 and not mouseover then
		return ports[select_i], select_center
	else
		return nil
	end
end

local popup_object
function makePopup()
	ui.popup("system-view-ui-popup", function()
		local isOrrery = systemView:GetDisplayMode() == "Orrery"
		local combatTarget = player:GetCombatTarget()
		local navTarget = Game.player:GetNavTarget()
		local isObject = popup_object.type == Projectable.OBJECT
		local isSystemBody = isObject and popup_object.base == Projectable.SYSTEMBODY
		local isShip = isObject and not isSystemBody and popup_object.ref:IsShip()
		ui.text(getLabel(popup_object))
		ui.separator()
		if isOrrery and ui.selectable(lc.CENTER, false, {}) then
			selectedObject = popup_object.ref
			systemView:SetSelectedObject(popup_object.type, popup_object.base, popup_object.ref)
			systemView:ViewSelectedObject()
		end
		if (isShip or isSystemBody and popup_object.ref.physicsBody) and ui.selectable(lc.SET_AS_TARGET, false, {}) then
			if isSystemBody then
				player:SetNavTarget(popup_object.ref.physicsBody)
				ui.playSfx("OK")
			else
				if combatTarget == popup_object.ref then player:SetCombatTarget(nil) end
				player:SetNavTarget(popup_object.ref)
				ui.playSfx("OK")
			end
		end
		if isShip and ui.selectable(lc.SET_AS_COMBAT_TARGET, false, {}) then
			if navTarget == popup_object.ref then player:SetNavTarget(nil) end
			player:SetCombatTarget(popup_object.ref)
		end
	end)
end

local expanded_hover_body = nil -- body with expanded hover area (atlas view only)
-- forked from data/pigui/views/game.lua
local function displayOnScreenObjects()
	local isOrrery = systemView:GetDisplayMode() == 'Orrery'

	local should_show_label = isOrrery and ui.shouldShowLabels()

	local label_offset = 14 -- enough so that the target rectangle fits
	local collapse = bodyIconSize -- size of clusters to be collapsed into single bodies
	local click_radius = collapse:length() * 0.5
	if not isOrrery then
		click_radius = collapse:length() * 0.8 / systemView:GetZoom()
	end
	-- make click_radius sufficiently smaller than the cluster size
	-- to prevent overlap of selection regions
	local objectCounter = 0
	local objects_grouped = systemView:GetProjectedGrouped(collapse, 1e64)

	-- if there's nothing to display, we're an unexplored system
	Windows.unexplored.visible = #objects_grouped == 0
	if Windows.unexplored.visible then return end

	local hoveredObject = nil
	local was_hovered = false
	local atlas_label_objects = {}

	for _,group in ipairs(objects_grouped) do
		local mainObject = group.mainObject

		local mainCoords = Vector2(group.screenCoordinates.x, group.screenCoordinates.y)
		local isSelected = mainObject.type == Projectable.OBJECT and mainObject.ref == systemView:GetSelectedObject().ref
		group.hasPlanner = mainObject.type == Projectable.OBJECT and mainObject.base == Projectable.PLANNER
		local hoverSize = group.screenSize

		local ports = {} -- ground ports
		local should_show_ports = false -- if we want to show ports even if we are not hovered
		if not isOrrery and mainObject.type == Projectable.OBJECT and mainObject.base == Projectable.SYSTEMBODY then
			-- should show ports if there is some indicators
			ports, should_show_ports = checkSurfPorts(mainObject.ref)
			-- or if the central body is selected
			should_show_ports = should_show_ports or isSelected
			-- if we hover over a body with ports, we expand its hover radius to make it easier to hover ports
			if mainObject.ref == expanded_hover_body or should_show_ports then hoverSize = group.screenSize + bodyIconSize.x end
		end

		drawGroupIcons(mainCoords, getBodyIcon(mainObject, true, isOrrery), getColor(mainObject), bodyIconSize, group, isSelected)

		local mp = ui.getMousePos()
		local label = getLabel(mainObject)
		local mouseover = not ui.isAnyWindowHovered() and
			(mp - mainCoords):length() < (isOrrery and click_radius or math.max(click_radius, hoverSize))

		if #label > 0 and (should_show_label or mouseover or should_show_ports) then
			if group.objects then
				label = label .. " (" .. #group.objects .. ")"
			end

			if isOrrery then
				local pos = mainCoords + Vector2(label_offset, 0)
				local hovered = mouseover and mainObject.type == Projectable.OBJECT
				local font = (hovered or isSelected) and hudfont_highlight or hudfont
				ui.addStyledText(pos + Vector2(2, 1), ui.anchor.left, ui.anchor.center, label , ui.theme.colors.black, font)
				ui.addStyledText(pos, ui.anchor.left, ui.anchor.center, label , getColor(mainObject), font)
			else
				local labeled = nil -- hovered or selected
				local labeled_center = nil
				if ports then
					if mouseover then
						was_hovered = true
						expanded_hover_body = mainObject.ref
					end
					labeled, labeled_center = displaySurfPorts(mainCoords, group.screenSize, ports, mouseover)
				end
				if labeled then
					-- HACK we just replace the current object with its selected or hovered ground port (if has)
					mainObject.type = Projectable.OBJECT
					mainObject.base = Projectable.SYSTEMBODY
					mainObject.ref = labeled
					mainCoords = labeled_center
					group.screenSize = bodyIconSize.x
					isSelected = labeled == systemView:GetSelectedObject().ref
					label = labeled.name
				end
				table.insert(atlas_label_objects, { label, group.screenSize, mainCoords, mouseover, isSelected })
			end
		end

		if mainObject.type == Projectable.OBJECT and (mainObject.base == Projectable.SYSTEMBODY or mainObject.base == Projectable.SHIP or mainObject.base == Projectable.PLAYER) then
			-- mouse release handler for right button
			if mouseover then
				if not ui.isAnyWindowHovered() and ui.isMouseReleased(1) then
					popup_object = mainObject
					ui.openPopup("system-view-ui-popup")
				end
			end
		end
		-- mouse release handler for left button
		if mouseover and mainObject.type == Projectable.OBJECT then
			hoveredObject = mainObject
		end
		objectCounter = objectCounter + 1
	end
	makePopup()
	if not was_hovered then expanded_hover_body = nil end

	-- atlas body labels have to be drawn after icons for proper ordering
	for _, v in ipairs(atlas_label_objects) do
		drawAtlasBodyLabel(table.unpack(v))
	end

	-- click once: select or deselect a body
	-- double click: zoom to body or reset viewpoint
	local clicked = not ui.isAnyWindowHovered() and (ui.isMouseClicked(0) or ui.isMouseDoubleClicked(0))
	if clicked then
		if hoveredObject then
			selectedObject = hoveredObject.ref
			systemView:SetSelectedObject(hoveredObject.type, hoveredObject.base, hoveredObject.ref)
			if ui.isMouseDoubleClicked(0) then systemView:ViewSelectedObject() end
		else
			selectedObject = nil
			systemView:ClearSelectedObject()
			if ui.isMouseDoubleClicked(0) then systemView:ResetViewpoint() end
		end
	end
end

local function tabular(data, maxSize)
	if data and #data > 0 then
		ui.columns(2, "Attributes", false)
		local nameWidth = 0
		local valueWidth = 0
		for _,item in pairs(data) do
			if item.value then
				local nWidth = ui.calcTextSize(item.name).x + ui.getItemSpacing().x
				local vWidth = ui.calcTextSize(item.value).x + ui.getItemSpacing().x
				if ui.getColumnWidth() < nWidth then
					textIcon(item.icon or icons.info, item.name)
				else
					ui.text(item.name)
				end
				ui.nextColumn()
				ui.text(item.value)
				ui.nextColumn()

				nameWidth = math.max(nameWidth, nWidth)
				valueWidth = math.max(valueWidth, vWidth)
			end
		end
		if nameWidth + valueWidth > maxSize then
			-- first of all, we want to see the values, but the keys should not be too small either
			nameWidth = math.max(maxSize - valueWidth, maxSize * 0.1)
		end
		ui.setColumnWidth(0, nameWidth)
	end
end

function Windows.objectInfo.ShouldShow()
	local obj = systemView:GetSelectedObject()

	if obj.type ~= Projectable.OBJECT or obj.base ~= Projectable.SHIP and obj.base ~= Projectable.SYSTEMBODY then
		return false
	end

	return true
end

function Windows.objectInfo:Show()
	local obj = systemView:GetSelectedObject()
	local isSystemBody = obj.base == Projectable.SYSTEMBODY
	local body = obj.ref

	--FIXME there is some flickering when changing from one info to another
	--which has different lenght. If the new one is shorter then the first header drawing
	--seems to be too high (accodring to positioning relative to old info).
	--Probably only durring the second drawing the position is ok. The window is anchored at bottom
	textIcon(getBodyIcon(obj))
	ui.text(isSystemBody and body.name or body.label)
	ui.spacing()

	if isSystemBody then
		ui.withFont(detailfont, function()
			ui.textWrapped(body.astroDescription)
		end)
	end

	ui.separator()
	ui.spacing()

	local data = {}

	--SystemBody data is static so we use cache
	--Ship data migh be dynamic in the future
	if isSystemBody and self.prev_body == body then
		data = self.data
	else
		self.prev_body = body

		if isSystemBody then -- system body
			local parent = body.parent
			local starport = body.superType == "STARPORT"
			local surface = body.type == "STARPORT_SURFACE"
			local sma = body.semiMajorAxis
			local semimajoraxis = nil
			if sma and sma > 0 then
				semimajoraxis = ui.Format.Distance(sma)
			end
			local rp = body.rotationPeriod * 24 * 60 * 60
			local op = body.orbitPeriod * 24 * 60 * 60
			local pop = math.round(body.population * 1e9)
			local techLevel = starport and SpaceStation.GetTechLevel(body) or nil
			if techLevel == 11 then
				techLevel = luc.MILITARY
			end
			data = {
				{ name = lc.MASS, icon = icons.body_radius,
					value = (not starport) and ui.Format.Mass(body.mass) or nil },
				{ name = lc.RADIUS, icon = icons.body_radius,
					value = (not starport) and ui.Format.Distance(body.radius) or nil },
				{ name = lc.SURFACE_GRAVITY, icon = icons.body_radius,
					value = (not starport) and ui.Format.Speed(body.gravity, true).." ("..ui.Format.Gravity(body.gravity / 9.80665)..")" or nil },
				{ name = lc.ESCAPE_VELOCITY, icon = icons.body_radius,
					value = (not starport) and ui.Format.Speed(body.escapeVelocity , true) or nil },
				{ name = lc.MEAN_DENSITY, icon = icons.body_radius,
					value = (not starport) and ui.Format.Mass(body.meanDensity).."/m3" or nil },
				{ name = lc.ORBITAL_PERIOD, icon = icons.body_orbit_period,
					value = op and op > 0 and ui.Format.Duration(op, 2) or nil },
				{ name = lc.DAY_LENGTH, icon = icons.body_day_length,
					value = rp > 0 and ui.Format.Duration(rp, 2) or nil },
				{ name = luc.ORBIT_APOAPSIS, icon = icons.body_semi_major_axis,
					value = (parent and not surface) and ui.Format.Distance(body.apoapsis) or nil },
				{ name = luc.ORBIT_PERIAPSIS, icon = icons.body_semi_major_axis,
					value = (parent and not surface) and ui.Format.Distance(body.periapsis) or nil },
				{ name = lc.SEMI_MAJOR_AXIS, icon = icons.body_semi_major_axis,
					value = semimajoraxis },
				{ name = lc.ECCENTRICITY, icon = icons.body_semi_major_axis,
					value = (parent and not surface) and string.format("%0.2f", body.eccentricity) or nil },
				{ name = lc.AXIAL_TILT, icon = icons.body_semi_major_axis,
					value = (not starport) and string.format("%0.2f", body.axialTilt) or nil },
				{ name = lc.POPULATION, icon = icons.personal,
					value = pop > 0 and ui.Format.NumberAbbv(pop) or nil },
				{ name = luc.TECH_LEVEL, icon = icons.equipment,
					value = starport and techLevel or nil }
			}

			--change the internal cached data only when new is fully built
			--prevents additional flickering
			self.data = data

		elseif obj.ref:IsShip() then -- physical body
			-- TODO: the advanced target scanner should add additional data here,
			-- but we really do not want to hardcode that here. there should be
			-- some kind of hook that the target scanner can hook into to display
			-- more info here.
			-- This is what should be inserted:
			table.insert(data, { name = luc.SHIP_TYPE, value = body:GetShipType() })
			if player:GetEquipCountOccupied('target_scanner') > 0 or player:GetEquipCountOccupied('advanced_target_scanner') > 0 then
				local hd = body:GetEquip("engine", 1)
				table.insert(data, { name = luc.HYPERDRIVE, value = hd and hd:GetName() or lc.NO_HYPERDRIVE })
				table.insert(data, { name = luc.MASS, value = Format.MassTonnes(body:GetStats().staticMass) })
				table.insert(data, { name = luc.CARGO, value = Format.MassTonnes(body:GetStats().usedCargo) })
			end
		else
			data = {}
		end
	end

	ui.withFont(detailfont, function()
		tabular(data, Windows.objectInfo.size.x)
	end)
end

function Windows.objectInfo.Dummy()
	ui.withFont(detailfont, function()
		ui.text("Tiny rocky planet with no significant")
	end)
end

function systemViewLayout:onUpdateWindowPivots(w)
	w.systemName.anchors   = { ui.anchor.center, ui.anchor.top }
	w.edgeButtons.anchors  = { ui.anchor.right,  ui.anchor.center }
	w.timeButtons.anchors  = { ui.anchor.right,  ui.anchor.bottom }
	w.orbitPlanner.anchors = { ui.anchor.right,  ui.anchor.bottom }
	w.objectInfo.anchors   = { ui.anchor.right,  ui.anchor.bottom }
	w.unexplored.anchors   = { ui.anchor.center, ui.anchor.center }
end

function systemViewLayout:onUpdateWindowConstraints(w)
	-- resizing, aligning windows - static
	w.systemName.pos.x = ui.screenWidth * 0.5
	w.systemName.pos.y = styles.MainButtonSize.y + (styles.MainButtonPadding + styles.WindowPadding.y) * 2 -- matches fx-window.lua
	w.systemName.size.x = 0 -- adaptive width

	w.edgeButtons.size.y = 0 -- adaptive height

	w.orbitPlanner.pos = w.timeButtons.pos - Vector2(w.edgeButtons.size.x, w.timeButtons.size.y)
	w.orbitPlanner.size.x = w.timeButtons.size.x - w.edgeButtons.size.x
	w.objectInfo.pos = Vector2(w.edgeButtons.pos.x - w.edgeButtons.size.x, w.orbitPlanner.pos.y - w.orbitPlanner.size.y)
	w.objectInfo.size = Vector2(math.max(w.objectInfo.size.x, w.orbitPlanner.size.x), 0) -- adaptive height
end

local function displaySystemViewUI()
	if not systemView then onGameStart() end

	if not ui.shouldDrawUI() then return end

	player = Game.player
	if Game.CurrentView() == "system" then
		if ui.isKeyReleased(ui.keys.tab) then
			systemViewLayout.enabled = not systemViewLayout.enabled
		end

		systemViewLayout:display()
		ui.withStyleColors({ WindowBg = colors.transparent }, function()
			leftSidebar:Draw()
		end)

		displayOnScreenObjects()

		if ui.escapeKeyReleased() then
			Game.SetView("sector")
		end

		if ui.ctrlHeld() and ui.isKeyReleased(ui.keys.delete) then
			package.reimport 'pigui.modules.system-overview-window'
			package.reimport 'pigui.modules.system-econ-view'
			package.reimport()
		end
	end
end

Event.Register("onGameStart", onGameStart)
Event.Register("onEnterSystem", onEnterSystem)
ui.registerHandler("system-view", ui.makeFullScreenHandler("system-view", displaySystemViewUI))

return {}
