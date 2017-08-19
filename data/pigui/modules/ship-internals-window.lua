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
local itemSpacingX = 8 -- imgui default

local function mainMenuButton(icon, selected, tooltip, color)
	if color == nil then
		color = colors.white
	end
	return ui.coloredSelectedIconButton(icon, mainButtonSize, selected, mainButtonFramePadding, colors.buttonBlue, color, tooltip)
end
local gauge_bg = colors.grey
local gauge_fg = colors.lightGrey
local function button_lowThrustPower()
	local thrust = player:GetLowThrustPower()
	ui.withStyleColors({
			["Button"] = colors.buttonBlue:shade(0.6),
			["ButtonHovered"] = colors.buttonBlue:shade(0.4),
			["ButtonActive"] = colors.buttonBlue:shade(0.2)},
		function ()
			if ui.lowThrustButton("lowthrust", mainButtonSize, thrust * 100, colors.lightBlueBackground, mainButtonFramePadding, gauge_fg, gauge_bg) then
				Game:ToggleLowThrustPowerOptions()
			end
	end)
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

local function button_thrustIndicator()
	local size = Vector(3, 2)
	-- local size = Vector(1.5, 1)
	ui.withStyleColors({
			["Button"] = colors.buttonBlue:shade(0.6),
			["ButtonHovered"] = colors.buttonBlue:shade(0.4),
			["ButtonActive"] = colors.buttonBlue:shade(0.2)},
		function ()
			ui.sameLine()
			local vel = Engine.WorldSpaceToShipSpace(player:GetVelocity())
			vel = vel / math.max(vel:magnitude(), 10) -- minimum of 10m/s
			local thrust = player:GetThrusterState()
			local v = ui.getCursorPos();
			ui.setCursorPos(Vector(v.x, 20));
			ui.thrustIndicator("foo", mainButtonSize * size, thrust, vel, colors.lightBlueBackground, mainButtonFramePadding, colors.gaugeVelocityLight, colors.gaugeVelocityDark, colors.gaugeThrustLight, colors.gaugeThrustDark)
			if ui.isItemHovered() then
				ui.setTooltip(lui.HUD_THRUST_INDICATOR)
			end
	end)
	-- if ui.isItemHovered() then
	-- 	ui.setTooltip(lc.SELECT_LOW_THRUST_POWER_LEVEL)
	-- 	local wheel = ui.getMouseWheel()
	-- 	if wheel ~= 0 then
	-- 		local new_thrust = thrust + (wheel / 50)
	-- 		if new_thrust > 1 then new_thrust = 1 end
	-- 		if new_thrust < 0 then new_thrust = 0 end
	-- 		player:SetLowThrustPower(new_thrust)
	-- 	end
	-- end
end

local function button_wheelstate()
	local wheelstate = player:GetWheelState() -- 0.0 is up, 1.0 is down
	if wheelstate == 0.0 then -- gear is up
		if mainMenuButton(icons.landing_gear_down, false, lui.HUD_BUTTON_LANDING_GEAR_IS_UP) or (ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f6)) then
			player:ToggleWheelState()
		end
	elseif wheelstate == 1.0 then -- gear is down
		if mainMenuButton(icons.landing_gear_up, false, lui.HUD_BUTTON_LANDING_GEAR_IS_DOWN) or (ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f6)) then
			player:ToggleWheelState()
		end
	else
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

local function displayShipFunctionWindow()
	player = Game.player
	local current_view = Game.CurrentView()
	local buttons = 6
	ui.setNextWindowPos(Vector(ui.screenWidth/2 - ui.reticuleCircleRadius - (mainButtonSize.x + 2 * mainButtonFramePadding) * buttons, ui.screenHeight - mainButtonSize.y * 3 - 8) , "Always")
	ui.window("ShipFunctions", {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus"},
						function()
							if current_view == "world" then
								local v = ui.getCursorPos()
								ui.setCursorPos(Vector(0, v.y + 1.33 * mainButtonSize.y));
								button_wheelstate()
								button_rotation_damping()
								ui.sameLine()
								button_lowThrustPower()
								button_thrustIndicator()
								if ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f8) then
									Game.ToggleLowThrustPowerOptions()
								end
							end -- current_view == "world"
	end)
end

ui.registerModule("game", displayShipFunctionWindow)

return {}
