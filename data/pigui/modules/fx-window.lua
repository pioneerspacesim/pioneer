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
			mainMenuButton(icons.hyperspace, false, lui.HUD_BUTTON_HYPERDRIVE_DISABLED, colors.grey)
		else
			local icon = icons.hyperspace_off
			local tooltip = lui.HUD_BUTTON_INITIATE_ILLEGAL_HYPERJUMP
			if legal then
				icon = icons.hyperspace
				tooltip = lui.HUD_BUTTON_INITIATE_HYPERJUMP
			end
			if mainMenuButton(icon, false, tooltip) or ui.isKeyReleased(ui.keys.f7)  then
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
local cam_tooltip = { ["internal"] = lui.HUD_BUTTON_INTERNAL_VIEW, ["external"] = lui.HUD_BUTTON_EXTERNAL_VIEW, ["sidereal"] = lui.HUD_BUTTON_SIDEREAL_VIEW }
local function button_world(current_view)
	ui.sameLine()
	if current_view ~= "world" then
		if mainMenuButton(icons.view_internal, false, lui.HUD_BUTTON_SWITCH_TO_WORLD_VIEW) or (ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f1)) then
			Game.SetView("world")
		end
	else
		local camtype = Game.GetWorldCamType()
		if mainMenuButton(icons["view_" .. camtype], true, cam_tooltip[camtype]) or (ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f1)) then
			Game.SetWorldCamType(next_cam_type[camtype])
		end
	end
end

local current_map_view = "sector"
local function buttons_map(current_view)
	local onmap = current_view == "sector" or current_view == "system" or current_view == "system_info" or current_view == "galaxy"

	ui.sameLine()
	local active = current_view == "sector"
	if mainMenuButton(icons.sector_map, active, active and lui.HUD_BUTTON_SWITCH_TO_WORLD_VIEW or lui.HUD_BUTTON_SWITCH_TO_SECTOR_MAP) or (onmap and ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f5)) then
		if active then
			Game.SetView("world")
		else
			Game.SetView("sector")
			current_map_view = "sector"
		end
	end

	ui.sameLine()
	active = current_view == "system"
	if mainMenuButton(icons.system_map, active, active and lui.HUD_BUTTON_SWITCH_TO_WORLD_VIEW or lui.HUD_BUTTON_SWITCH_TO_SYSTEM_MAP) or (onmap and ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f6)) then
		if active then
			Game.SetView("world")
		else
			Game.SetView("system")
			current_map_view = "system"
		end
	end

	ui.sameLine()
	active = current_view == "system_info"
	if mainMenuButton(icons.system_overview, active, active and lui.HUD_BUTTON_SWITCH_TO_WORLD_VIEW or lui.HUD_BUTTON_SWITCH_TO_SYSTEM_OVERVIEW) or (onmap and ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f7)) then
		if active then
			ui.systemInfoViewNextPage()
		else
			Game.SetView("system_info")
			current_map_view = "system_info"
		end
	end

	ui.sameLine()
	active = current_view == "galaxy"
	if mainMenuButton(icons.galaxy_map, active, active and lui.HUD_BUTTON_SWITCH_TO_WORLD_VIEW or lui.HUD_BUTTON_SWITCH_TO_GALAXY_MAP) or (onmap and ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f8)) then
		if active then
			Game.SetView("world")
		else
			Game.SetView("galaxy")
			current_map_view = "galaxy"
		end
	end
	if ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f2) then
		if onmap then
			Game.SetView("world")
		else
			Game.SetView(current_map_view)
		end
	end
end

local function button_info(current_view)
	ui.sameLine()
	if (mainMenuButton(icons.personal_info, current_view == "info", lui.HUD_BUTTON_SHOW_PERSONAL_INFO) or (ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f3))) then
		if current_view ~= "info" then
			Game.SetView("info")
		else
			Game.SetView("world")
		end
	end
end

local function button_comms(current_view)
	ui.sameLine()
	if mainMenuButton(icons.comms, current_view == "space_station", lui.HUD_BUTTON_SHOW_COMMS) or (ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f4)) then
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
		if mainMenuButton(icons.autopilot_blastoff, false, lui.HUD_BUTTON_BLASTOFF) or (ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f5)) then
			player:BlastOff()
		end
	elseif player:IsDocked() then
		ui.sameLine()
		if mainMenuButton(icons.autopilot_undock, false, lui.HUD_BUTTON_UNDOCK) or (ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f5)) then
			player:Undock()
		end
	end
end

local function button_wheelstate()
	local wheelstate = player:GetWheelState() -- 0.0 is up, 1.0 is down
	if wheelstate == 0.0 then -- gear is up
		ui.sameLine()
		if mainMenuButton(icons.landing_gear_down, false, lui.HUD_BUTTON_LANDING_GEAR_IS_UP) or (ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f6)) then
			player:ToggleWheelState()
		end
	elseif wheelstate == 1.0 then -- gear is down
		ui.sameLine()
		if mainMenuButton(icons.landing_gear_up, false, lui.HUD_BUTTON_LANDING_GEAR_IS_DOWN) or (ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f6)) then
			player:ToggleWheelState()
		end
	else
		ui.sameLine()
		mainMenuButton(icons.landing_gear_up, false, lui.HUD_BUTTON_LANDING_GEAR_IS_MOVING, colors.grey)
	end
end

local function button_rotation_damping()
	local rotation_damping = player:GetRotationDamping()
	if rotation_damping then
		ui.sameLine()
		if mainMenuButton(icons.rotation_damping_on, false, lui.HUD_BUTTON_ROTATION_DAMPING_IS_ON) then
			player:ToggleRotationDamping()
		end
	else
		ui.sameLine()
		if mainMenuButton(icons.rotation_damping_off, false, lui.HUD_BUTTON_ROTATION_DAMPING_IS_OFF) then
			player:ToggleRotationDamping()
		end
	end
end

local current_mfd = "scanner"
local last_mfd = "scanner"
local function button_mfd()
	ui.sameLine()
	if current_mfd == "scanner" or current_mfd == "radar" then
		if mainMenuButton(icons.scanner, false, lui.HUD_BUTTON_SCANNER) or (ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f9)) then
			last_mfd = current_mfd
			Event.Queue('changeMFD', 'equipment')
		end
	else
		if mainMenuButton(icons.repairs, false, lui.HUD_BUTTON_EQUIPMENT) or (ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f9)) then
			Event.Queue('changeMFD', last_mfd)
		end
	end
end

Event.Register('onChangeMFD', function(selected)
								 current_mfd = selected
end)

local flightstate_info = {
	["CONTROL_MANUAL"] = { icon = icons.autopilot_manual, tooltip = lui.HUD_BUTTON_MANUAL_CONTROL },
	-- "CONTROL_AUTOPILOT" = nil
	["CONTROL_FIXSPEED"] = { icon = icons.autopilot_set_speed, tooltip = lui.HUD_BUTTON_SET_SPEED },
	["CONTROL_FIXHEADING_FORWARD"] = { icon = icons.prograde, tooltip = lui.HUD_BUTTON_FIX_PROGRADE },
	["CONTROL_FIXHEADING_BACKWARD"] = { icon = icons.retrograde , tooltip = lui.HUD_BUTTON_FIX_RETROGRADE },
	["CONTROL_FIXHEADING_NORMAL"] = { icon = icons.normal, tooltip = lui.HUD_BUTTON_FIX_NORMAL },
	["CONTROL_FIXHEADING_ANTINORMAL"] = { icon = icons.antinormal, tooltip = lui.HUD_BUTTON_FIX_ANTINORMAL },
	["CONTROL_FIXHEADING_RADIALLY_INWARD"] = { icon = icons.radial_in, tooltip = lui.HUD_BUTTON_FIX_RADIAL_IN },
	["CONTROL_FIXHEADING_RADIALLY_OUTWARD"] = { icon = icons.radial_out, tooltip = lui.HUD_BUTTON_FIX_RADIAL_OUT },
	["CONTROL_FIXHEADING_KILLROT"] = { icon = icons.rotation_damping_on , tooltip = lui.HUD_BUTTON_KILL_ROTATION }
}

local aicommand_info = {
	["CMD_DOCK"] = { icon = icons.autopilot_dock, tooltip = lui.HUD_BUTTON_AUTOPILOT_DOCKING },
	["CMD_FLYTO"] = { icon = icons.autopilot_fly_to, tooltip = lui.HUD_BUTTON_AUTOPILOT_FLYING_TO_TARGET },
	["CMD_FLYAROUND"] = { icon = icons.autopilot_medium_orbit, tooltip = lui.HUD_BUTTON_AUTOPILOT_ENTERING_ORBIT },
}

local function button_flight_control()
	ui.sameLine()
	local flightstate = player:GetFlightState()
	local flightcontrolstate = player:GetFlightControlState()
	local fcsi = flightstate_info[flightcontrolstate]
	local icon = icons.autopilot_manual
	local tooltip = lui.HUD_BUTTON_MANUAL_CONTROL
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
		local speed = player:GetSetSpeed()
		if speed then
			local distance, unit = ui.Format.Speed(speed)
			tooltip = tooltip .. " " .. distance .. unit
		end
  end
	if mainMenuButton(icon, false, tooltip) or (flightstate == "FLYING" and ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f5)) then
		Game.ChangeFlightState()
	end
end


local function displayFxWindow()
	player = Game.player
	local current_view = Game.CurrentView()
	ui.setNextWindowPos(Vector(ui.screenWidth/2 - (mainButtonSize.x + 2 * mainButtonFramePadding) * 6, 0) , "FirstUseEver")
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

								if ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f8) then
									Game.ToggleLowThrustPowerOptions()
								end

							end -- current_view == "world"
	end)
end

ui.registerModule("game", displayFxWindow)

return {}
