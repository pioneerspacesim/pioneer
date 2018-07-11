local Engine = import('Engine')
local Game = import('Game')
local ui = import('pigui/pigui.lua')
local Vector = import('Vector')
local Event = import('Event')
local Lang = import("Lang")
local lc = Lang.GetResource("core")
local lui = Lang.GetResource("ui-core");
local Equipment = import("Equipment")
local utils = import('utils')

local player = nil
local colors = ui.theme.colors
local icons = ui.theme.icons

local mainButtonSize = Vector(24,24) * (ui.screenHeight / 1200)
local mainButtonFramePadding = 3

local function showDvLine(rightIcon, resetIcon, leftIcon, key, Formatter, rightTooltip, resetTooltip, leftTooltip)
	local wheel = function()
		if ui.isItemHovered() then
			local w = ui.getMouseWheel()
			if w ~= 0 then
				Engine.TransferPlannerAdd(key, w * 10)
			end
		end
	end
	local press = ui.coloredSelectedIconButton(leftIcon, mainButtonSize, false, mainButtonFramePadding, colors.buttonBlue, colors.white, leftTooltip)
	if press or (key ~= "factor" and ui.isItemActive()) then
		Engine.TransferPlannerAdd(key, -10)
	end
	wheel()
	ui.sameLine()
	if ui.coloredSelectedIconButton(resetIcon, mainButtonSize, false, mainButtonFramePadding, colors.buttonBlue, colors.white, resetTooltip) then
		Engine.TransferPlannerReset(key)
	end
	wheel()
	ui.sameLine()
	press = ui.coloredSelectedIconButton(rightIcon, mainButtonSize, false, mainButtonFramePadding, colors.buttonBlue, colors.white, rightTooltip)
	if press or (key ~= "factor" and ui.isItemActive()) then
		Engine.TransferPlannerAdd(key, 10)
	end
	wheel()
	ui.sameLine()
	local speed, speed_unit = Formatter(Engine.TransferPlannerGet(key))
	ui.text(speed .. " " .. speed_unit)
	return 0
end
local time_selected_button_icon
local function timeButton(icon, tooltip, factor)
	if ui.coloredSelectedIconButton(icon, mainButtonSize, false, mainButtonFramePadding, colors.buttonBlue, colors.white, tooltip) then
		time_selected_button_icon = icon
	end
	local active = ui.isItemActive()
	if active then
		Engine.SystemMapAccelerateTime(factor)
	end
	ui.sameLine()
	return active
end
local ship_drawing = "off"
local show_lagrange = "off"
local nextShipDrawings = { ["off"] = "boxes", ["boxes"] = "orbits", ["orbits"] = "off" }
local nextShowLagrange = { ["off"] = "icon", ["icon"] = "icontext", ["icontext"] = "off" }
local function showOrbitPlannerWindow()
	ui.setNextWindowSize(Vector(ui.screenWidth / 5, (ui.screenHeight / 5) * 2), "Always")
	ui.setNextWindowPos(Vector(ui.screenWidth - ui.screenWidth / 5 - 10, (ui.screenHeight / 5) * 2 + 20), "Always")
	ui.withStyleColors({["WindowBg"] = colors.lightBlackBackground}, function()
			ui.window("OrbitPlannerWindow", {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus"},
								function()
									ui.text("Orbit planner")
									ui.separator()
									if ui.coloredSelectedIconButton(icons.ship, mainButtonSize, showShips, mainButtonFramePadding, colors.buttonBlue, colors.white, "Show ships") then
										ship_drawing = nextShipDrawings[ship_drawing]
										Engine.SystemMapSetShipDrawing(ship_drawing);
									end
									ui.sameLine()
									if ui.coloredSelectedIconButton(icons.autopilot_medium_orbit, mainButtonSize, showLagrangePoints, mainButtonFramePadding, colors.buttonBlue, colors.white, "Show Lagrange points") then
										show_lagrange = nextShowLagrange[show_lagrange]
										Engine.SystemMapSetShowLagrange(show_lagrange);
									end
									ui.sameLine()
									ui.coloredSelectedIconButton(icons.zoom_in, mainButtonSize, false, mainButtonFramePadding, colors.buttonBlue, colors.white, "Zoom in")
									if ui.isItemActive() then
										Engine.SystemMapZoom("in")
									end
									ui.sameLine()
									ui.coloredSelectedIconButton(icons.zoom_out, mainButtonSize, false, mainButtonFramePadding, colors.buttonBlue, colors.white, "Zoom out")
									if ui.isItemActive() then
										Engine.SystemMapZoom("out")
									end
									ui.separator()

									showDvLine(icons.left, icons.maneuver, icons.right, "factor", function(i) return i, "x" end, "Decrease delta factor", "Reset delta factor", "Increase delta factor")
									showDvLine(icons.left, icons.eta, icons.right, "starttime",
																					 function(i)
																						 local now = Game.time
																						 local start = Engine.SystemMapGetOrbitPlannerStartTime()
																						 if start then
																							 return ui.Format.Duration(math.floor(start - now)), ""
																						 else
																							 return "now", ""
																						 end
																					 end,
																					 "Decrease time", "Reset time", "Increase time")
									showDvLine(icons.prograde, icons.maneuver, icons.retrograde, "prograde", ui.Format.Speed, "Thrust prograde", "Reset prograde thrust", "Thrust retrograde")
									showDvLine(icons.normal, icons.maneuver, icons.antinormal, "normal", ui.Format.Speed, "Thrust normal", "Reset normal thrust", "Thrust antinormal")
									showDvLine(icons.radial_in, icons.maneuver, icons.radial_out, "radial", ui.Format.Speed, "Thrust radially in", "Reset radial thrust", "Thrust radially out")

									ui.separator()
									local t = Engine.SystemMapGetOrbitPlannerTime()
									ui.text(t and ui.Format.Datetime(t) or "now")
									local r = false
									r = timeButton(icons.time_backward_100x, "-100x",-10000000) or r
									r = timeButton(icons.time_backward_10x, "-10x", -100000) or r
									r = timeButton(icons.time_backward_1x, "-1x", -1000) or r
									r = timeButton(icons.time_center, "Now", nil) or r
									r = timeButton(icons.time_forward_1x, "1x", 1000) or r
									r = timeButton(icons.time_forward_10x, "10x", 100000) or r
									r = timeButton(icons.time_forward_100x, "100x", 10000000) or r
									if not r then
										if time_selected_button_icon == icons.time_center then
											Engine.SystemMapAccelerateTime(nil)
										else
											Engine.SystemMapAccelerateTime(0.0)
										end
									end
			end)
	end)
end
local function showTargetInfoWindow(systemBody)
	if not systemBody then
		return
	end
	ui.setNextWindowSize(Vector(ui.screenWidth / 5, (ui.screenHeight / 5) * 2), "Always")
	ui.setNextWindowPos(Vector(20, (ui.screenHeight / 5) * 2 + 20), "Always")
	ui.withStyleColors({["WindowBg"] = colors.lightBlackBackground}, function()
			ui.window("TargetInfoWindow", {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus"},
								function()
									ui.text(systemBody.name)
									ui.text(ui.Format.Duration(systemBody.rotationPeriod * 24 * 60 * 60, 2))
									local v,u = ui.Format.Distance(systemBody.radius)
									ui.text(v .. " " .. u)
									v,u = ui.Format.Distance(systemBody.semiMajorAxis)
									ui.text(v .. " " .. u)
									ui.text(ui.Format.Duration(systemBody.orbitPeriod * 24 * 60 * 60, 2))
			end)
	end)
end
local function displaySystemViewUI()
	player = Game.player
	local current_view = Game.CurrentView()

	if current_view == "system" and not Game.InHyperspace() then
		showOrbitPlannerWindow()
		showTargetInfoWindow(Engine.SystemMapSelectedObject())
	end
end

ui.registerModule("game", displaySystemViewUI)

return {}
