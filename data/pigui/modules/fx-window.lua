local Engine = import('Engine')
local Game = import('Game')
local ui = import('pigui/pigui.lua')
local Vector = import('Vector')
local Color = import('Color')
local Lang = import("Lang")
local lc = Lang.GetResource("core");
local lui = Lang.GetResource("ui-core");
local utils = import("utils")
local Event = import("Event")

local player = nil
local colors = ui.theme.colors
local icons = ui.theme.icons

local mainButtonSize = Vector(32,32)
local mainButtonFramePadding = 3
local function mainMenuButton(icon, selected, tooltip, color)
	if color == nil then
		color = colors.white
	end
	return ui.coloredSelectedIconButton(icon, mainButtonSize, selected, mainButtonFramePadding, colors.buttonBlue, color, tooltip)
end

local currentView = "internal"

local function button_hyperspace()
	local disabled = false
	local shown = true
	local legal = player:IsHyperjumpAllowed()
	local abort = false
	local targetpath = player:GetHyperspaceTarget()
	local target = targetpath and targetpath:GetStarSystem()
	if player:CanHyperjumpTo(targetpath) then
		if player:IsDocked() or player:IsLanded() then
			disabled = true
		elseif player:IsHyperspaceActive() then
			abort = true
		end
	else
		shown = false
	end


	if shown then
		ui.sameLine()
		if disabled then
			mainMenuButton(icons.hyperspace, false, "hyperspace", colors.grey)
		else
			if mainMenuButton(legal and icons.hyperspace or icons.hyperspace_off, false, "hyperspace") or ui.isKeyReleased(ui.keys.f7)  then
				if player:IsHyperspaceActive() then
					player:AbortHyperjump()
				else
					player:HyperjumpTo(player:GetHyperspaceTarget())
				end
			end
		end
	end
end

local next_cam_type = { ["internal"] = "external", ["external"] = "sidereal", ["sidereal"] = "internal" }
local function button_world(current_view)
	ui.sameLine()
	if current_view ~= "world" then
		if mainMenuButton(icons.view_internal, false, "world view") or ui.isKeyReleased(ui.keys.f1) then
			Game.SetView("world")
		end
	else
		local camtype = Game.GetWorldCamType()
		if mainMenuButton(icons["view_" .. camtype], true, camtype .. " view") or ui.isKeyReleased(ui.keys.f1) then
			Game.SetWorldCamType(next_cam_type[camtype])
		end
	end
end

local current_map_view = "sector"
local function buttons_map(current_view)
	local onmap = current_view == "sector" or current_view == "system" or current_view == "system_info" or current_view == "galaxy"

	ui.sameLine()
	if mainMenuButton(icons.sector_map, current_view == "sector", "sector map") or (onmap and ui.isKeyReleased(ui.keys.f5)) then
		if current_view == "sector" then
			Game.SetView("world")
		else
			Game.SetView("sector")
			current_map_view = "sector"
		end
	end

	ui.sameLine()
	if mainMenuButton(icons.system_map, current_view == "system", "system map") or (onmap and ui.isKeyReleased(ui.keys.f6)) then
		if current_view == "system" then
			Game.SetView("world")
		else
			Game.SetView("system")
			current_map_view = "system"
		end
	end

	ui.sameLine()
	if mainMenuButton(icons.system_overview, current_view == "system_info", "system overview") or (onmap and ui.isKeyReleased(ui.keys.f7)) then
		if current_view == "system_info" then
			ui.systemInfoViewNextPage()
		else
			Game.SetView("system_info")
			current_map_view = "system_info"
		end
	end

	ui.sameLine()
	if mainMenuButton(icons.galaxy_map, current_view == "galaxy", "galaxy map") or (onmap and ui.isKeyReleased(ui.keys.f8)) then
		if current_view == "galaxy" then
			Game.SetView("world")
		else
			Game.SetView("galaxy")
			current_map_view = "galaxy"
		end
	end
	if ui.isKeyReleased(ui.keys.f2) then
		if onmap then
			Game.SetView("world")
		else
			Game.SetView(current_map_view)
		end
	end
end

local function button_info(current_view)
	ui.sameLine()
	if (mainMenuButton(icons.personal_info, current_view == "info", "personal info") or ui.isKeyReleased(ui.keys.f3)) then
		if current_view ~= "info" then
			Game.SetView("info")
		else
			Game.SetView("world")
		end
	end
end

local function button_comms(current_view)
	ui.sameLine()
	if mainMenuButton(icons.comms, current_view == "space_station", "comms") or ui.isKeyReleased(ui.keys.f4) then
		if player:IsDocked() then
			if current_view == "space_station" then
				Game.SetView("world")
			else
				Game.SetView("space_station")
			end
		else
			Game.ToggleTargetActions()
		end
	end
end

local function button_undock()
	if player:IsLanded() then
		ui.sameLine()
		if mainMenuButton(icons.autopilot_blastoff, false, "blastoff") or ui.isKeyReleased(ui.keys.f5) then
			player:BlastOff()
		end
	elseif player:IsDocked() then
		ui.sameLine()
		if mainMenuButton(icons.autopilot_undock, false, "undock") or ui.isKeyReleased(ui.keys.f5) then
			player:Undock()
		end
	end
end

local function button_wheelstate()
	local wheelstate = player:GetWheelState() -- 0.0 is up, 1.0 is down
	if wheelstate == 0.0 then -- gear is up
		ui.sameLine()
		if mainMenuButton(icons.landing_gear_down, false, "landing gear is up") or ui.isKeyReleased(ui.keys.f6) then
			player:ToggleWheelState()
		end
	elseif wheelstate == 1.0 then -- gear is down
		ui.sameLine()
		if mainMenuButton(icons.landing_gear_up, false, "landing gear is down") or ui.isKeyReleased(ui.keys.f6) then
			player:ToggleWheelState()
		end
	else
		ui.sameLine()
		mainMenuButton(icons.landing_gear_up, false, "landing gear is moving", colors.grey)
	end
end

local function button_rotation_damping()
	local rotation_damping = player:GetRotationDamping()
	if rotation_damping then
		ui.sameLine()
		if mainMenuButton(icons.rotation_damping_on, false, "rotation damping is on") then
			player:ToggleRotationDamping()
		end
	else
		ui.sameLine()
		if mainMenuButton(icons.rotation_damping_off, false, "rotation damping is off") then
			player:ToggleRotationDamping()
		end
	end
end

local current_mfd = "scanner"
local function button_mfd()
	ui.sameLine()
	if current_mfd == "scanner" then
		if mainMenuButton(icons.scanner, false, "scanner") or ui.isKeyReleased(ui.keys.f9) then
			Game.ChangeMFD("equipment")
			current_mfd = "equipment"
		end
	else
		if mainMenuButton(icons.repairs, false, "equipment") or ui.isKeyReleased(ui.keys.f9) then
			Game.ChangeMFD("scanner")
			current_mfd = "scanner"
		end
	end
end

local flightstate_info = {
	["CONTROL_MANUAL"] = { icon = icons.autopilot_manual, tooltip = "manual control" },
	-- "CONTROL_AUTOPILOT" = nil
	["CONTROL_FIXSPEED"] = { icon = icons.autopilot_set_speed, tooltip = "set speed" },
	["CONTROL_FIXHEADING_FORWARD"] = { icon = icons.prograde, tooltip = "fix heading prograde" },
	["CONTROL_FIXHEADING_BACKWARD"] = { icon = icons.retrograde , tooltip = "fix heading retrograde" },
	["CONTROL_FIXHEADING_NORMAL"] = { icon = icons.normal, tooltip = "fix heading normal" },
	["CONTROL_FIXHEADING_ANTINORMAL"] = { icon = icons.antinormal, tooltip = "fix heading antinormal" },
	["CONTROL_FIXHEADING_RADIALLY_INWARD"] = { icon = icons.radial_in, tooltip = "fix heading radially inward" },
	["CONTROL_FIXHEADING_RADIALLY_OUTWARD"] = { icon = icons.radial_out, tooltip = "fix heading radially outward" },
	["CONTROL_FIXHEADING_KILLROT"] = { icon = icons.rotation_damping_on , tooltip = "kill rotation" },
}

local aicommand_info = {
	["CMD_DOCK"] = { icon = icons.autopilot_dock, tooltip = "autopilot dock" },
	["CMD_FLYTO"] = { icon = icons.autopilot_fly_to, tooltip = "autopilot fly to" },
	["CMD_FLYAROUND"] = { icon = icons.autopilot_medium_orbit, tooltip = "autopilot orbit" },
}

local function button_flight_control()
	ui.sameLine()
	local flightstate = player:GetFlightState()
	local flightcontrolstate = player:GetFlightControlState()
	local fcsi = flightstate_info[flightcontrolstate]
	local icon = icons.autopilot_manual
	local tooltip = "manual control"
	if fcsi then
		icon = fcsi.icon
		tooltip = fcsi.tooltip
	end
	if flightcontrolstate == "CONTROL_AUTOPILOT" then
		local cmd = player:GetCurrentAICommand()
		local ci = aicommand_info[cmd]
		icon = icons.backward
		tooltip = "UNKNOWN AI COMMAND"
		if ci then
			icon = ci.icon
			tooltip = ci.tooltip
		end
	elseif flightcontrolstate == "CONTROL_FIXSPEED" then
		local distance, unit = ui.Format.Speed(player:GetSetSpeed())
		tooltip = tooltip .. " " .. distance .. unit
  end
	if mainMenuButton(icon, false, tooltip) or (flightstate == "FLYING" and ui.isKeyReleased(ui.keys.f5)) then
		Game.ChangeFlightState()
	end
end


local function displayFxWindow()
	player = Game.player
	local current_view = Game.CurrentView()
	ui.setNextWindowPos(Vector(ui.screenWidth/3, 0) , "FirstUseEver")
	ui.window("Fx", {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus"},
						function()
							button_world(current_view)

							button_info(current_view)

							button_comms(current_view)

							buttons_map(current_view)

							if current_view == "world" then
								button_hyperspace()

								button_undock()

								button_wheelstate()

								button_rotation_damping()

								button_mfd()

								button_flight_control()

								if ui.isKeyReleased(ui.keys.f8) then
									Game.ToggleLowThrustPowerOptions()
								end

							end -- current_view == "world"
	end)
end

ui.registerModule("game", displayFxWindow)
return {}
