-- Copyright © 2008-2022 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'

local Lang = require 'Lang'
local lui = Lang.GetResource("ui-core");

local ui = require 'pigui'
local Vector2 = _G.Vector2

local player = nil
local icons = ui.theme.icons

local mainButtonSize = ui.theme.styles.MainButtonSize
local mainButtonFramePadding = ui.theme.styles.MainButtonPadding

local function button_hyperspace()
	local disabled = false
	local shown = true
	local legal = player:IsHyperjumpAllowed()
	local targetpath = player:GetHyperspaceTarget()
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
		if disabled then
			ui.mainMenuButton(icons.hyperspace, lui.HUD_BUTTON_HYPERDRIVE_DISABLED, ui.theme.buttonColors.disabled)
		else
			local icon = icons.hyperspace_off
			local tooltip = lui.HUD_BUTTON_INITIATE_ILLEGAL_HYPERJUMP
			if legal then
				icon = icons.hyperspace
				tooltip = lui.HUD_BUTTON_INITIATE_HYPERJUMP
			end
			if ui.mainMenuButton(icon, tooltip) or ui.isKeyReleased(ui.keys.f7)  then
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
		if ui.mainMenuButton(icons.autopilot_blastoff, lui.HUD_BUTTON_BLASTOFF) or (ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f5)) then
			Game.SetTimeAcceleration("1x")
			player:BlastOff()
		end
	elseif player:IsDocked() then
		if ui.mainMenuButton(icons.autopilot_undock, lui.HUD_BUTTON_UNDOCK) or (ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f5)) then
			Game.SetTimeAcceleration("1x")
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
	["CMD_FORMATION"] = { icon = icons.autopilot_fly_to, tooltip = lui.HUD_BUTTON_AUTOPILOT_FLYING_TO_TARGET },
	["CMD_FLYAROUND"] = { icon = icons.autopilot_medium_orbit, tooltip = lui.HUD_BUTTON_AUTOPILOT_ENTERING_ORBIT },
}

local function button_flight_control()
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
			tooltip = tooltip .. " " .. ui.Format.Speed(speed)
		end
  end
	if ui.mainMenuButton(icon, tooltip) or (flightstate == "FLYING" and ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f5)) then
		local newState = "CONTROL_MANUAL"
		if ui.ctrlHeld() and flightcontrolstate == "CONTROL_FIXSPEED" then
			newState = "CONTROL_FIXHEADING_FORWARD"
		elseif flightcontrolstate == "CONTROL_MANUAL" then
			newState = "CONTROL_FIXSPEED"
		end
		Game.player:SetFlightControlState(newState)
		ui.playBoinkNoise()
	end
	if ui.isItemHovered() and flightcontrolstate == "CONTROL_FIXSPEED" then
		local wheel = ui.getMouseWheel()
		if wheel ~= 0 then
			local delta = wheel
			if ui.shiftHeld() then
				delta = delta * 10
			end
			Game.player:ChangeSetSpeed(delta)
		end
	end

end

local function displayAutoPilotWindow()
	if ui.optionsWindow.isOpen then return end
	player = Game.player
	local current_view = Game.CurrentView()
	local window_posx = ui.screenWidth/2 + ui.reticuleCircleRadius / 4 * 3
	local window_posy = ui.screenHeight - mainButtonSize.y - mainButtonFramePadding * 2 - ui.getWindowPadding().y * 2
	ui.setNextWindowPos(Vector2(window_posx, window_posy) , "Always")
	ui.window("AutoPilot", {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus", "NoSavedSettings", "AlwaysAutoResize"},
						function()
							if current_view == "world" then
								button_hyperspace()
								ui.sameLine()
								button_undock()
								ui.sameLine()
								button_flight_control()
							end -- current_view == "world"
	end)
end

ui.registerModule("game", { id = "autopilot-window", draw = displayAutoPilotWindow })

return {}
