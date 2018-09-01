-- Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import('Engine')
local Game = import('Game')
local Space = import('Space')
local Vector = import('Vector')
local Format = import('Format')
local ui = import('pigui/pigui.lua')
local utils = import('utils')
local Lang = import("Lang")
local lui = Lang.GetResource("ui-core");

local colors = ui.theme.colors
local icons = ui.theme.icons

local ASTEROID_RADIUS = 1500000 -- rocky planets smaller than this (in meters) are considered an asteroid, not a planet

local function getBodyIcon(body)
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
		if body:IsMoon() then
			return icons.moon
		else
			local sb = body:GetSystemBody()
			if sb.radius < ASTEROID_RADIUS then
				return icons.asteroid_hollow
			else
				return icons.rocky_planet
			end
		end
	elseif body:IsShip() then
		local shipClass = body:GetShipClass()
		if icons[shipClass] then
			return icons[shipClass]
		else
			print("data/pigui/game.lua: getBodyIcon unknown ship class " .. (shipClass and shipClass or "nil"))
			return icons.ship -- TODO: better icon
		end
	elseif body:IsHyperspaceCloud() then
		return icons.hyperspace -- TODO: better icon
	elseif body:IsMissile() then
		return icons.bullseye -- TODO: better icon
	elseif body:IsCargoContainer() then
		return icons.rocky_planet
	else
		print("data/pigui/game.lua: getBodyIcon not sure how to process body, supertype: " .. (st and st or "nil") .. ", type: " .. (t and t or "nil"))
		utils.print_r(body)
		return icons.ship
	end
end

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
		ui.text(string.rep(" ", indent))
		ui.sameLine()
		ui.icon(getBodyIcon(body), Vector(16,16), colors.white)
		ui.sameLine()
		if ui.selectable(label or "UNKNOWN", is_target, {"SpanAllColumns"}) then
			Game.player:SetNavTarget(body)
		end
		if ui.isItemHovered() and ui.isMouseClicked(1) then
			ui.openDefaultRadialMenu(body)
		end
		ui.sameLine()
		if has_moons then
			ui.icon(icons.moon, Vector(16,16), colors.white)
			ui.sameLine(0,0.01)
		end
		if has_ground_stations then
			ui.icon(icons.starport, Vector(16,16), colors.white)
			ui.sameLine(0,0.01)
		end
		if has_space_stations then
			ui.icon(icons.spacestation, Vector(16,16), colors.white)
			ui.sameLine(0,0.01)
		end
		ui.nextColumn()
		ui.text(Format.Distance(body:DistanceTo(Game.player)))
		ui.nextColumn()
	end
	local children = entry.children or {}
	table.sort(children, sortFunction)
	for _,v in pairs(children) do
		showEntry(v, indent + 4, sortFunction)
	end
end
local shouldSortByPlayerDistance = false
local shouldShowStations = false
local shouldShowMoons = false
local filterText = ""
local ignore
local showWindow = false
local function showInfoWindow()
	if Game.player:IsDocked() then
		showWindow = false
	end
	local width_fraction = 5
	local height_fraction = 2
	local mainButtonSize = Vector(32,32) * (ui.screenHeight / 1200)
	local button_size = Vector(24,24) * (ui.screenHeight / 1200)
	local frame_padding = 1
	local bg_color = colors.buttonBlue
	local fg_color = colors.white
	if Game.player:IsDocked() then
		-- do nothing at all
	elseif not showWindow then
		ui.setNextWindowPos(Vector(ui.screenWidth - button_size.x * 3 - 10 , 10) , "Always")
		ui.window("SystemTargetsSmall", {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus"},
							function()
								if ui.coloredSelectedIconButton(icons.system_overview, mainButtonSize, false, frame_padding, bg_color, fg_color, "Show window") then
									showWindow = true
								end
		end)
	else
		ui.setNextWindowSize(Vector(ui.screenWidth / width_fraction, ui.screenHeight / height_fraction) , "Always")
		ui.setNextWindowPos(Vector(ui.screenWidth - (ui.screenWidth / width_fraction) - 10 , 10) , "Always")
		ui.withStyleColors({ ["WindowBg"] = colors.commsWindowBackground }, function()
				ui.withStyleVars({ ["WindowRounding"] = 0.0 }, function()
						ui.window("SystemTargets", {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus"},
											function()
												ui.withFont(ui.fonts.pionillium.medium.name, ui.fonts.pionillium.medium.size, function()
																			--											ignore, shouldSortByPlayerDistance = ui.checkbox("Player Distance", shouldSortByPlayerDistance)
																			if ui.coloredSelectedIconButton(icons.distance, button_size, shouldSortByPlayerDistance, frame_padding, bg_color, fg_color, lui.TOGGLE_OVERVIEW_SORT_BY_PLAYER_DISTANCE) then
																				shouldSortByPlayerDistance = not shouldSortByPlayerDistance
																			end
																			ui.sameLine()
																			--											ignore, shouldShowMoons = ui.checkbox("Moons", shouldShowMoons)
																			if ui.coloredSelectedIconButton(icons.moon, button_size, shouldShowMoons, frame_padding, bg_color, fg_color, lui.TOGGLE_OVERVIEW_SHOW_MOONS) then
																				shouldShowMoons = not shouldShowMoons
																			end
																			ui.sameLine()
																			-- ignore, shouldShowStations = ui.checkbox("Stations", shouldShowStations)
																			if ui.coloredSelectedIconButton(icons.filter_stations, button_size, shouldShowStations, frame_padding, bg_color, fg_color, lui.TOGGLE_OVERVIEW_SHOW_STATIONS) then
																				shouldShowStations = not shouldShowStations
																			end
																			ui.sameLine()
																			ui.dummy(Vector(ui.screenWidth / width_fraction - 6 * button_size.x - 7 * frame_padding, 0)) -- magical calculation :-/
																			ui.sameLine()
																			if ui.coloredSelectedIconButton(icons.system_overview, button_size, false, frame_padding, bg_color, fg_color, lui.TOGGLE_OVERVIEW_WINDOW) then
																				showWindow = false
																			end
																			filterText, ignore = ui.inputText("", filterText, {})
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
												end)
						end)
				end)
		end)
	end
end
ui.registerModule("game", showInfoWindow)
ui.toggleSystemTargets = function()
	showWindow = not showWindow
end
return {}
