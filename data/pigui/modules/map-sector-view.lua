-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Game = require 'Game'
local utils = require 'utils'
local Event = require 'Event'
local SystemPath = require 'SystemPath'
local hyperJumpPlanner = require 'pigui.modules.hyperjump-planner'

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
local MAX_SEARCH_STRINGS_VISIBLE = 15
local textIconSize = nil
local edgePadding = nil

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
	ui.icon(icon, textIconSize, svColor.FONT, tooltip)
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
	current = newWindow("SectorMapCurrentSystem"), -- current system string
	hjPlanner = newWindow("HyperJumpPlanner"), -- hyper jump planner
	systemInfo = newWindow("SectorMapSystemInfo"), -- selected system information
	searchBar = newWindow("SectorMapSearchBar"),
	edgeButtons = newWindow("SectorMapEdgeButtons"),
	factions = newWindow("SectorMapFactions")
}

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
				if ui.coloredSelectedIconButton(icons.maneuver, textIconSize, false, 0, svColor.WINDOW_BG, svColor.FONT, lui.CENTER_ON_SYSTEM) then
					sectorView:GotoSystemPath(systempath)
				end
			end
			-- selected system alternative labels
			if next(starsystem.other_names) ~= nil then
				ui.pushTextWrapPos(ui.getContentRegion().x)
				ui.textWrapped(table.concat(starsystem.other_names, ", "))
				ui.popTextWrapPos()
			end
			-- jump data
			if not current_systempath:IsSameSystem(systempath) then
			ui.separator()
				local jumpStatus, distance, fuelRequired, duration = player:GetHyperspaceDetails(current_systempath, systempath)
				ui.text(jumpStatus .. "  " .. string.format("%.2f", distance) .. lc.UNIT_LY .. "  " .. fuelRequired .. lc.UNIT_TONNES .. "  " .. ui.Format.Duration(duration, 2))
			end
			-- description
			ui.pushTextWrapPos(ui.getContentRegion().x)
			ui.textWrapped(starsystem.shortDescription)
			ui.popTextWrapPos()
			ui.separator()
			-- number of stars
			local numstars = starsystem.numberOfStars
			local numstarstext = ""
			if numstars == 4 then
				numstarstext = lc.QUADRUPLE_SYSTEM
			elseif numstars == 3 then
				numstarstext = lc.TRIPLE_SYSTEM
			elseif numstars == 2 then
				numstarstext = lc.BINARY_SYSTEM
			else
				numstarstext = starsystem.rootSystemBody.astroDescription
			end
			ui.text(numstarstext)
			-- star list
			local stars = starsystem:GetJumpable()
			for _,star in pairs(stars) do
				local pos = ui.getCursorPos() + Vector2(0, 1) -- add vertical alignment, not quite necessary
				if ui.selectable("## " .. star.name, star.path == systempath, {}) then
					clicked = star.path
				end
				ui.setCursorPos(pos)
				textIcon(icons.sun)
				ui.sameLine()
				ui.text(star.name)
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

local hyperspaceDetailsCache = {}

local function clearHyperspaceCache(ship)
	if ship and ship == player then
		hyperspaceDetailsCache = {}
	end
end

Event.Register("onShipEquipmentChanged", clearHyperspaceCache)
Event.Register("onShipTypeChanged", clearHyperspaceCache)

local function getHyperspaceDetails(path)
	local p = path.sectorX .. "/" .. path.sectorY .. "/" .. path.sectorZ .. "/" .. path.systemIndex
	local it = hyperspaceDetailsCache[p]
	if not it then
		local jumpStatus, distance, fuelRequired, duration = player:GetHyperspaceDetails(path)
		it = { jumpStatus = jumpStatus, distance = distance, fuelRequired = fuelRequired, duration = duration }
		hyperspaceDetailsCache[p] = it
	end
	return it.jumpStatus, it.distance, it.fuelRequired, it.duration
end

function Windows.searchBar.Show()
	ui.text(lc.SEARCH)
	search_text, changed = ui.inputText("", search_text, {})
	if search_text ~= "" then
		local parsedSystem = changed and SystemPath.ParseString(search_text)
		if parsedSystem and parsedSystem ~= nil then
			sectorView:GotoSectorPath(parsedSystem)
		else
			local systempaths = sectorView:SearchNearbyStarSystemsByName(search_text)
			if #systempaths == 0 then
				ui.text(lc.NOT_FOUND)
			else
				local data = {}
				for _,path in pairs(systempaths) do
					local jumpStatus, distance, fuelRequired, duration = getHyperspaceDetails(path)
					table.insert(data, { jumpStatus = jumpStatus, distance = distance, fuelRequired = fuelRequired, duration = duration, path = path })
				end
				ui.child("search_results", function ()
					table.sort(data, function(a,b)
						if a.path == b.path or a.path:IsSameSystem(b.path) then
							return false
						end
						return a.distance < b.distance
					end)
					for _,item in pairs(data) do
						local system = item.path:GetStarSystem()
						local label = system.name
						label = label .. '  (' .. lui[item.jumpStatus] .. "), " .. string.format("%.2f", item.distance) .. lc.UNIT_LY .. ", " .. item.fuelRequired .. lc.UNIT_TONNES .. ", " .. ui.Format.Duration(item.duration, 2)
						if ui.selectable(label, false, {}) then
							sectorView:SwitchToPath(item.path)
						end
					end
				end)
			end
			-- calulating window size for next frame to fit some search results
			Windows.searchBar.size.y = Windows.searchBar.emptyHeight + math.min(#systempaths, MAX_SEARCH_STRINGS_VISIBLE) * ui.calcTextSize("ONE LINE").y
		end
	else
		-- calulating window size for the next frame for no search results
		Windows.searchBar.size.y = Windows.searchBar.emptyHeight
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
	ui.withStyleColors(w.style_colors, function() ui.window(w.name, w.params, w.Show) end)
end

local dummyFrames = 3

local function displaySectorViewWindow()
	player = Game.player
	if Game.CurrentView() == "sector" then
		if dummyFrames > 0 then -- do it a few frames, because imgui need a few frames to make the correct window size

			-- first, doing some one-time actions here
			-- calculating in-text icon size for used font size
			if not textIconSize then
				ui.withFont(font, function()
					textIconSize = ui.calcTextSize("H")
					textIconSize.x = textIconSize.y -- make square
				end)
				edgePadding = textIconSize
			end

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
				Windows.searchBar.emptyHeight = Windows.searchBar.size.y
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
	end
end

ui.registerModule("game", displaySectorViewWindow)
Event.Register("onGameStart", onGameStart)
Event.Register("onLeaveSystem", function()
	hyperspaceDetailsCache = {}
end)
-- events moved from hyperJumpPlanner
Event.Register("onGameEnd", hyperJumpPlanner.onGameEnd)
Event.Register("onEnterSystem", hyperJumpPlanner.onEnterSystem)
Event.Register("onShipEquipmentChanged", hyperJumpPlanner.onShipEquipmentChanged)

return {}
