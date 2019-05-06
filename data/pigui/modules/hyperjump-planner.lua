local Engine = import('Engine')
local Game = import('Game')
local ui = import('pigui/pigui.lua')
local Event = import('Event')
local Lang = import("Lang")
local lc = Lang.GetResource("core")
local lui = Lang.GetResource("ui-core");
local Equipment = import("Equipment")

local player = nil
local colors = ui.theme.colors
local icons = ui.theme.icons

local mainButtonSize = Vector2(24,24) * (ui.screenHeight / 1200)
local mainButtonFramePadding = 3

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
local hideHyperJumpPlaner = false

local function showSettings()
	if ui.collapsingHeader(lui.SETTINGS, {"DefaultOpen"}) then
		local changed
		changed, remove_first_if_current = ui.checkbox(lui.REMOVE_WHEN_COMPLETED, remove_first_if_current)
	end
end -- showSettings

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

local function showInfo()
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
end -- showInfo

local function mainButton(icon, tooltip, callback)
	local button = ui.coloredSelectedIconButton(icon, mainButtonSize, false, mainButtonFramePadding, colors.buttonBlue, colors.white, tooltip)
	if button then
		callback()
	end
	return button
end --mainButton

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
	ui.setNextWindowSize(Vector2(ui.screenWidth / 5, (ui.screenHeight / 5) * 2), "Always")
	ui.setNextWindowPos(Vector2(ui.screenWidth - ui.screenWidth / 5 - 10, ui.screenHeight - ((ui.screenHeight / 5) * 2) - 10), "Always")
	ui.withStyleColors({["WindowBg"] = colors.lightBlackBackground}, function()
		ui.window("MapSectorViewHyperJumpPlanner", {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus"},
			function()
				ui.text(lui.HYPERJUMP_ROUTE)
				ui.separator()
				showInfo()
				ui.separator()
				showJumpRoute()
				ui.separator()
				showSettings()
		end)
	end)
end -- showHyperJumpPlannerWindow


local function displayHyperJumpPlanner()
	player = Game.player
	local current_view = Game.CurrentView()

	if current_view == "sector" and not Game.InHyperspace() then
		local drive = table.unpack(player:GetEquip("engine")) or nil
		local fuel_type = drive and drive.fuel or Equipment.cargo.hydrogen
		current_system = Game.system
		current_path = current_system.path
		current_fuel = player:CountEquip(fuel_type,"cargo")
		map_selected_path = Engine.GetSectorMapSelectedSystemPath()
		hyperjump_route = Engine.SectorMapGetRoute()
		route_jumps = Engine.SectorMapGetRouteSize()
		if ui.isKeyReleased(ui.keys.tab) then
			hideHyperJumpPlaner = not hideHyperJumpPlaner;
		end
		if not hideHyperJumpPlaner then
			showHyperJumpPlannerWindow()
		end
	end
end -- displayHyperJumpPlanner

ui.registerModule("game", displayHyperJumpPlanner)

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
