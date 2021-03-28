-- Copyright © 2008-2022 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local Engine = require 'Engine'
local Event = require 'Event'
local Lang = require 'Lang'
local ui = require 'pigui'
local Format = require 'Format'
local Constants = _G.Constants

local Vector2 = _G.Vector2
local lc = Lang.GetResource("core")
local luc = Lang.GetResource("ui-core")

local player = nil
local colors = ui.theme.colors
local icons = ui.theme.icons

local systemView

local mainButtonSize = ui.rescaleUI(Vector2(32,32), Vector2(1600, 900))
local mainButtonFramePadding = 3
local indicatorSize = Vector2(30 , 30)

local selectedObject -- object, centered in SystemView

local hudfont = ui.fonts.pionillium.small
local detailfont = ui.fonts.pionillium.medium
local winfont = ui.fonts.pionillium.medlarge

local ASTEROID_RADIUS = 1500000 -- rocky planets smaller than this (in meters) are considered an asteroid, not a planet


--load enums Projectable::types and Projectable::bases in one table "Projectable"
local Projectable = {}
for _, key in pairs(Constants.ProjectableTypes) do Projectable[key] = Engine.GetEnumValue("ProjectableTypes", key) end
for _, key in pairs(Constants.ProjectableBases) do Projectable[key] = Engine.GetEnumValue("ProjectableBases", key) end

local function setAlpha(c, a)
	return Color(c.r, c.g, c.b, a)
end

-- all colors, used in this module
local svColor = {
	BUTTON_ACTIVE = colors.buttonBlue,
	BUTTON_INACTIVE = setAlpha(colors.buttonBlue, 0),
	BUTTON_SEMIACTIVE = setAlpha(colors.buttonBlue, 80),
	BUTTON_INK = colors.buttonInk,
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
	SELECTED_SHIP_ORBIT = colors.systemMapSelectedShipOrbit,
	SHIP = colors.systemMapShip,
	SHIP_ORBIT = colors.systemMapShipOrbit,
	SYSTEMBODY = colors.systemMapSystemBody,
	SYSTEMBODY_ICON = colors.systemMapSystemBodyIcon,
	SYSTEMBODY_ORBIT = colors.systemMapSystemBodyOrbit,
	WINDOW_BG = colors.lightBlackBackground,
	UNKNOWN = colors.unknown
}

-- button states
local function loop3items(a, b, c) return { [a] = b, [b] = c, [c] = a } end

local buttonState = {
	SHIPS_OFF     = { icon = icons.ships_no_orbits,    color = svColor.BUTTON_INACTIVE },
	SHIPS_ON      = { icon = icons.ships_no_orbits,    color = svColor.BUTTON_SEMIACTIVE },
	SHIPS_ORBITS  = { icon = icons.ships_with_orbits,  color = svColor.BUTTON_ACTIVE },
	LAG_OFF       = { icon = icons.lagrange_no_text,   color = svColor.BUTTON_INACTIVE },
	LAG_ICON      = { icon = icons.lagrange_no_text,   color = svColor.BUTTON_SEMIACTIVE },
	LAG_ICONTEXT  = { icon = icons.lagrange_with_text, color = svColor.BUTTON_ACTIVE },
	GRID_OFF      = { icon = icons.toggle_grid,        color = svColor.BUTTON_INACTIVE },
	GRID_ON       = { icon = icons.toggle_grid,        color = svColor.BUTTON_SEMIACTIVE },
	GRID_AND_LEGS = { icon = icons.toggle_grid,        color = svColor.BUTTON_ACTIVE },
	[true]        = {                                  color = svColor.BUTTON_ACTIVE },
	[false]       = {                                  color = svColor.BUTTON_INACTIVE }
}

local ship_drawing = "SHIPS_OFF"
local show_lagrange = "LAG_OFF"
local show_grid = "GRID_OFF"
local nextShipDrawings = loop3items("SHIPS_OFF", "SHIPS_ON", "SHIPS_ORBITS")
local nextShowLagrange = loop3items("LAG_OFF", "LAG_ICON", "LAG_ICONTEXT")
local nextShowGrid = loop3items("GRID_OFF", "GRID_ON", "GRID_AND_LEGS")

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
	local press = ui.coloredSelectedIconButton(leftIcon, mainButtonSize, false, mainButtonFramePadding, svColor.BUTTON_ACTIVE, svColor.BUTTON_INK, leftTooltip..id, nil)
	if press or (key ~= "factor" and ui.isItemActive()) then
		systemView:TransferPlannerAdd(key, -10)
	end
	wheel()
	ui.sameLine()
	if ui.coloredSelectedIconButton(resetIcon, mainButtonSize, false, mainButtonFramePadding, svColor.BUTTON_ACTIVE, svColor.BUTTON_INK, resetTooltip..id, nil) then
		systemView:TransferPlannerReset(key)
	end
	wheel()
	ui.sameLine()
	press = ui.coloredSelectedIconButton(rightIcon, mainButtonSize, false, mainButtonFramePadding, svColor.BUTTON_ACTIVE, svColor.BUTTON_INK, rightTooltip..id, nil)
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
	if ui.coloredSelectedIconButton(icon, mainButtonSize, false, mainButtonFramePadding, svColor.BUTTON_ACTIVE, svColor.BUTTON_INK, tooltip) then
		time_selected_button_icon = icon
	end
	local active = ui.isItemActive()
	if active then
		systemView:AccelerateTime(factor)
	end
	ui.sameLine()
	return active
end

local function newWindow(name)
	return {
		size = Vector2(0.0, 0.0),
		pos = Vector2(0.0, 0.0),
		visible = true,
		name = name,
		style_colors = {["WindowBg"] = svColor.WINDOW_BG},
		params = {"NoTitleBar", "AlwaysAutoResize", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus", "NoSavedSettings"}
	}
end

-- all windows in this view
local Windows = {
	systemName = newWindow("SystemMapSystemName"),
	objectInfo = newWindow("SystemMapObjectIngo"),
	edgeButtons = newWindow("SystemMapEdgeButtons"),
	orbitPlanner = newWindow("SystemMapOrbitPlanner"),
	timeButtons = newWindow("SystemMapTimeButtons")
}

local function edgeButton(icon, tooltip, state)
	return ui.coloredSelectedIconButton(icon, mainButtonSize, false, mainButtonFramePadding, (state ~= nil and state.color or svColor.BUTTON_ACTIVE), svColor.BUTTON_INK, tooltip)
end

function Windows.edgeButtons.Show()
	local isOrrery = systemView:GetDisplayMode() == "Orrery"
	-- view control buttons
	if edgeButton(icons.reset_view, luc.RESET_ORIENTATION_AND_ZOOM) then
		systemView:SetVisibility("RESET_VIEW")
	end
	edgeButton(icons.rotate_view, luc.ROTATE_VIEW)
	systemView:SetRotateMode(ui.isItemActive())
	edgeButton(icons.search_lens, luc.ZOOM)
	systemView:SetZoomMode(ui.isItemActive())

	if isOrrery and edgeButton(icons.system_overview, luc.HUD_BUTTON_SWITCH_TO_SYSTEM_OVERVIEW) then
		systemView:SetDisplayMode('Atlas')
	end
	if not isOrrery and edgeButton(icons.system_map, luc.HUD_BUTTON_SWITCH_TO_SYSTEM_MAP) then
		systemView:SetDisplayMode('Orrery')
	end
	ui.newLine()
	-- visibility control buttons
	if edgeButton(buttonState[ship_drawing].icon, lc.SHIPS_DISPLAY_MODE_TOGGLE, buttonState[ship_drawing]) then
		ship_drawing = nextShipDrawings[ship_drawing]
		systemView:SetVisibility(ship_drawing)
	end
	if edgeButton(buttonState[show_lagrange].icon, lc.L4L5_DISPLAY_MODE_TOGGLE, buttonState[show_lagrange]) then
		show_lagrange = nextShowLagrange[show_lagrange]
		systemView:SetVisibility(show_lagrange)
	end
	if edgeButton(buttonState[show_grid].icon, lc.GRID_DISPLAY_MODE_TOGGLE, buttonState[show_grid]) then
		show_grid = nextShowGrid[show_grid]
		systemView:SetVisibility(show_grid)
	end
	ui.newLine()
	-- windows control buttons
	if edgeButton(icons.info, lc.OBJECT_INFO, buttonState[Windows.objectInfo.visible]) then
		Windows.objectInfo.visible = not Windows.objectInfo.visible
	end
	if edgeButton(icons.semi_major_axis, lc.ORBIT_PLANNER, buttonState[Windows.orbitPlanner.visible]) then
		Windows.orbitPlanner.visible = not Windows.orbitPlanner.visible
	end
end

function Windows.orbitPlanner.Show()
	if systemView:GetDisplayMode() ~= 'Orrery' then
		Windows.orbitPlanner.visible = false
		return
	end

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
local function getBodyIcon(obj, forWorld)
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

local function drawGroupIcons(coords, icon, color, iconSize, group)
	-- indicators
	local stackedSize = indicatorSize
	local stackStep = Vector2(10, 10)
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

local unexloredWindowFlags = ui.WindowFlags {"NoTitleBar", "AlwaysAutoResize", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus", "NoSavedSettings", "NoInputs"}
-- forked from data/pigui/views/game.lua
local function displayOnScreenObjects()
	local isOrrery = systemView:GetDisplayMode() == 'Orrery'

	local navTarget = player:GetNavTarget()
	local combatTarget = player:GetCombatTarget()

	local should_show_label = ui.shouldShowLabels()
	if not isOrrery then
		should_show_label = should_show_label and systemView:GetZoom() <= 0.5
	end

	local iconsize = Vector2(18 , 18)
	local label_offset = 14 -- enough so that the target rectangle fits
	local collapse = iconsize -- size of clusters to be collapsed into single bodies
	local click_radius = collapse:length() * 0.5
	if not isOrrery then
		click_radius = collapse:length() * 0.8 / systemView:GetZoom()
	end
	-- make click_radius sufficiently smaller than the cluster size
	-- to prevent overlap of selection regions
	local objectCounter = 0
	local objects_grouped = systemView:GetProjectedGrouped(collapse, 1e64)
	if #objects_grouped == 0 then
		ui.setNextWindowPos(Vector2(ui.screenWidth, ui.screenHeight) / 2 - ui.calcTextSize(lc.UNEXPLORED_SYSTEM_NO_SYSTEM_VIEW) / 2, "Always")
		ui.withStyleColors({["WindowBg"] = svColor.WINDOW_BG}, function()
			ui.window("NoSystemView", unexloredWindowFlags,
			function()
				ui.text(lc.UNEXPLORED_SYSTEM_NO_SYSTEM_VIEW);
			end)
		end)
		return
	end

	for _,group in ipairs(objects_grouped) do
		local mainObject = group.mainObject
		local mainCoords = Vector2(group.screenCoordinates.x, group.screenCoordinates.y)
		group.hasPlanner = mainObject.type == Projectable.OBJECT and mainObject.base == Projectable.PLANNER

		drawGroupIcons(mainCoords, getBodyIcon(mainObject, true), getColor(mainObject), iconsize, group)

		local mp = ui.getMousePos()
		local label = getLabel(mainObject)
		if should_show_label or (mp - mainCoords):length() < click_radius then
			if group.objects then
				label = label .. " (" .. #group.objects .. ")"
			end
			ui.addStyledText(mainCoords + Vector2(label_offset,0), ui.anchor.left, ui.anchor.center, label , getColor(mainObject), hudfont)
		end

		if mainObject.type == Projectable.OBJECT and (mainObject.base == Projectable.SYSTEMBODY or mainObject.base == Projectable.SHIP or mainObject.base == Projectable.PLAYER) then
			-- mouse release handler for right button
			if (mp - mainCoords):length() < click_radius then
				if not ui.isAnyWindowHovered() and ui.isMouseReleased(1) then
					ui.openPopup("target" .. label)
				end
			end
			-- make popup
			ui.popup("target" .. label, function()
				local isObject = mainObject.type == Projectable.OBJECT
				local isSystemBody = isObject and mainObject.base == Projectable.SYSTEMBODY
				local isShip = isObject and not isSystemBody and mainObject.ref:IsShip()
				ui.text(getLabel(mainObject))
				ui.separator()
				if isOrrery and ui.selectable(lc.CENTER, false, {}) then
					systemView:SetSelectedObject(mainObject.type, mainObject.base, mainObject.ref)
				end
				if (isShip or isSystemBody and mainObject.ref.physicsBody) and ui.selectable(lc.SET_AS_TARGET, false, {}) then
					if isSystemBody then
						player:SetNavTarget(mainObject.ref.physicsBody)
						ui.playSfx("OK")
					else
						if combatTarget == mainObject.ref then player:SetCombatTarget(nil) end
						player:SetNavTarget(mainObject.ref)
						ui.playSfx("OK")
					end
				end
				if isShip and ui.selectable(lc.SET_AS_COMBAT_TARGET, false, {}) then
					if navTarget == mainObject.ref then player:SetNavTarget(nil) end
					player:SetCombatTarget(mainObject.ref)
				end
			end)
		end
		-- mouse release handler for left button
		if (mp - mainCoords):length() < click_radius then
			if not ui.isAnyWindowHovered() and ui.isMouseReleased(0) and mainObject.type == Projectable.OBJECT then
				systemView:SetSelectedObject(mainObject.type, mainObject.base, mainObject.ref)
			end
		end
		objectCounter = objectCounter + 1
	end
end

local function tabular(data, maxSize)
	if data and #data > 0 then
		ui.columns(2, "Attributes", false)
		local nameWidth = 0
		local valueWidth = 0
		for _,item in pairs(data) do
			if item.value then
				local nWidth = ui.calcTextSize(item.name).x + itemSpacing.x
				local vWidth = ui.calcTextSize(item.value).x + itemSpacing.x
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

function Windows.objectInfo.Show()

	local obj = systemView:GetSelectedObject()
	if obj.type ~= Projectable.OBJECT or obj.base ~= Projectable.SHIP and obj.base ~= Projectable.SYSTEMBODY then
		textIcon(icons.info)
		ui.text(lc.OBJECT_INFO)
		ui.separator()
		return
	end

	local isSystemBody = obj.base == Projectable.SYSTEMBODY
	local body = obj.ref

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

	local data = { }

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
		data = {
			{ name = lc.MASS, icon = icons.body_radius,
			value = (not starport) and ui.Format.Mass(body.mass) or nil },
			{ name = lc.RADIUS, icon = icons.body_radius,
			value = (not starport) and ui.Format.Distance(body.radius) or nil },
			{ name = lc.SURFACE_GRAVITY, icon = icons.body_radius,
			value = (not starport) and ui.Format.Speed(body.gravity, true).." ("..ui.Format.Gravity(body.gravity / 9.8066)..")" or nil },
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
			value = pop > 0 and ui.Format.Number(pop) or nil },

		}

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

	ui.withFont(detailfont, function()
		tabular(data, Windows.objectInfo.size.x)
	end)
end

function Windows.objectInfo.Dummy()
	ui.text(lc.OBJECT_INFO)
	ui.spacing()
	ui.separator()
	ui.spacing()
	ui.text("TAB LINE")
	ui.text("TAB LINE")
	ui.text("TAB LINE")
	ui.text("TAB LINE")
	ui.text("TAB LINE")
	ui.text("TAB LINE")
	ui.text("TAB LINE")
	ui.text("TAB LINE")
	ui.text("TAB LINE")
	ui.text("TAB LINE")
	ui.text("TAB LINE")
end

local function showWindow(w)
	ui.setNextWindowSize(w.size, "Always")
	ui.setNextWindowPos(w.pos, "Always")
	ui.withStyleColors(w.style_colors, function() ui.window(w.name, w.params, w.Show) end)
end

local dummyFrames = 3

local hideSystemViewWindows = false

local function displaySystemViewUI()
	player = Game.player
	local current_view = Game.CurrentView()
	if current_view == "system" then
		if dummyFrames > 0 then -- do it a few frames, because imgui need a few frames to make the correct window size

			-- first, doing some one-time actions here
			-- measuring windows (or dummies)
			ui.withFont(winfont, function()
				for _,w in pairs(Windows) do
					ui.setNextWindowPos(Vector2(ui.screenWidth, 0.0), "Always")
					ui.window(w.name, w.params, function()
						if w.Dummy then w.Dummy()
						else w.Show()
						end
						w.size = ui.getWindowSize()
					end)
				end
			end)

			-- make final calculations on the last non-working frame
			if dummyFrames == 1 then
				-- resizing, aligning windows - static
				Windows.systemName.pos = Vector2(winfont.size)
				Windows.systemName.size.x = 0 -- adaptive width
				Windows.edgeButtons.pos = Vector2(ui.screenWidth - Windows.edgeButtons.size.x, ui.screenHeight / 2 - Windows.edgeButtons.size.y / 2) -- center-right
				Windows.timeButtons.pos = Vector2(ui.screenWidth, ui.screenHeight) - Windows.timeButtons.size
				Windows.orbitPlanner.pos = Windows.timeButtons.pos - Vector2(0, Windows.orbitPlanner.size.y)
				Windows.orbitPlanner.size.x = Windows.edgeButtons.pos.x - Windows.timeButtons.pos.x
				Windows.objectInfo.pos = Windows.orbitPlanner.pos - Vector2(0, Windows.objectInfo.size.y)
				Windows.objectInfo.size = Vector2(Windows.orbitPlanner.size.x, 0) -- adaptive height
			end
			dummyFrames = dummyFrames - 1
		else
			if ui.isKeyReleased(ui.keys.tab) then
				hideSystemViewWindows = not hideSystemViewWindows;
			end
			if not hideSystemViewWindows then
				-- display all windows
				ui.withFont(winfont, function()
					for _,w in pairs(Windows) do
						if w.visible then showWindow(w) end
					end
				end)
			end
			displayOnScreenObjects()
		end

		if ui.escapeKeyReleased() then
			Game.SetView("sector")
		end
	end
end

Event.Register("onGameStart", onGameStart)
Event.Register("onEnterSystem", onEnterSystem)
ui.registerHandler("system-view", ui.makeFullScreenHandler("system-view", displaySystemViewUI))
return {}
