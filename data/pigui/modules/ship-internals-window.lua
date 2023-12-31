-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Game = require 'Game'

local Lang = require 'Lang'
local lc = Lang.GetResource("core");
local lui = Lang.GetResource("ui-core");

local ui = require 'pigui'
local Vector2 = _G.Vector2

local player = nil
local colors = ui.theme.colors
local icons = ui.theme.icons

local mainButtonSize = ui.theme.styles.MainButtonSize
local mainButtonFramePadding = ui.theme.styles.MainButtonPadding

local show_thrust_slider = false

local gauge_bg = colors.grey
local gauge_fg = colors.lightGrey
local function button_lowThrustPower()
	local thrust = player:GetLowThrustPower()
	local winpos = ui.getWindowPos()
	local pos = ui.getCursorPos()
	if ui.lowThrustButton("lowthrust", mainButtonSize, thrust * 100, colors.transparent, mainButtonFramePadding, gauge_fg, gauge_bg)  then
		show_thrust_slider = not show_thrust_slider
	end

	if show_thrust_slider then
		local p = winpos + pos - Vector2(8,100+9)
		ui.setNextWindowPos(p,'Always')

		ui.window("ThrustSliderWindow", {"NoTitleBar", "NoResize"},
			function()
				ui.withStyleColors({["SliderGrab"] =colors.white, ["SliderGrabActive"]=colors.buttonBlue},function()
					local new_thrust = ui.vSliderInt('###ThrustLowPowerSlider',Vector2(mainButtonSize.x + 1 + 2 * mainButtonFramePadding,100), thrust*100,0,100)
					player:SetLowThrustPower(new_thrust/100)
				end)
			end)
	end
	if ui.isItemHovered() then
		ui.setTooltip(lc.SELECT_LOW_THRUST_POWER_LEVEL)
		local wheel = ui.getMouseWheel()
		if wheel ~= 0 then
			local new_thrust = thrust + (wheel / 50)
			if new_thrust > 1 then new_thrust = 1 end
			if new_thrust < 0 then new_thrust = 0 end
			player:SetLowThrustPower(new_thrust)
		end
	end
end

local function button_thrustIndicator(thrust_widget_size)
	local vel = Engine.WorldSpaceToShipSpace(player:GetVelocity())
	vel = vel / math.max(vel:length(), 10) -- minimum of 10m/s
	local thrust = player:GetThrusterState()
	ui.thrustIndicator("foo", thrust_widget_size, thrust, vel, colors.transparent, mainButtonFramePadding, colors.gaugeVelocityLight, colors.gaugeVelocityDark, colors.gaugeThrustLight, colors.gaugeThrustDark)
	if ui.isItemHovered() then
		ui.setTooltip(lui.HUD_THRUST_INDICATOR)
	end
end

local function button_wheelstate()
	local wheelstate = player:GetWheelState() -- 0.0 is up, 1.0 is down
	local locked = player:GetFlightControlState() == "CONTROL_AUTOPILOT"
	if locked and wheelstate == 0.0 then
		ui.mainMenuButton(icons.landing_gear_down, lc.AUTOPILOT_CONTROL, ui.theme.buttonColors.disabled)
	elseif wheelstate == 0.0 then -- gear is up
		if ui.mainMenuButton(icons.landing_gear_down, lui.HUD_BUTTON_LANDING_GEAR_IS_UP) then
			player:ToggleWheelState()
		end
	elseif wheelstate == 1.0 then -- gear is down
		if ui.mainMenuButton(icons.landing_gear_up, lui.HUD_BUTTON_LANDING_GEAR_IS_DOWN) then
			player:ToggleWheelState()
		end
	else
		ui.mainMenuButton(icons.landing_gear_up, lui.HUD_BUTTON_LANDING_GEAR_IS_MOVING, ui.theme.buttonColors.disabled)
	end
end

local function button_rotation_damping()
	local rotation_damping = player:GetRotationDamping()
	if rotation_damping then
		if ui.mainMenuButton(icons.rotation_damping_on, lui.HUD_BUTTON_ROTATION_DAMPING_IS_ON) then
			player:ToggleRotationDamping()
		end
	else
		if ui.mainMenuButton(icons.rotation_damping_off, lui.HUD_BUTTON_ROTATION_DAMPING_IS_OFF) then
			player:ToggleRotationDamping()
		end
	end
end

local windowFlags = ui.WindowFlags {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus", "NoSavedSettings", "AlwaysAutoResize"}
local function displayShipFunctionWindow()
	if ui.optionsWindow.isOpen then return end
	player = Game.player
	local current_view = Game.CurrentView()
	local buttons = 3
	local thrust_widget_size = Vector2(mainButtonSize.x * 3, mainButtonSize.y * 2)
	assert(thrust_widget_size.y >= mainButtonSize.y)
	local window_width = ui.getWindowPadding().x * 2 + (mainButtonSize.x + 2 * mainButtonFramePadding + ui.getItemSpacing().x) * buttons + thrust_widget_size.x + mainButtonFramePadding * 2
	local window_height = thrust_widget_size.y + mainButtonFramePadding * 2 + ui.getWindowPadding().y * 2
	local window_posx = ui.screenWidth/2 - ui.reticuleCircleRadius - window_width + 12 -- manual move a little closer to the center
	local window_posy = ui.screenHeight - window_height
	ui.setNextWindowPos(Vector2(window_posx, window_posy), "Always")
	ui.window("ShipFunctions", windowFlags, function()
		if current_view == "world" then
			local shift = Vector2(0.0, thrust_widget_size.y - mainButtonSize.y)
			ui.addCursorPos(shift)
			button_wheelstate()
			ui.sameLine()
			button_rotation_damping()
			ui.sameLine()
			button_lowThrustPower()
			ui.sameLine()
			ui.addCursorPos(-shift)
			button_thrustIndicator(thrust_widget_size)
			if ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f8) then
				show_thrust_slider = not show_thrust_slider
			end
		end -- current_view == "world"
	end)
end

ui.registerModule("game", { id = "ship-internals-window", draw = displayShipFunctionWindow })

return {}
