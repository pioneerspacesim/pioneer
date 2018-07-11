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

									showDvLine(icons.left, icons.maneuver, icons.right, "factor", function(i) return i, "x" end, "Increase delta factor", "Reset delta factor", "Decrease delta factor")
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
														 "Increase time", "Reset time", "Decrease time")
									showDvLine(icons.prograde, icons.maneuver, icons.retrograde, "prograde", ui.Format.Speed, "Thrust prograde", "Reset prograde thrust", "Thrust retrograde")
									showDvLine(icons.normal, icons.maneuver, icons.antinormal, "normal", ui.Format.Speed, "Thrust normal", "Reset normal thrust", "Thrust antinormal")
									showDvLine(icons.radial_in, icons.maneuver, icons.radial_out, "radial", ui.Format.Speed, "Thrust radially in", "Reset radial thrust", "Thrust radially out")

									ui.separator()
									local t = Engine.SystemMapGetOrbitPlannerTime()
									ui.text(t and ui.Format.Datetime(t) or "now")
									local r = false
									r = timeButton(icons.time_backward_100x, "-10,000,000x",-10000000) or r
									r = timeButton(icons.time_backward_10x, "-100,000x", -100000) or r
									r = timeButton(icons.time_backward_1x, "-1,000x", -1000) or r
									r = timeButton(icons.time_center, "Now", nil) or r
									r = timeButton(icons.time_forward_1x, "1,000x", 1000) or r
									r = timeButton(icons.time_forward_10x, "100,000x", 100000) or r
									r = timeButton(icons.time_forward_100x, "10,000,000x", 10000000) or r
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
local function tabular(data)
	if data and #data > 0 then
		ui.columns(2, "Attributes", true)
		for _,item in pairs(data) do
			-- ui.setColumnOffset(1, width / 4)
			if item.value then
				ui.text(item.name)
				ui.nextColumn()
				ui.text(item.value)
				ui.nextColumn()
			end
		end
		ui.columns(1, "NoAttributes", false)
	end
end
local function showTargetInfoWindow(systemBody, body)
	local width = ui.screenWidth / 5
	ui.setNextWindowSize(Vector(width, (ui.screenHeight / 5) * 2), "Always")
	ui.setNextWindowPos(Vector(20, (ui.screenHeight / 5) * 2 + 20), "Always")
	ui.withStyleColors({["WindowBg"] = colors.lightBlackBackground}, function()
			ui.window("TargetInfoWindow", {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus"},
								function()
									local data
									if systemBody then
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
											{ name = lc.DAY_LENGTH .. lc.ROTATIONAL_PERIOD,
												value = rp > 0 and ui.Format.Duration(rp, 2) or nil },
											{ name = lc.RADIUS,
												value = radius },
											{ name = lc.SEMI_MAJOR_AXIS,
												value = semimajoraxis },
											{ name = lc.ORBITAL_PERIOD,
												value = op and op > 0 and ui.Format.Duration(op, 2) or nil }
										}
									elseif body and body:IsShip() then
										local pos = Engine.SystemMapProject(body:GetPosition())
										pos.x = pos.x / 800.0 * 1920.0
										pos.y = pos.y / 600.0 * 1200.0
										ui.addIcon(pos, icons.maneuver, colors.white, Vector(32,32), ui.anchor.center, ui.anchor.center, "TEST")
										local name = body.label
										data = {{ name = lc.NAME_OBJECT,
															value = name },
										}
										-- TODO: the advanced target scanner should add additional data here,
										-- but we really do not want to hardcode that here. there should be
										-- some kind of hook that the target scanner can hook into to display
										-- more info here.
										-- This is what should be inserted:
										table.insert(data, { name = "Ship Type",
																				 value = body:GetShipType()
										})
										local hd = body:GetEquip("engine", 1)
										table.insert(data, { name = "Hyperdrive",
																				 value = hd and hd:GetName() or lc.NO_HYPERDRIVE
										})
										table.insert(data, { name = lc.MASS,
																				 value = ui.Format.MassT(body:GetStats().staticMass)
										})
										table.insert(data, { name = lc.CARGO,
																				 value = ui.Format.MassT(body:GetStats().usedCargo)
										})
									else
										data = {}
									end
									tabular(data)
			end)
	end)
end
local function displaySystemViewUI()
	player = Game.player
	local current_view = Game.CurrentView()

	if current_view == "system" and not Game.InHyperspace() then
		ui.withFont(ui.fonts.pionillium.medium.name, ui.fonts.pionillium.medium.size, function()
									showOrbitPlannerWindow()
									showTargetInfoWindow(Engine.SystemMapSelectedObject(), player:GetNavTarget())
		end)
	end
end

ui.registerModule("game", displaySystemViewUI)

return {}
