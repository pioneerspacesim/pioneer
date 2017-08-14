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

local mainButtonSize = Vector(32,32) * (ui.screenHeight / 1200)
local mainButtonFramePadding = 3

local function mainMenuButton(icon, selected, tooltip, color)
	if color == nil then
		color = colors.white
	end
	return ui.coloredSelectedIconButton(icon, mainButtonSize, selected, mainButtonFramePadding, colors.buttonBlue, color, tooltip)
end

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

local function displayAutoPilotWindow()
	player = Game.player
	local current_view = Game.CurrentView()
	local buttons = 3
	ui.setNextWindowSize(Vector(mainButtonSize.x * 5, mainButtonSize.y * 2), "Always")
	ui.setNextWindowPos(Vector(ui.screenWidth/2 + ui.reticuleCircleRadius / 4 * 3, ui.screenHeight - mainButtonSize.y * 1.5 - 8) , "Always")
	ui.window("AutoPilot", {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus"},
						function()
							if current_view == "world" then
								button_hyperspace()

								button_undock()

								button_flight_control()
							end -- current_view == "world"
	end)
end

ui.registerModule("game", displayAutoPilotWindow)

return {}
