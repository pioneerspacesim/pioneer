-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local Event = require 'Event'
local bindManager = require 'bind-manager'
local AutoSave    = require 'modules.AutoSave.AutoSave'

local Lang = require 'Lang'
local lui = Lang.GetResource("ui-core");

local ui = require 'pigui'
local Vector2 = _G.Vector2
local lc = Lang.GetResource("core");
local colors = ui.theme.colors

local player = nil
local icons = ui.theme.icons

local mainButtonSize = ui.theme.styles.MainButtonSize
local smallButtonSize = ui.theme.styles.SmallButtonSize
local mainButtonFramePadding = ui.theme.styles.MainButtonPadding
local semi_transp = ui.theme.buttonColors.transparent

local bindings = {
	limiter = bindManager.registerAction('BindToggleSpeedLimiter'),
}

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

	if not shown then return false end

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
	return true
end

local function button_undock()
	if player:IsLanded() then
		if ui.mainMenuButton(icons.autopilot_blastoff, lui.HUD_BUTTON_BLASTOFF) or (ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f5)) then
			Game.SetTimeAcceleration("1x")
			AutoSave.Save()
			player:BlastOff()
		end
		return true
	elseif player:IsDocked() then
		if ui.mainMenuButton(icons.autopilot_undock, lui.HUD_BUTTON_UNDOCK) or (ui.noModifierHeld() and ui.isKeyReleased(ui.keys.f5)) then
			Game.SetTimeAcceleration("1x")
			AutoSave.Save()
			player:Undock()
		end
		return true
	end
	return false
end

local speed_limiter = (function()
	-- 'const'
	local ANIM_LENGTH = 15 -- frames
	local MAX_SPEED_LIMIT = 300000 -- km/s
	-- init at load / reset
	local speed_limit -- m/s, nil means limiter is off
	local anim_active
	local anim_width
	local init_frame
	-- init at first frame
	local max_width
	local min_width
	local anim_step

	local function runOpenLimiter()
		anim_width = min_width
		anim_active = true
	end

	local function runCloseLimiter()
		anim_width = max_width
		speed_limit = nil
		anim_active = true
	end

	local function reset()
		anim_active = false
		speed_limit = nil
		anim_width = 0
		init_frame = true -- to run first frame init
	end

	local function show()
		-- first frame init
		-- must be inside the ImGui frame to know the correct sizes
		if init_frame then
			local arrow_size = ui.getFrameHeight() -- square icon
			max_width = ui.calcTextSize(string.format("%.2f " .. lc.UNIT_KILOMETERS_PER_SECOND, MAX_SPEED_LIMIT)).x + ui.getItemSpacing().x + arrow_size * 2
			min_width = ui.getItemSpacing().x
			anim_step = (max_width - min_width) / ANIM_LENGTH
			init_frame = false
		end

		local toggle_limiter = not speed_limit ~= not player:GetSpeedLimit() -- convert to bool via not
		speed_limit = player:GetSpeedLimit()

		-- variant == nil means default variant
		local variant = not (speed_limit or anim_active) and ui.theme.buttonColors.disabled
		-- don't show tooltip during animation
		local tooltip = variant and bindings.limiter.tooltip or anim_active and "" or lui.TURN_OFF
		if ui.mainMenuButton(icons.speed_limiter, tooltip .. "##speed_limiter_toggle", variant) then
				bindings.limiter.action:OnPress()
		end

		if toggle_limiter then
			if speed_limit then
				runOpenLimiter()
			else
				runCloseLimiter()
			end
		end

		-- animation open / close
		local full_width = max_width
		if anim_active then
			if speed_limit then
				if anim_width < max_width then
					anim_width = anim_width + anim_step
					full_width = anim_width
				else
					anim_active = false
				end
			else
				if anim_width > min_width then
					anim_width = anim_width - anim_step
					full_width = anim_width
				else
					anim_active = false
				end
			end
		end

		if not speed_limit and not anim_active then return end

		ui.sameLine()

		-- limiter display

		local cp = ui.getCursorScreenPos() + Vector2(0 - ui.getItemSpacing().x, 0)
		local full_height = mainButtonSize.y + 2 * mainButtonFramePadding
		local txt_shift = full_height / 2 - ui.getFrameHeight() / 2
		ui.addRectFilled(cp, cp + Vector2(full_width, full_height), colors.Button, 0, 0)
		if anim_active then
			ui.addRectFilled(cp + Vector2(0, txt_shift), cp + Vector2(full_width - ui.getItemSpacing().x, full_height - txt_shift), colors.lightBlackBackground, 0, 0)
			ui.addCursorPos(Vector2(full_width, 0))
		else
			-- because of this, the window became larger by <txt_shift> pixels from
			-- the bottom (but this is invisible and it sticks out of the bottom edge
			-- of the screen)
			ui.addCursorPos(Vector2(-ui.getItemSpacing().x, txt_shift))
			-- fixed width for drag
			local drag_width = full_width - ui.getItemSpacing().x
			ui.pushItemWidth(drag_width)
			-- arrows
			if not ui.wantTextInput() then
				local size = ui.getFrameHeight()
				ui.addIcon(ui.getCursorScreenPos(), icons.time_backward_1x, colors.white, Vector2(size, size), ui.anchor.left, ui.anchor.top)
				ui.addIcon(ui.getCursorScreenPos() + Vector2(drag_width, 0), icons.time_forward_1x, colors.white, Vector2(size, size), ui.anchor.right, ui.anchor.top)
			end
			local step = math.max(0.01, speed_limit / 1000 / 500)
			local value, changed
			ui.withStyleColors( {["FrameBg"] = colors.lightBlackBackground}, function()
				value, changed = ui.dragFloat("##speed_limiter_drag", speed_limit / 1000, step, 0.0, MAX_SPEED_LIMIT, "%.2f " .. lc.UNIT_KILOMETERS_PER_SECOND)
			end)
			if ui.isItemHovered() then
				ui.setTooltip(lc.SET_MAXIMUM_SPEED_LIMIT)
			end
			-- mouse wheel
			if ui.isItemHovered() then
				local wheel = ui.getMouseWheel()
				if wheel ~= 0 then
					local delta = wheel
					changed = true
					value = speed_limit / 1000 + delta * step * 5
				end
			end
			ui.popItemWidth()

			-- apply value
			if changed then
				value = math.min(math.max(value, 0), MAX_SPEED_LIMIT) * 1000
				speed_limit = value
				player:SetSpeedLimit(value)
			end

			-- aux buttons
			local buttonSize = smallButtonSize + Vector2(mainButtonFramePadding * 2, mainButtonFramePadding * 2)
			local buttonVOffset = ui.getCursorPos().y - full_height - buttonSize.y - txt_shift
			ui.setCursorPos(Vector2(ui.getContentRegion().x - buttonSize.x * 3, buttonVOffset))

			ui.group(function()
				if ui.mainMenuButton(icons.frame_away, lui.SET_TO_ZERO .. "##speed_limiter_set_zero", semi_transp, smallButtonSize) then
					player:SetSpeedLimit(0)
				end
				ui.sameLine(0, 0)
				if ui.mainMenuButton(icons.manual_flight, lui.SET_TO_CURRENT_VELOCITY .. "##speed_limiter_set_current", semi_transp, smallButtonSize) then
					player:SetSpeedLimit(player:GetVelocity():length())
				end
				ui.sameLine(0, 0)
				if ui.mainMenuButton(icons.deltav, lui.SET_TO_45PERCENT_OF_DELTAV .. "##speed_limiter_set_45dv", semi_transp, smallButtonSize) then
					player:SetSpeedLimit(player:GetMaxDeltaV() * 0.45)
				end

			end)

		end
	end
	reset() -- reset vars on creation
	return { reset = reset, show = show }
end)()

local function displayAutoPilotWindow()
	if ui.optionsWindow.isOpen then return end
	player = Game.player
	local current_view = Game.CurrentView()
	local window_h = mainButtonSize.y + smallButtonSize.y + mainButtonFramePadding * 4 + ui.getWindowPadding().y * 2
	local shift = smallButtonSize.y + mainButtonFramePadding * 2
	local window_posx = ui.screenWidth/2 + ui.reticuleCircleRadius / 4 * 3
	local window_posy = ui.screenHeight - window_h
	ui.setNextWindowPos(Vector2(window_posx, window_posy) , "Always")
	ui.window("AutoPilot", {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus", "NoSavedSettings", "AlwaysAutoResize"},
		function()
			if current_view == "world" then
				ui.addCursorPos(Vector2(0, shift))
				if button_hyperspace() then ui.sameLine() end
				if button_undock() then ui.sameLine() end
				speed_limiter.show()
			end -- current_view == "world"
		end)
end

ui.registerModule("game", { id = "autopilot-window", draw = displayAutoPilotWindow, debugReload = function() package.reimport() end })
Event.Register("onGameStart", speed_limiter.reset)

return {}
