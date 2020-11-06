-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Game = require 'Game'
local utils = require 'utils'
local Event = require 'Event'
local Format = require 'Format'
local SystemPath = require 'SystemPath'
local hyperJumpPlanner = require 'pigui.modules.hyperjump-planner'
local systemEconView = require 'pigui.modules.system-econ-view'

local Lang = require 'Lang'
local lc = Lang.GetResource("core");
local lui = Lang.GetResource("ui-core");

local ui = require 'pigui'

local player = nil
local colors = ui.theme.colors
local icons = ui.theme.icons
local hideSectorViewWindows = false

local mainButtonSize = ui.rescaleUI(Vector2(32,32), Vector2(1600, 900))
local mainButtonFramePadding = 3

local font = ui.fonts.pionillium.medlarge
local smallfont = ui.fonts.pionillium.medium
local MAX_SEARCH_STRINGS_VISIBLE = 15
local edgePadding = Vector2(font.size)

local function setAlpha(c, a)
	return Color(c.r, c.g, c.b, a)
end
-- all colors, used in this module
local svColor = {
	BUTTON_ACTIVE = colors.buttonBlue,
	BUTTON_INACTIVE = setAlpha(colors.buttonBlue, 0),
	BUTTON_SEMIACTIVE = setAlpha(colors.buttonBlue, 80),
	BUTTON_INK = colors.buttonInk,
	FONT = colors.font,
	UNKNOWN = colors.unknown,
	WINDOW_BG = colors.lightBlackBackground
}

local buttonState = {
	[true]        = { color = svColor.BUTTON_ACTIVE },
	[false]       = { color = svColor.BUTTON_INACTIVE }
}

local draw_vertical_lines = false
local draw_out_range_labels = false
local draw_uninhabited_labels = true
local automatic_system_selection = true
local lock_hyperspace_target = false

local function mainMenuButton(icon, tooltip)
	return ui.coloredSelectedIconButton(icon, mainButtonSize, false, mainButtonFramePadding, svColor.BUTTON_ACTIVE, svColor.BUTTON_INK, tooltip)
end

local function textIcon(icon, tooltip)
	ui.icon(icon, Vector2(ui.getTextLineHeight()), svColor.FONT, tooltip)
end

local sectorView

local onGameStart = function ()
	-- connect to class SectorView
	sectorView = Game.sectorView
	-- connect hyper jump planner to class SectorView
	hyperJumpPlanner.setSectorView(sectorView)
	-- update visibility states
	sectorView:SetAutomaticSystemSelection(automatic_system_selection)
	sectorView:SetDrawOutRangeLabels(draw_out_range_labels)
	sectorView:SetDrawUninhabitedLabels(draw_uninhabited_labels)
	sectorView:SetDrawVerticalLines(draw_vertical_lines)
end

local hyperspaceDetailsCache = {}

local function clearHyperspaceCache(ship)
	if ship and ship == player then
		hyperspaceDetailsCache = {}
	end
end

local function getHyperspaceDetails(path)
	local p = path.sectorX .. "/" .. path.sectorY .. "/" .. path.sectorZ .. "/" .. path.systemIndex
	local it = hyperspaceDetailsCache[p]
	if not it then
		local jumpStatus, distance, fuelRequired, duration = player:GetHyperspaceDetails(path)
		it = { jumpStatus = jumpStatus, distance = distance, fuelRequired = fuelRequired, duration = duration, path = path }
		hyperspaceDetailsCache[p] = it
	end
	return it
end

local function newWindow(name)
	return {
		size = Vector2(0.0, 0.0),
		pos = Vector2(0.0, 0.0),
		visible = true,
		name = name,
		style_colors = {["WindowBg"] = svColor.WINDOW_BG},
		params = ui.WindowFlags {"NoTitleBar", "AlwaysAutoResize", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus", "NoSavedSettings"}
	}
end

-- all windows in this view
local Windows = {
	current = newWindow("SectorMapCurrentSystem"), -- current system string
	hjPlanner = newWindow("HyperJumpPlanner"), -- hyper jump planner
	systemInfo = newWindow("SectorMapSystemInfo"), -- selected system information
	searchBar = newWindow("SectorMapSearchBar"),
	edgeButtons = newWindow("SectorMapEdgeButtons"),
	factions = newWindow("SectorMapFactions")
}

local statusIcons = {
	OK = { icon = icons.route_destination },
	CURRENT_SYSTEM = { icon = icons.navtarget },
	INSUFFICIENT_FUEL = { icon = icons.fuel },
	OUT_OF_RANGE = { icon = icons.alert_generic },
	NO_DRIVE = { icon = icons.hyperspace_off }
}

local function draw_jump_status(item)
	textIcon(statusIcons[item.jumpStatus].icon, lui[item.jumpStatus])
	ui.sameLine()
	ui.text(string.format("%.2f%s %d%s %s",
		item.distance, lc.UNIT_LY, item.fuelRequired, lc.UNIT_TONNES, ui.Format.Duration(item.duration, 2)))
end

local function calc_star_dist(star)
	local dist = 0.0
	while star do
		dist = dist + (star.apoapsis or 0 + star.periapsis or 0) / 2
		star = star.parent
	end
	return dist
end

function Windows.systemInfo.Show()
	local label = lc.SELECTED_SYSTEM
	local current_systempath = sectorView:GetCurrentSystemPath()
	local systempath = sectorView:GetSelectedSystemPath()
	if systempath then
		local starsystem = systempath:GetStarSystem()
		local clicked = false
		ui.withID(label, function()
			-- selected system label
			textIcon(icons.info)
			ui.sameLine()
			ui.text(starsystem.name .. " (" .. math.floor(systempath.sectorX) .. ", " .. math.floor(systempath.sectorY) .. ", " .. math.floor(systempath.sectorZ) .. ")")
			if not sectorView:IsCenteredOn(systempath) then
				-- add button to center on the object
				ui.sameLine()
				if ui.coloredSelectedIconButton(icons.maneuver, Vector2(ui.getTextLineHeight()), false, 0, svColor.WINDOW_BG, svColor.FONT, lui.CENTER_ON_SYSTEM) then
					sectorView:GotoSystemPath(systempath)
				end
			end

			-- number of stars
			local numstarsText = {
				-- don't ask for the astro description of a gravpoint
				starsystem.numberOfStars == 1 and starsystem.rootSystemBody.astroDescription or "",
				lc.BINARY_SYSTEM,
				lc.TRIPLE_SYSTEM,
				lc.QUADRUPLE_SYSTEM
			}

			-- description
			ui.withFont(smallfont, function()
				-- jump data
				if not current_systempath:IsSameSystem(systempath) then
					draw_jump_status(getHyperspaceDetails(systempath))
				end
				ui.spacing()

				-- selected system alternative labels
				if next(starsystem.other_names) ~= nil then
					ui.pushTextWrapPos(ui.getContentRegion().x)
					ui.textWrapped(table.concat(starsystem.other_names, ", "))
					ui.popTextWrapPos()
				end

				-- system description
				ui.pushTextWrapPos(ui.getContentRegion().x)
				ui.textWrapped(starsystem.shortDescription)
				ui.popTextWrapPos()
				ui.separator()
				ui.spacing()
				ui.text(numstarsText[starsystem.numberOfStars])
				ui.spacing()
			end)

			-- star list
			local stars = starsystem:GetJumpable()
			for _,star in pairs(stars) do
				local pos = ui.getCursorPos() + Vector2(0, 1) -- add vertical alignment, not quite necessary
				if ui.selectable("## " .. star.name, star.path == systempath, {}) then
					clicked = star.path
				end
				ui.sameLine(0, 0)
				textIcon(icons.sun)
				ui.sameLine()
				ui.text(star.name)
				-- distance from system center
				local dist = calc_star_dist(star)
				if dist > 1 then
					local dist_text = Format.Distance(dist)
					ui.sameLine(ui.getColumnWidth() - ui.calcTextSize(dist_text).x)
					ui.text(dist_text)
				end
			end
			if clicked then
				sectorView:SwitchToPath(clicked)
			end

			-- check if the selected star has changed
			if systempath ~= prevSystemPath then
				-- if so, check the route, and update there if necessary
				hyperJumpPlanner.updateInRoute(systempath)
				prevSystemPath = systempath
			end
		end)
	end
end

function Windows.systemInfo.Dummy()
	textIcon(icons.sun)
	ui.sameLine()
	ui.text("Selected system")
	ui.separator()
	ui.text("Distance")
	ui.selectable("Star 1", false, {})
	ui.selectable("Star 2", false, {})
	ui.selectable("Star 3", false, {})
	ui.selectable("Star 4", false, {})
	ui.text("Three\nline\ndescription")
	-- let's count a row of 6 buttons so that it is as in the system map
	mainMenuButton(icons.reset_view, "DUMMY")
	ui.sameLine()
	mainMenuButton(icons.reset_view, "DUMMY")
	ui.sameLine()
	mainMenuButton(icons.reset_view, "DUMMY")
	ui.sameLine()
	mainMenuButton(icons.reset_view, "DUMMY")
	ui.sameLine()
	mainMenuButton(icons.reset_view, "DUMMY")
	ui.sameLine()
	mainMenuButton(icons.reset_view, "DUMMY")
	ui.sameLine()
	mainMenuButton(icons.reset_view, "DUMMY")
	ui.sameLine()
end

local prevSystemPath = nil

local search_text = ""

local function showSettings()
	local changed
	changed, draw_vertical_lines = ui.checkbox(lc.DRAW_VERTICAL_LINES, draw_vertical_lines)
	if changed then
		sectorView:SetDrawVerticalLines(draw_vertical_lines)
	end
	changed, draw_out_range_labels = ui.checkbox(lc.DRAW_OUT_RANGE_LABELS, draw_out_range_labels)
	if changed then
		sectorView:SetDrawOutRangeLabels(draw_out_range_labels)
	end
	changed, draw_uninhabited_labels = ui.checkbox(lc.DRAW_UNINHABITED_LABELS, draw_uninhabited_labels)
	if changed then
		sectorView:SetDrawUninhabitedLabels(draw_uninhabited_labels)
	end
	changed, automatic_system_selection = ui.checkbox(lc.AUTOMATIC_SYSTEM_SELECTION, automatic_system_selection)
	if changed then
		sectorView:SetAutomaticSystemSelection(automatic_system_selection)
	end
	-- end
end

function Windows.edgeButtons.Show()
	-- view control buttons
	if mainMenuButton(icons.reset_view, lc.RESET_ORIENTATION_AND_ZOOM) then
		sectorView:ResetView()
	end
	mainMenuButton(icons.rotate_view, lui.ROTATE_VIEW)
	sectorView:SetRotateMode(ui.isItemActive())
	mainMenuButton(icons.search_lens, lui.ZOOM)
	sectorView:SetZoomMode(ui.isItemActive())
	ui.text("")
	-- settings buttons
	if mainMenuButton(icons.settings, lui.SETTINGS) then
		ui.openPopup("sectorViewLabelSettings")
	end
	ui.popup("sectorViewLabelSettings", function()
		showSettings()
	end)
	if ui.coloredSelectedIconButton(icons.shield_other, mainButtonSize, false, mainButtonFramePadding, buttonState[Windows.factions.visible].color, svColor.BUTTON_INK, "Factions") then
		Windows.factions.visible = not Windows.factions.visible
	end
	if ui.coloredSelectedIconButton(icons.info, mainButtonSize, false, mainButtonFramePadding, buttonState[Windows.systemInfo.visible].color, svColor.BUTTON_INK, lc.OBJECT_INFO) then
		Windows.systemInfo.visible = not Windows.systemInfo.visible
	end
	if ui.coloredSelectedIconButton(icons.route, mainButtonSize, false, mainButtonFramePadding, buttonState[Windows.hjPlanner.visible].color, svColor.BUTTON_INK, lui.HYPERJUMP_ROUTE) then
		Windows.hjPlanner.visible = not Windows.hjPlanner.visible
	end
end

local function drawSearchResults(systempaths)
	local data = {}
	for _,path in pairs(systempaths) do table.insert(data, getHyperspaceDetails(path)) end

	ui.child("search_results", function ()
		table.sort(data, function(a,b)
			return a.path ~= b.path and (not a.path:IsSameSystem(b.path)) and a.distance < b.distance
		end)
		for _,item in pairs(data) do
			local system = item.path:GetStarSystem()
			if ui.selectable(system.name, false, {}) then
				sectorView:SwitchToPath(item.path)
			end
			ui.sameLine()
			ui.withFont(smallfont, function() draw_jump_status(item) end)
		end
	end)
end

local searchString, systemPaths = "", nil
local leftBarMode = "SEARCH"

function Windows.searchBar:Show()
	if mainMenuButton(icons.search_lens, lc.SEARCH) then leftBarMode = "SEARCH" end
	ui.sameLine()
	if mainMenuButton(icons.money, lui.ECONOMY_TRADE) then leftBarMode = "ECON" end

	ui.spacing()

	if leftBarMode == "SEARCH" then
		ui.text(lc.SEARCH)
		search_text, changed = ui.inputText("", search_text, {})
		ui.spacing()
		local parsedSystem = changed and search_text ~= "" and SystemPath.ParseString(search_text)
		if parsedSystem and parsedSystem ~= nil then
			sectorView:GotoSectorPath(parsedSystem)
		end

		if search_text ~= searchString then
			systemPaths = search_text ~= "" and sectorView:SearchNearbyStarSystemsByName(search_text)
			searchString = search_text
		end

		if not systemPaths or #systemPaths == 0 then
			ui.text(lc.NOT_FOUND)
		else
			drawSearchResults(systemPaths)
		end
	elseif leftBarMode == "ECON" then
		local selectedPath = sectorView:GetSelectedSystemPath()
		local currentPath = sectorView:GetCurrentSystemPath()

		local currentSys = currentPath:GetStarSystem()
		local selectedSys = selectedPath and selectedPath:GetStarSystem()
		local showComparison = selectedSys.population > 0
			and not currentPath:IsSameSystem(selectedPath)
			and (Game.player.trade_computer_cap or 0) > 0

		ui.withFont(smallfont, function() ui.text(lui.COMMODITY_TRADE_ANALYSIS) end)
		ui.spacing()
		if showComparison then
			ui.text(currentSys.name)
			ui.sameLine(ui.getColumnWidth() - ui.calcTextSize(selectedSys.name).x)
		end
		ui.text(selectedSys.name)

		ui.spacing()
		ui.separator()
		ui.spacing()

		ui.withFont(smallfont, function()
			if showComparison then
				systemEconView.draw(currentSys, selectedSys)
			else
				systemEconView.draw(selectedSys)
			end
		end)
	end
end

function Windows.searchBar.Dummy()
	ui.text("***************")
	ui.inputText("", "", {})
	ui.text("***************")
end

function Windows.current.Show()
	local path = sectorView:GetCurrentSystemPath()
	local starsystem = path:GetStarSystem()
	textIcon(icons.navtarget)
	ui.sameLine()
	if ui.selectable(' ' .. starsystem.name .. " (" .. path.sectorX .. ", " .. path.sectorY .. ", " .. path.sectorZ .. ")") then
		sectorView:SwitchToPath(path)
	end
end

function Windows.factions.Show()
	textIcon(icons.shield)
	ui.sameLine()
	ui.text("Factions")
	local factions = sectorView:GetFactions()
	for _,f in pairs(factions) do
		local changed, value
		ui.withStyleColors({ ["Text"] = Color(f.faction.colour.r, f.faction.colour.g, f.faction.colour.b) }, function()
			changed, value = ui.checkbox(f.faction.name, f.visible)
		end)
		if changed then
			sectorView:SetFactionVisible(f.faction, value)
		end
	end
end

Windows.hjPlanner.Show = hyperJumpPlanner.display
Windows.hjPlanner.Dummy = hyperJumpPlanner.Dummy

local function showWindow(w)
	ui.setNextWindowSize(w.size, "Always")
	ui.setNextWindowPos(w.pos, "Always")
	ui.withStyleColors(w.style_colors, function() ui.window(w.name, w.params, function() w:Show() end) end)
end

local dummyFrames = 3

local function displaySectorViewWindow()
	player = Game.player
	if Game.CurrentView() == "sector" then
		if dummyFrames > 0 then -- do it a few frames, because imgui need a few frames to make the correct window size

			-- measuring windows (or dummies)
			ui.withFont(font, function()
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
				Windows.current.pos = edgePadding
				Windows.current.size.x = 0 -- adaptive width
				Windows.hjPlanner.size.x = math.max(Windows.hjPlanner.size.x, Windows.systemInfo.size.x - Windows.edgeButtons.size.x)
				Windows.hjPlanner.pos = Vector2(ui.screenWidth - Windows.edgeButtons.size.x, ui.screenHeight - edgePadding.y) - Windows.hjPlanner.size
				Windows.systemInfo.pos = Windows.hjPlanner.pos - Vector2(0, Windows.systemInfo.size.y)
				Windows.systemInfo.size = Vector2(Windows.hjPlanner.size.x, 0) -- adaptive height
				Windows.searchBar.pos = Windows.current.pos + Windows.current.size
				Windows.searchBar.size = Vector2(0, ui.screenHeight - Windows.searchBar.pos.y - edgePadding.y - ui.timeWindowSize.y)
				Windows.edgeButtons.pos = Vector2(ui.screenWidth - Windows.edgeButtons.size.x, ui.screenHeight / 2 - Windows.edgeButtons.size.y / 2) -- center-right
				Windows.factions.pos = Vector2(Windows.systemInfo.pos.x, Windows.current.pos.y)
				Windows.factions.size = Vector2(ui.screenWidth - Windows.factions.pos.x - edgePadding.x, 0.0)
			end
			dummyFrames = dummyFrames - 1
		else
			if ui.isKeyReleased(ui.keys.tab) then
				hideSectorViewWindows = not hideSectorViewWindows;
			end
			if not hideSectorViewWindows then
				-- display all windows
				ui.withFont(font, function()
					for _,w in pairs(Windows) do
						if w.visible then showWindow(w) end
					end
				end)
			end
		end

		if ui.noModifierHeld() and ui.isKeyReleased(ui.keys.escape) then
			Game.SetView("world")
		end
	end
end

ui.registerModule("game", displaySectorViewWindow)
Event.Register("onGameStart", onGameStart)
Event.Register("onLeaveSystem", function()
	hyperspaceDetailsCache = {}
end)

-- reset cached data
Event.Register("onGameEnd", function()
	searchString = ""
	systemPaths = nil
	leftBarMode = "SEARCH"
end)

-- events moved from hyperJumpPlanner
Event.Register("onGameEnd", hyperJumpPlanner.onGameEnd)
Event.Register("onEnterSystem", hyperJumpPlanner.onEnterSystem)
Event.Register("onShipEquipmentChanged", hyperJumpPlanner.onShipEquipmentChanged)

Event.Register("onShipEquipmentChanged", clearHyperspaceCache)
Event.Register("onShipTypeChanged", clearHyperspaceCache)

return {}
