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

local sectorMapTab = 'info'

-- hyperjump route stuff
local hyperjump_route = {}
local route_jumps = 0
local auto_route_min_jump = 2 -- minimum jump distance when auto routing
local current_system
local current_path
local map_selected_path
local selected_jump
local current_fuel
local remove_first_if_current = true

local function mainMenuButton(icon, selected, tooltip, color)
	color = color or colors.white
	return ui.coloredSelectedIconButton(icon, mainButtonSize, selected, mainButtonFramePadding, colors.buttonBlue, color, tooltip)
end


local function setWindowPos()
	ui.setNextWindowPos(Vector2(mainButtonSize.x + (mainButtonSize.x / 2) + 10, mainButtonSize.x + (mainButtonSize.x / 2) + 15) , "Always")
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
										Engine.SetSectorMapSelected(clicked)
										Engine.SectorMapGotoSystemPath(clicked)
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
	ui.setNextWindowSize(Vector2(ui.screenWidth / 5, ui.screenHeight / 5.8), "Always")
	setWindowPos()
	ui.withStyleColors({["WindowBg"] = colors.commsWindowBackground}, function()
		ui.window("MapSectorViewInfo", {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus", "HorizontalScrollbar"},
			function()
				ui.text("Settings")
				ui.separator()
				local changed
				changed, draw_vertical_lines = ui.checkbox(lc.DRAW_VERTICAL_LINES, draw_vertical_lines)
				if changed then
					Engine.SetSectorMapDrawVerticalLines(draw_vertical_lines)
				end
				changed, draw_out_range_labels = ui.checkbox(lc.DRAW_OUT_RANGE_LABELS, draw_out_range_labels)
				if changed then
					Engine.SetSectorMapDrawOutRangeLabels(draw_out_range_labels)
				end
				changed, draw_uninhabited_labels = ui.checkbox(lc.DRAW_UNINHABITED_LABELS, draw_uninhabited_labels)
				if changed then
					Engine.SetSectorMapDrawUninhabitedLabels(draw_uninhabited_labels)
				end
				changed, automatic_system_selection = ui.checkbox(lc.AUTOMATIC_SYSTEM_SELECTION, automatic_system_selection)
				if changed then
					Engine.SetSectorMapAutomaticSystemSelection(automatic_system_selection)
				end
				changed, remove_first_if_current = ui.checkbox(lui.REMOVE_WHEN_COMPLETED, remove_first_if_current)
			end)	
		end)
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
	ui.setNextWindowSize(Vector2(ui.screenWidth / 5, ui.screenHeight / 3 * 2.1), "Always")
	setWindowPos()
	ui.withStyleColors({["WindowBg"] = colors.commsWindowBackground}, function()
		ui.window("MapSectorViewInfo", {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus", "HorizontalScrollbar"},
			function()
				ui.text(lc.SEARCH)
				search_text, changed = ui.inputText("", search_text, {})
				if search_text ~= "" then
					local parsedSystem = SystemPath.ParseString(search_text)
					if parsedSystem ~= nil then
						Engine.SectorMapGotoSectorPath(parsedSystem)
					else
						local systempaths = Engine.SearchNearbyStarSystemsByName(search_text)
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
														 Engine.SetSectorMapSelected(item.path)
														 Engine.SectorMapGotoSystemPath(item.path)
													 end
												 end
							end)
						end
					end
				end
			end)
		end)
end

local function showInfoWindow()
	ui.setNextWindowSize(Vector2(ui.screenWidth / 5, ui.screenHeight / 2.25), "Always")
	setWindowPos()
	ui.withStyleColors({["WindowBg"] = colors.commsWindowBackground}, function()
			ui.window("MapSectorViewInfo", {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus", "HorizontalScrollbar"},
				function()
					local sector = Engine.GetSectorMapCenterSector()
					local distance = Engine.GetSectorMapCenterDistance()
					ui.text("Sector (" .. math.floor(sector.x) .. ", " .. math.floor(sector.y) .. ", " .. math.floor(sector.z) .. "), Distance " .. string.format("%.2f", distance) .. lc.UNIT_LY)
					ui.separator()
					local current = Engine.GetSectorMapCurrentSystemPath()
					local selected = Engine.GetSectorMapSelectedSystemPath()
					local hyperspaceTarget = Engine.GetSectorMapHyperspaceTargetSystemPath()

					showSystemInfo(lc.CURRENT_SYSTEM, current, current)
					ui.separator()
					showSystemInfo(lc.HYPERSPACE_TARGET, current, hyperspaceTarget, selected)
					local changed, lht = ui.checkbox(lock_hyperspace_target and lc.LOCKED or lc.FOLLOWING_SELECTION, lock_hyperspace_target)
					lock_hyperspace_target = lht
					if changed then
						Engine.SetSectorMapLockHyperspaceTarget(lock_hyperspace_target)
					end
					ui.separator()
					showSystemInfo(lc.SELECTED_SYSTEM, current, selected)
			end)
	end)
end
local function showFactionLegendWindow()
	ui.setNextWindowSize(Vector2(ui.screenWidth / 5, ui.screenHeight / 2.25), "Always")
	setWindowPos()
	--ui.setNextWindowSize(Vector2(ui.screenWidth / 5, ui.screenHeight / 5), "Always")
	--ui.setNextWindowPos(Vector2(ui.screenWidth - ui.screenWidth / 5 - 10, 10) , "Always")
	ui.withStyleColors({["WindowBg"] = colors.commsWindowBackground}, function()
		ui.window("MapSectorViewFactions", {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus"},
							function()
								ui.text("Factions")
								local factions = Engine.GetSectorMapFactions()
								for _,f in pairs(factions) do
									local changed, value
									ui.withStyleColors({ ["Text"] = Color(f.faction.colour.r, f.faction.colour.g, f.faction.colour.b) }, function()
											changed, value = ui.checkbox(f.faction.name, f.visible)
									end)
									if changed then
										Engine.SetSectorMapFactionVisible(f.faction, value)
									end
								end
		end)
	end)
end

local function showSectorZoomWindow()
	ui.setNextWindowSize(Vector2((mainButtonSize.x + (mainButtonSize.x / 2) + 5) * 4, mainButtonSize.x + (mainButtonSize.x / 2) + 10 ), "Always")
	ui.setNextWindowPos(Vector2(5, 1) , "Always")
	ui.window("MapSectorZoom", {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus"},
		function()
			ui.text(string.format("%.2f", Engine.GetSectorMapZoomLevel()) .. lc.UNIT_LY)
			ui.sameLine()
			local zoom_in = mainMenuButton(icons.zoom_in, false, "Zoom in")
			if zoom_in or ui.isItemActive() then
				Engine.SectorMapZoomIn()
			end
			ui.sameLine()
			local zoom_out = mainMenuButton(icons.zoom_out, false, "Zoom out")
			if zoom_out or ui.isItemActive() then
				Engine.SectorMapZoomOut()
			end
		end
	)
end

local function showSectorViewTabs()
	ui.setNextWindowSize(Vector2(mainButtonSize.x + (mainButtonSize.x / 2) + 5 , (mainButtonSize.x + (mainButtonSize.x / 2) + 5) * 5), "Always")
	ui.setNextWindowPos(Vector2(5, mainButtonSize.x + (mainButtonSize.x / 2) + 10 ) , "Always")
	ui.window("MapSectorViewTabs", {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus"},
		function()
			local info = mainMenuButton(icons.info, sectorMapTab == "info", "Sector Information")
			if info then
				if sectorMapTab == 'info' then
					sectorMapTab = 'none'
				else
					sectorMapTab = 'info'
				end
			end
			local factions = mainMenuButton(icons.bookmarks, sectorMapTab == "faction", "Factions")
			if factions then
				if sectorMapTab == 'faction' then
					sectorMapTab = 'none'
				else
					sectorMapTab = 'faction'
				end
			end
			local search = mainMenuButton(icons.search_lens, sectorMapTab == "search", "Search")
			if search then
				if sectorMapTab == 'search' then
					sectorMapTab = 'none'
				else
					sectorMapTab = 'search'
				end
			end
			local hyperjumpRoute = mainMenuButton(icons.hyperspace, sectorMapTab == "hyperjump", "Hyperjump Route")
			if hyperjumpRoute then
				if sectorMapTab == 'hyperjump' then
					sectorMapTab = 'none'
				else
					sectorMapTab = 'hyperjump'
				end
			end			
			local settings = mainMenuButton(icons.settings, sectorMapTab == "settings", "Settings")
			if settings then
				if sectorMapTab == 'settings' then
					sectorMapTab = 'none'
				else
					sectorMapTab = 'settings'
				end
			end
		end)
		
end --showSectorViewTabs

local function showRouteInfo()
	if ui.collapsingHeader(lui.ROUTE_INFO,{"DefaultOpen"}) then
		local total_fuel = 0
		local total_duration = 0
		local total_distance = 0

		local start = current_path
		-- Tally up totals for the entire jump plan
		for _,jump in pairs(hyperjump_route) do
			local status, distance, fuel, duration = player:GetHyperspaceDetails(start, jump)

			total_fuel = total_fuel + fuel
			total_duration = total_duration + duration
			total_distance = total_distance + distance

			start = jump
		end

		ui.text(lui.CURRENT_SYSTEM .. ": " .. current_system.name .. " (" .. current_path.sectorX .. "," .. current_path.sectorY .. "," .. current_path.sectorZ ..")")
		ui.text(lui.FINAL_TARGET)

		if route_jumps > 0 then
			local final_path = hyperjump_route[route_jumps]
			local final_sys = final_path:GetStarSystem()
			ui.sameLine()
			ui.text(final_sys.name .. " (" .. final_path.sectorX .. "," .. final_path.sectorY .. "," .. final_path.sectorZ .. ")")
		end
		ui.text(lui.CURRENT_FUEL .. " " .. current_fuel .. lc.UNIT_TONNES)
		ui.sameLine()
		ui.text(lui.REQUIRED_FUEL .. " " .. total_fuel .. lc.UNIT_TONNES)

		ui.text(lui.TOTAL_DURATION .. " " ..ui.Format.Duration(total_duration, 2))
		ui.sameLine()
		ui.text(lui.TOTAL_DISTANCE .. " " ..string.format("%.2f", total_distance) .. lc.UNIT_LY)
	end
end -- showRouteInfo

local function mainButton(icon, tooltip, callback)
	local button = ui.coloredSelectedIconButton(icon, mainButtonSize, false, mainButtonFramePadding, colors.buttonBlue, colors.white, tooltip)
	if button then
		callback()
	end
	return button
end --mainButton

local function showJumpData(start, target, status, distance, fuel, duration, short)
	--local color = status == "OK" and colors.white or colors.alertRed	-- TODO: dedicated colors?
	local color = colors.white
	if short then
		ui.withStyleColors({["Text"] = color}, function()

			ui.text(target:GetStarSystem().name)
			ui.sameLine()
			ui.text("("..fuel .. lc.UNIT_TONNES..")")
		end)
	else
		ui.withStyleColors({["Text"] = color}, function()
			ui.text(start:GetStarSystem().name)
			ui.sameLine()
			ui.text("->")
			ui.sameLine()
			ui.text(target:GetStarSystem().name)
			ui.sameLine()
			ui.text(":")
			ui.sameLine()
			ui.text(string.format("%.2f", distance) .. lc.UNIT_LY)
			ui.sameLine()
			ui.text(fuel .. lc.UNIT_TONNES)
			ui.sameLine()
			ui.text(ui.Format.Duration(duration, 2))
		end)
	end
end -- showJumpData

local function showJumpRoute()
	if ui.collapsingHeader(lui.ROUTE_JUMPS, {"DefaultOpen"}) then
		mainButton(icons.forward, lui.ADD_JUMP,
			function()
				Engine.SectorMapAddToRoute(map_selected_path)
		end)
		ui.sameLine()

		mainButton(icons.current_line, lui.REMOVE_JUMP,
			function()
				local new_route = {}
				local new_count = 0
				if selected_jump then
					Engine.SectorMapRemoveRouteItem(selected_jump)
				end
		end)
		ui.sameLine()

		mainButton(icons.current_periapsis, lui.MOVE_UP,
			function()
				if selected_jump then
					if Engine.SectorMapMoveRouteItemUp(selected_jump) then
						selected_jump = selected_jump - 1
					end
				end
		end)
		ui.sameLine()

		mainButton(icons.current_apoapsis, lui.MOVE_DOWN,
			function()
				if selected_jump then
					if Engine.SectorMapMoveRouteItemDown(selected_jump) then
						selected_jump = selected_jump + 1
					end
				end
		end)
		ui.sameLine()

		mainButton(icons.retrograde_thin, lui.CLEAR_ROUTE,
			function()
				Engine.SectorMapClearRoute()
				selected_jump = nil
		end)
		ui.sameLine()

		mainButton(icons.hyperspace, lui.AUTO_ROUTE,
			function()
				Engine.SectorMapAutoRoute()

		end)
		ui.sameLine()

		mainButton(icons.search_lens, lui.CENTER_ON_SYSTEM,
			function()
				if selected_jump then
					Engine.SectorMapGotoSystemPath(hyperjump_route[selected_jump])
				end
		end)

		ui.separator()

		local start = current_path
		local clicked
		local running_fuel = 0
		for jumpIndex, jump in pairs(hyperjump_route) do
			local jump_sys = jump:GetStarSystem()
			local status, distance, fuel, duration = player:GetHyperspaceDetails(start, jump)
			local color
			local remaining_fuel = current_fuel - running_fuel - fuel

			if remaining_fuel == 0 then
				color = colors.alertYellow
			else
				if remaining_fuel < 0 then
					color = colors.alertRed
				else
					color = colors.white
				end
			end

			ui.withStyleColors({["Text"] = color},
				function()
					if ui.selectable(jumpIndex..": ".. jump_sys.name .. " (" .. string.format("%.2f", distance) .. lc.UNIT_LY .. " - " .. fuel .. lc.UNIT_TONNES..")", jumpIndex == selected_jump, {}) then
						clicked = jumpIndex
					end
			end)
			running_fuel = fuel + running_fuel
			start = jump
		end

		if clicked then
			selected_jump = clicked
		end
	end
end -- showJumpPlan

local function showHyperJumpPlannerWindow()
	ui.setNextWindowSize(Vector2(ui.screenWidth / 5, ui.screenHeight / 3 * 2.1), "Always")
	setWindowPos()
	ui.withStyleColors({["WindowBg"] = colors.commsWindowBackground}, function()
		ui.window("MapSectorViewHyperJumpPlanner", {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus"},
			function()
				ui.text(lui.HYPERJUMP_ROUTE)
				ui.separator()
				showRouteInfo()
				ui.separator()
				showJumpRoute()
		end)
	end)
end -- showHyperJumpPlannerWindow

local function displaySectorViewWindow()
	if not initialized then
		Engine.SetSectorMapAutomaticSystemSelection(automatic_system_selection)
		Engine.SetSectorMapDrawOutRangeLabels(draw_out_range_labels)
		Engine.SetSectorMapDrawUninhabitedLabels(draw_uninhabited_labels)
		Engine.SetSectorMapDrawVerticalLines(draw_vertical_lines)
		initialized = true
	end
	player = Game.player
	local current_view = Game.CurrentView()
	if current_view == "sector" then
		
		-- hyperjump planner stuff
		local drive = table.unpack(player:GetEquip("engine")) or nil
		local fuel_type = drive and drive.fuel or Equipment.cargo.hydrogen
		current_system = Game.system
		current_path = current_system.path
		current_fuel = player:CountEquip(fuel_type,"cargo")
		map_selected_path = Engine.GetSectorMapSelectedSystemPath()
		hyperjump_route = Engine.SectorMapGetRoute()
		route_jumps = Engine.SectorMapGetRouteSize()
	
	
		if ui.isKeyReleased(ui.keys.tab) then
			hideSectorViewWindows = not hideSectorViewWindows;
		end
		if not hideSectorViewWindows then
			showSectorZoomWindow()
			showSectorViewTabs()
			if sectorMapTab == 'info' then
				showInfoWindow()
			elseif sectorMapTab == 'search' then
				showSearch()
			elseif sectorMapTab == 'settings' then
				showSettings()
			elseif sectorMapTab == 'hyperjump' then
				showHyperJumpPlannerWindow()
			elseif sectorMapTab == 'faction' then
				showFactionLegendWindow()
			end
		end
	end
end

ui.registerModule("game", displaySectorViewWindow)
Event.Register("onLeaveSystem", function()
								 hyperspaceDetailsCache = {}
end)


Event.Register("onEnterSystem",
	function(ship)
		-- remove the first jump if it's the current system (and enabled to do so)
		-- this should be the case if you are following a route and want the route to be
		-- updated as you make multiple jumps
		if ship:IsPlayer() and remove_first_if_current then
			if route_jumps > 0 and hyperjump_route[1]:IsSameSystem(Game.system.path) then
				Engine.SectorMapRemoveRouteItem(1)
			end
		end
end)

Event.Register("onGameEnd",
	function(ship)
		-- clear the route out so it doesn't show up if the user starts a new game
		Engine.SectorMapClearRoute()
end)

return {}
