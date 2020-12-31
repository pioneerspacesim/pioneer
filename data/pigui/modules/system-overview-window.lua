-- Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Game = require 'Game'
local Space = require 'Space'
local Format = require 'Format'
local utils = require 'utils'

local Lang = require 'Lang'
local lui = Lang.GetResource("ui-core");

local ui = require 'pigui'

local colors = ui.theme.colors
local icons = ui.theme.icons

local getBodyIcon = require 'pigui.modules.flight-ui.body-icons'
local iconSize = Vector2(24,24)
local bodyIconSize = Vector2(18,18)
local width_fraction = ui.rescaleUI(6, Vector2(1920, 1200))
local height_fraction = 2
local button_size = Vector2(32,32) * (ui.screenHeight / 1200)
local frame_padding = 1
local bg_color = colors.buttonBlue
local fg_color = colors.white

local function sortByPlayerDistance(a,b)
	if a.body == nil then
		return false;
	end

	if b.body == nil then
		return false;
	end

	return a.body:DistanceTo(Game.player) < b.body:DistanceTo(Game.player)
end

local function sortBySystemDistance(a,b)
	return (a.systemBody.periapsis + a.systemBody.apoapsis) < (b.systemBody.periapsis + b.systemBody.apoapsis)
end
local function calculateEntry(systemBody, parent, navTarget, filterFunction, always_include)
	local body = systemBody.body
	local result = nil
	local should_discard = false
	local is_target = body == navTarget
	if body then
		result = { systemBody = systemBody,
			body = body,
			label = body.label,
			children = {},
			is_target = is_target,
			has_space_stations = false,
			has_ground_stations = false,
			has_moons = false,
		}
		if not filterFunction(body) then
			should_discard = true
		end
		if body:IsSpaceStation() then
			parent.has_space_stations = true
		elseif body:IsGroundStation() then
			parent.has_ground_stations = true
		elseif body:IsMoon() then
			parent.has_moons = true
		end
	else
		result = { systemBody = systemBody,
			body = nil,
			label = systemBody.name,
			children = {},
			is_target = false,
			has_space_stations = false,
			has_ground_stations = false,
			has_moons = false,
		}
	end

	local children = systemBody.children or {}
	for _,v in pairs(children) do
		local c = calculateEntry(v, result, navTarget, filterFunction, is_target)
		if c then
			should_discard = false
			table.insert(result.children, c)
		end
	end
	if should_discard and not always_include and not is_target then
		return nil
	else
		return result
	end
end
local function showEntry(entry, indent, sortFunction)
	local body = entry.body
	local is_target = entry.is_target
	local label = entry.label
	local has_ground_stations = entry.has_ground_stations
	local has_space_stations = entry.has_space_stations
	local has_moons = entry.has_moons
	if body then
		ui.dummy(Vector2(iconSize.x * indent / 2.0, iconSize.y))
		ui.sameLine()
		ui.icon(getBodyIcon(body), iconSize, colors.white)
		ui.sameLine()
		local pos = ui.getCursorPos()
		if ui.selectable("##" .. (label or "UNKNOWN"), is_target, {"SpanAllColumns"}, Vector2(0, iconSize.y)) then
			Game.player:SetNavTarget(body)
			ui.playSfx("OK")
		end
		if ui.isItemHovered() and ui.isMouseClicked(1) then
			ui.openDefaultRadialMenu(body)
		end
		ui.setCursorPos(pos)
		ui.alignTextToLineHeight(iconSize.y)
		ui.text(label or "UNKNOWN")
		ui.sameLine()
		if has_moons then
			ui.icon(icons.moon, bodyIconSize, colors.white)
			ui.sameLine(0,0.01)
		end
		if has_ground_stations then
			ui.icon(icons.starport, bodyIconSize, colors.white)
			ui.sameLine(0,0.01)
		end
		if has_space_stations then
			ui.icon(icons.spacestation, bodyIconSize, colors.white)
			ui.sameLine(0,0.01)
		end
		ui.nextColumn()
		ui.dummy(Vector2(0, iconSize.y))
		ui.sameLine()
		ui.alignTextToLineHeight(iconSize.y)
		ui.text(Format.Distance(body:DistanceTo(Game.player)))
		ui.nextColumn()
	end
	local children = entry.children or {}
	table.sort(children, sortFunction)
	for _,v in pairs(children) do
		showEntry(v, indent + 1, sortFunction)
	end
end


local shouldSortByPlayerDistance = false
local shouldShowStations = false
local shouldShowMoons = false
local filterText = ""
local showWindow = false

local function drawBodyList()
	if ui.coloredSelectedIconButton(icons.distance, button_size, shouldSortByPlayerDistance, frame_padding, bg_color, fg_color, lui.TOGGLE_OVERVIEW_SORT_BY_PLAYER_DISTANCE) then
		shouldSortByPlayerDistance = not shouldSortByPlayerDistance
	end
	ui.sameLine()
	if ui.coloredSelectedIconButton(icons.moon, button_size, shouldShowMoons, frame_padding, bg_color, fg_color, lui.TOGGLE_OVERVIEW_SHOW_MOONS) then
		shouldShowMoons = not shouldShowMoons
	end
	ui.sameLine()
	if ui.coloredSelectedIconButton(icons.filter_stations, button_size, shouldShowStations, frame_padding, bg_color, fg_color, lui.TOGGLE_OVERVIEW_SHOW_STATIONS) then
		shouldShowStations = not shouldShowStations
	end
	ui.sameLine(ui.getWindowSize().x - (button_size.x + 10))
	if ui.coloredSelectedIconButton(icons.system_overview, button_size, false, frame_padding, bg_color, fg_color, lui.TOGGLE_OVERVIEW_WINDOW) then
		showWindow = false
	end

	filterText = ui.inputText("", filterText, {})
	ui.sameLine()
	ui.icon(icons.filter_bodies, button_size, colors.frame, lui.OVERVIEW_NAME_FILTER)

	local sortFunction = shouldSortByPlayerDistance and sortByPlayerDistance or sortBySystemDistance
	local filterFunction = function(body)
		if body then
			-- only plain text matches, no regexes
			if filterText ~= "" and filterText ~= nil and not string.find(body.label:lower(), filterText:lower(), 1, true) then
				return false
			end
			if (not shouldShowMoons) and body:IsMoon() then
				return false
			elseif (not shouldShowStations) and body:IsStation() then
				return false
			end
		end
		return true
	end

	ui.child("spaceTargets", function()
		local root = Space.rootSystemBody
		local tree = calculateEntry(root, nil, Game.player:GetNavTarget(), filterFunction, false)
		if tree then
			ui.columns(2, "spaceTargetColumnsOn", false) -- no border
			ui.setColumnOffset(1, ui.screenWidth / width_fraction * 0.66)
			showEntry(tree, 0, sortFunction)
			ui.columns(1, "spaceTargetColumnsOff", false) -- no border
			ui.radialMenu("systemoverviewspacetargets")
		else
			ui.text(lui.NO_FILTER_MATCHES)
		end
	end)
end

local function showInfoWindow()
	if Game.CurrentView() == "world" then
		if Game.InHyperspace() or not Game.system.explored then
			showWindow = false
		end
		if not showWindow then
			ui.setNextWindowPos(Vector2(ui.screenWidth - button_size.x * 3 - 10 , 10) , "Always")
			ui.window("SystemTargetsSmall", {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus", "NoSavedSettings"},
			function()
				if ui.coloredSelectedIconButton(icons.system_overview, button_size, false, frame_padding, bg_color, fg_color, lui.TOGGLE_OVERVIEW_WINDOW) then
					showWindow = true
				end
			end)
		else
			ui.setNextWindowSize(Vector2(ui.screenWidth / width_fraction, ui.screenHeight / height_fraction) , "Always")
			ui.setNextWindowPos(Vector2(ui.screenWidth - (ui.screenWidth / width_fraction) - 10 , 10) , "Always")
			ui.withStyleColorsAndVars({ ["WindowBg"] = colors.commsWindowBackground }, { ["WindowRounding"] = 0.0 }, function()
				ui.window("SystemTargets", {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus"}, function()
					ui.withFont(ui.fonts.pionillium.medium, drawBodyList)
				end)
			end)

			if ui.ctrlHeld() and ui.isKeyReleased(ui.keys.delete) then
				package.reimport()
			end
		end
	end
end

ui.registerModule("game", {
	id = "system-overview-window",
	draw = showInfoWindow
})
ui.toggleSystemTargets = function()
	showWindow = not showWindow
end
return {}
