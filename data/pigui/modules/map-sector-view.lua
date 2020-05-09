-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Game = require 'Game'
local utils = require 'utils'
local Event = require 'Event'
local SystemPath = require 'SystemPath'

local Lang = require 'Lang'
local lc = Lang.GetResource("core");
local lui = Lang.GetResource("ui-core");

local ui = require 'pigui'

local player = nil
local colors = ui.theme.colors
local icons = ui.theme.icons
local hideSectorViewWindows = false

local mainButtonSize = Vector2(32,32) * (ui.screenHeight / 1200)
local mainButtonFramePadding = 3
local function mainMenuButton(icon, selected, tooltip, color)
	color = color or colors.white
	return ui.coloredSelectedIconButton(icon, mainButtonSize, selected, mainButtonFramePadding, colors.buttonBlue, color, tooltip)
end

local sectorView

local onGameStart = function ()
	--connect to class SectorView
	sectorView = Game.sectorView
end

local function showSystemInfo(label, current_systempath, systempath, othersystempath)
	if systempath then
		local starsystem = systempath:GetStarSystem()
		local clicked = false
		ui.withID(label, function()
			local jumpData = ""
			if not current_systempath:IsSameSystem(systempath) then
				local jumpStatus, distance, fuelRequired, duration = player:GetHyperspaceDetails(current_systempath, systempath)
				jumpData = "\n" .. jumpStatus .. "  " .. string.format("%.2f", distance) .. lc.UNIT_LY .. "  " .. fuelRequired .. lc.UNIT_TONNES .. "  " .. ui.Format.Duration(duration, 2)
			end
			if ui.collapsingHeader(label .. ': ' .. starsystem.name .. '  ' .. jumpData .. " (" .. math.floor(systempath.sectorX) .. ", " .. math.floor(systempath.sectorY) .. ", " .. math.floor(systempath.sectorZ) .. ")", { "DefaultOpen" }) then
				local stars = starsystem:GetStars()
				for _,star in pairs(stars) do
					if ui.selectable(star.name, star.path == systempath, {}) then
						clicked = star.path
					end
				end
				if clicked then
					sectorView:SetSelected(clicked)
					sectorView:GotoSystemPath(clicked)
				end
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
				if next(starsystem.other_names) ~= nil then
					ui.text(table.concat(starsystem.other_names, ", "))
				end
				ui.text(starsystem.shortDescription)
				if othersystempath and not othersystempath:IsSameSystem(systempath) then
					local otherstarsystem = othersystempath:GetStarSystem()
					local jumpStatus, distance, fuelRequired, duration = player:GetHyperspaceDetails(systempath, othersystempath)
					ui.text('Relative to ' .. otherstarsystem.name .. ': ' .. jumpStatus .. "  " .. string.format("%.2f", distance) .. lc.UNIT_LY .. "  " .. fuelRequired .. lc.UNIT_TONNES .. "  " .. ui.Format.Duration(duration, 2))
				end
			end
		end)
	end
end

local search_text = ""
local draw_vertical_lines = false
local draw_out_range_labels = false
local draw_uninhabited_labels = true
local automatic_system_selection = true
local lock_hyperspace_target = false

local initialized
local function showSettings()
	if ui.collapsingHeader("Settings", { "DefaultOpen" }) then
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

local function showSearch()
	ui.text(lc.SEARCH)
	search_text, changed = ui.inputText("", search_text, {})
	if search_text ~= "" then
		local parsedSystem = SystemPath.ParseString(search_text)
		if parsedSystem ~= nil then
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
							sectorView:SetSelected(item.path)
							sectorView.GotoSystemPath(item.path)
						end
					end
				end)
			end
		end
	end
end

local function showInfoWindow()
	ui.withStyleColors({["WindowBg"] = colors.lightBlackBackground}, function()
		ui.window("MapSectorViewInfo", {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus", "HorizontalScrollbar"},
		function()
			ui.text(string.format("%.2f", sectorView:GetZoomLevel()) .. lc.UNIT_LY)
			ui.sameLine()
			local zoom_in = mainMenuButton(icons.zoom_in, false, "Zoom in")
			if zoom_in or ui.isItemActive() then
				sectorView:ZoomIn()
			end
			ui.sameLine()
			local zoom_out = mainMenuButton(icons.zoom_out, false, "Zoom out")
			if zoom_out or ui.isItemActive() then
				sectorView:ZoomOut()
			end
			ui.separator()
			local sector = sectorView:GetCenterSector()
			local distance = sectorView:GetCenterDistance()
			ui.text("Sector (" .. math.floor(sector.x) .. ", " .. math.floor(sector.y) .. ", " .. math.floor(sector.z) .. "), Distance " .. string.format("%.2f", distance) .. lc.UNIT_LY)
			ui.separator()
			local current = sectorView:GetCurrentSystemPath()
			local selected = sectorView:GetSelectedSystemPath()
			local hyperspaceTarget = sectorView:GetHyperspaceTargetSystemPath()

			showSystemInfo(lc.CURRENT_SYSTEM, current, current)
			ui.separator()
			showSystemInfo(lc.HYPERSPACE_TARGET, current, hyperspaceTarget, selected)
			local changed, lht = ui.checkbox(lock_hyperspace_target and lc.LOCKED or lc.FOLLOWING_SELECTION, lock_hyperspace_target)
			lock_hyperspace_target = lht
			if changed then
				sectorView:SetLockHyperspaceTarget(lock_hyperspace_target)
			end
			ui.separator()
			showSystemInfo(lc.SELECTED_SYSTEM, current, selected)
			ui.separator()
			showSettings()
			ui.separator()
			showSearch()
		end)
	end)
end

local function showFactionLegendWindow()
	ui.setNextWindowSize(Vector2(ui.screenWidth / 5, ui.screenHeight / 5), "Always")
	ui.setNextWindowPos(Vector2(ui.screenWidth - ui.screenWidth / 5 - 10, 10) , "Always")
	ui.window("MapSectorViewFactions", {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus"},
	function()
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
	end)
end

local function displaySectorViewWindow()
	if not initialized then
		sectorView:SetAutomaticSystemSelection(automatic_system_selection)
		sectorView:SetDrawOutRangeLabels(draw_out_range_labels)
		sectorView:SetDrawUninhabitedLabels(draw_uninhabited_labels)
		sectorView:SetDrawVerticalLines(draw_vertical_lines)
		initialized = true
	end
	player = Game.player
	local current_view = Game.CurrentView()
	if current_view == "sector" then
		ui.setNextWindowSize(Vector2(ui.screenWidth / 5, ui.screenHeight / 3 * 2.1), "Always")
		ui.setNextWindowPos(Vector2(10, 10) , "Always")
		if ui.isKeyReleased(ui.keys.tab) then
			hideSectorViewWindows = not hideSectorViewWindows;
		end
		if not hideSectorViewWindows then
			showInfoWindow()
			showFactionLegendWindow()
		end
	end
end

ui.registerModule("game", displaySectorViewWindow)
Event.Register("onGameStart", onGameStart)
Event.Register("onLeaveSystem", function()
	hyperspaceDetailsCache = {}
end)

return {}
