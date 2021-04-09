-- Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
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

-- will be initialized at first frame
local textIconSize = nil
local edgePadding = nil

local mainButtonSize = ui.rescaleUI(Vector2(32,32), Vector2(1600, 900))
local mainButtonFramePadding = 3
local indicatorSize = Vector2(30 , 30)

local selectedObject -- object, centered in SystemView

local hudfont = ui.fonts.pionillium.small
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
	ui.icon(icon, textIconSize, svColor.FONT, tooltip)
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

function Windows.edgeButtons.Show()
	-- view control buttons
	if ui.coloredSelectedIconButton(icons.reset_view, mainButtonSize, false, mainButtonFramePadding, svColor.BUTTON_ACTIVE, svColor.BUTTON_INK, luc.RESET_ORIENTATION_AND_ZOOM) then
		systemView:SetVisibility("RESET_VIEW")
	end
	ui.coloredSelectedIconButton(icons.rotate_view, mainButtonSize, false, mainButtonFramePadding, svColor.BUTTON_ACTIVE, svColor.BUTTON_INK, luc.ROTATE_VIEW)
	systemView:SetRotateMode(ui.isItemActive())
	ui.coloredSelectedIconButton(icons.search_lens,mainButtonSize, false, mainButtonFramePadding, svColor.BUTTON_ACTIVE, svColor.BUTTON_INK, luc.ZOOM)
	systemView:SetZoomMode(ui.isItemActive())
	ui.text("")
	-- visibility control buttons
	if ui.coloredSelectedIconButton(buttonState[ship_drawing].icon, mainButtonSize, false, mainButtonFramePadding, buttonState[ship_drawing].color, svColor.BUTTON_INK, lc.SHIPS_DISPLAY_MODE_TOGGLE) then
		ship_drawing = nextShipDrawings[ship_drawing]
		systemView:SetVisibility(ship_drawing)
	end
	if ui.coloredSelectedIconButton(buttonState[show_lagrange].icon, mainButtonSize, false, mainButtonFramePadding, buttonState[show_lagrange].color, svColor.BUTTON_INK, lc.L4L5_DISPLAY_MODE_TOGGLE) then
		show_lagrange = nextShowLagrange[show_lagrange]
		systemView:SetVisibility(show_lagrange)
	end
	if ui.coloredSelectedIconButton(buttonState[show_grid].icon, mainButtonSize, false, mainButtonFramePadding, buttonState[show_grid].color, svColor.BUTTON_INK, lc.GRID_DISPLAY_MODE_TOGGLE) then
		show_grid = nextShowGrid[show_grid]
		systemView:SetVisibility(show_grid)
	end
	ui.text("")
	-- windows control buttons
	if ui.coloredSelectedIconButton(icons.info, mainButtonSize, false, mainButtonFramePadding, buttonState[Windows.objectInfo.visible].color, svColor.BUTTON_INK, lc.OBJECT_INFO) then
		Windows.objectInfo.visible = not Windows.objectInfo.visible
	end
	if ui.coloredSelectedIconButton(icons.semi_major_axis, mainButtonSize, false, mainButtonFramePadding, buttonState[Windows.orbitPlanner.visible].color, svColor.BUTTON_INK, lc.ORBIT_PLANNER) then
		Windows.orbitPlanner.visible = not Windows.orbitPlanner.visible
	end
end

function Windows.orbitPlanner.Show()
	textIcon(icons.semi_major_axis)
	ui.sameLine()
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
	showDvLine(icons.decrease, icons.orbit_prograde, icons.increase, "prograde", ui.Format.Speed, luc.DECREASE, lc.PLANNER_RESET_PROGRADE, luc.INCREASE)
	showDvLine(icons.decrease, icons.orbit_normal, icons.increase, "normal", ui.Format.Speed, luc.DECREASE, lc.PLANNER_RESET_NORMAL, luc.INCREASE)
	showDvLine(icons.decrease, icons.orbit_radial, icons.increase, "radial", ui.Format.Speed, luc.DECREASE, lc.PLANNER_RESET_RADIAL, luc.INCREASE)
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

local function getBodyIcon(obj)
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
	elseif obj.base == Projectable.SYSTEMBODY then
		local body = obj.ref
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
			if body.IsMoon then
				return icons.moon
			else
				if body.radius < ASTEROID_RADIUS then
					return icons.asteroid_hollow
				else
					return icons.rocky_planet
				end
			end
		end -- st
	else
		-- physical body
		local body = obj.ref
		if body:IsShip() then
			local shipClass = body:GetShipClass()
			if icons[shipClass] then
				return icons[shipClass]
			else
				print("system-view-ui.lua: getBodyIcon unknown ship class " .. (shipClass and shipClass or "nil"))
				return icons.ship -- TODO: better icon
			end
		elseif body:IsHyperspaceCloud() then
			return icons.hyperspace -- TODO: better icon
		elseif body:IsMissile() then
			return icons.bullseye -- TODO: better icon
		elseif body:IsCargoContainer() then
			return icons.rocky_planet -- TODO: better icon
		else
			local t, st = body.type, body.superType
			print("system-view-ui.lua: getBodyIcon not sure how to process body, supertype: " .. (st and st or "nil") .. ", type: " .. (t and t or "nil"))
			--utils.print_r(body)
			return icons.ship
		end
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

-- forked from data/pigui/views/game.lua
local function displayOnScreenObjects()

	local navTarget = player:GetNavTarget()
	local combatTarget = player:GetCombatTarget()

	local should_show_label = ui.shouldShowLabels()
	local iconsize = Vector2(18 , 18)
	local label_offset = 14 -- enough so that the target rectangle fits
	local collapse = iconsize -- size of clusters to be collapsed into single bodies
	local click_radius = collapse:length() * 0.5
	-- make click_radius sufficiently smaller than the cluster size
	-- to prevent overlap of selection regions
	local objectCounter = 0
	local objects_grouped = systemView:GetProjectedGrouped(collapse, 1e64)
	if #objects_grouped == 0 then
		ui.setNextWindowPos(Vector2(ui.screenWidth, ui.screenHeight) / 2 - ui.calcTextSize(lc.UNEXPLORED_SYSTEM_NO_SYSTEM_VIEW) / 2, "Always")
		ui.withStyleColors({["WindowBg"] = svColor.WINDOW_BG}, function()
			ui.window("NoSystemView", {"NoTitleBar", "AlwaysAutoResize", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus", "NoSavedSettings"},
			function()
				ui.text(lc.UNEXPLORED_SYSTEM_NO_SYSTEM_VIEW);
			end)
		end)
		return
	end

	for _,group in ipairs(objects_grouped) do
		local mainObject = group.mainObject
		local mainCoords = Vector2(group.screenCoordinates.x, group.screenCoordinates.y)

		-- indicators
		local stackedSize = indicatorSize
		local stackStep = Vector2(10, 10)
		if group.hasPlayer then
			ui.addIcon(mainCoords, icons.square, svColor.PLAYER, stackedSize, ui.anchor.center, ui.anchor.center)
			stackedSize = stackedSize + stackStep
		end
		if group.hasNavTarget then
			ui.addIcon(mainCoords, icons.square, svColor.NAV_TARGET, stackedSize, ui.anchor.center, ui.anchor.center)
			stackedSize = stackedSize + stackStep
		end
		if group.hasCombatTarget then
			ui.addIcon(mainCoords, icons.square, svColor.COMBAT_TARGET, stackedSize, ui.anchor.center, ui.anchor.center)
			stackedSize = stackedSize + stackStep
		end
		if mainObject.type == Projectable.OBJECT and mainObject.base == Projectable.PLANNER then ui.addIcon(mainCoords, icons.square, svColor.PLANNER, indicatorSize, ui.anchor.center, ui.anchor.center) end

		ui.addIcon(mainCoords, getBodyIcon(mainObject), getColor(mainObject), iconsize, ui.anchor.center, ui.anchor.center)

		local label = getLabel(mainObject)
		if should_show_label then
			if group.objects then
				label = label .. " (" .. #group.objects .. ")"
			end
			ui.addStyledText(mainCoords + Vector2(label_offset,0), ui.anchor.left, ui.anchor.center, label , getColor(mainObject), hudfont)
		end
		local mp = ui.getMousePos()

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
				if ui.selectable(lc.CENTER, false, {}) then
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
				ui.text(item.name)
				ui.nextColumn()
				ui.text(item.value)
				ui.nextColumn()
				-- adding "--" for spacing
				nameWidth = math.max(nameWidth, ui.calcTextSize(item.name .. "--").x)
				valueWidth = math.max(valueWidth, ui.calcTextSize(item.value .. "--").x)
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
	textIcon(icons.info)
	ui.sameLine()
	ui.text(lc.OBJECT_INFO)
	ui.separator()
	local obj = systemView:GetSelectedObject()
	if obj.type ~= Projectable.OBJECT or obj.base ~= Projectable.SHIP and obj.base ~= Projectable.SYSTEMBODY then return end
	local data
	-- system body
	if obj.base == Projectable.SYSTEMBODY then
		local systemBody = obj.ref
		local name = systemBody.name
		local rp = systemBody.rotationPeriod * 24 * 60 * 60
		local r = systemBody.radius
		local radius = nil
		if r and r > 0 then
			local v,u = ui.Format.Distance(r)
			radius = v .. u
		end
		local sma = systemBody.semiMajorAxis
		local semimajoraxis = nil
		if sma and sma > 0 then
			local v,u = ui.Format.Distance(sma)
			semimajoraxis = v .. u
		end
		local op = systemBody.orbitPeriod * 24 * 60 * 60
		data = {
			{ name = lc.NAME_OBJECT,
			value = name },
			{ name = lc.DAY_LENGTH,
			value = rp > 0 and ui.Format.Duration(rp, 2) or nil },
			{ name = lc.RADIUS,
			value = radius },
			{ name = lc.SEMI_MAJOR_AXIS,
			value = semimajoraxis },
			{ name = lc.ORBITAL_PERIOD,
			value = op and op > 0 and ui.Format.Duration(op, 2) or nil }
		}
		-- physical body
	elseif obj.ref:IsShip() then
		local body = obj.ref
		local name = body.label
		data = {{ name = lc.NAME_OBJECT, value = name }}
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
	tabular(data, Windows.objectInfo.size.x)
end

function Windows.objectInfo.Dummy()
	ui.text(lc.OBJECT_INFO)
	ui.separator()
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
			-- calculating in-text icon size for used font size
			if not textIconSize then
				ui.withFont(winfont, function()
					textIconSize = ui.calcTextSize("H")
					textIconSize.x = textIconSize.y -- make square
				end)
				edgePadding = textIconSize
			end
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
				Windows.systemName.pos = Vector2(edgePadding.x, edgePadding.y)
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
