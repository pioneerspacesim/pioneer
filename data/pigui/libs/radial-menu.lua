-- Copyright © 2008-2022 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt
local Lang = require 'Lang'
local Engine = require 'Engine'
local pigui = Engine.pigui
local Game = require 'Game'
local ui = require 'pigui.baseui'

local lc = Lang.GetResource("core")


local shouldShowRadialMenu = false
local radialMenuPos = Vector2(0,0)
local radialMenuSize = 10
local radialMenuIconSize = Vector2(radialMenuSize)
local radialMenuTarget = nil
local radialMenuMouseButton = 1
local radialMenuActions = {}
local radialMenuMousePos = nil

--
-- Function: ui.openRadialMenu
--
-- ui.openRadialMenu(target, mouse_button, size, actions)
--
-- Show a radial menu
--
--
-- Example:
--
-- >
--
-- Parameters:
--
--   target -
--   mouse_button -
--   size -
--   action - table
--
-- Returns:
--
--   nil
--
function ui.openRadialMenu(target, mouse_button, size, actions)
	ui.openPopup("##radialmenupopup")
	shouldShowRadialMenu = true
	radialMenuTarget = target
	radialMenuPos = ui.getMousePos()
	radialMenuSize = size
	radialMenuMouseButton = mouse_button
	radialMenuActions = actions
	radialMenuMousePos = ui.getMousePos()
	-- move away from screen edge
	radialMenuPos.x = math.min(math.max(radialMenuPos.x, size*3), ui.screenWidth - size*3)
	radialMenuPos.y = math.min(math.max(radialMenuPos.y, size*3), ui.screenHeight - size*3)
end

-- TODO: add cloud Lang::SET_HYPERSPACE_TARGET_TO_FOLLOW_THIS_DEPARTURE
local radial_menu_actions_station = {
	{icon=ui.theme.icons.comms, tooltip=lc.REQUEST_DOCKING_CLEARANCE,
		action=function(target)
			local clearanceGranted = target:RequestDockingClearance(Game.player)
			-- TODO: play a negative sound if clearance is refused
			Game.player:SetNavTarget(target)
			ui.playSfx("OK")
		end},
	{icon=ui.theme.icons.autopilot_dock, tooltip=lc.AUTOPILOT_DOCK_WITH_STATION,
		action=function(target)
	 		if next(Game.player:GetEquip('autopilot')) ~= nil then
		 		Game.player:SetFlightControlState("CONTROL_AUTOPILOT")
		 		Game.player:AIDockWith(target)
		 		Game.player:SetNavTarget(target)
				ui.playSfx("OK")
			else
				Game.AddCommsLogLine(lc.NO_AUTOPILOT_INSTALLED)
			end
		end},
}

local radial_menu_actions_all_bodies = {
	{icon=ui.theme.icons.autopilot_fly_to, tooltip=lc.AUTOPILOT_FLY_TO_VICINITY_OF,
		action=function(target)
			if next(Game.player:GetEquip('autopilot')) ~= nil then
		 		Game.player:SetFlightControlState("CONTROL_AUTOPILOT")
		 		Game.player:AIFlyTo(target)
		 		Game.player:SetNavTarget(target)
				ui.playSfx("OK")
			else
				Game.AddCommsLogLine(lc.NO_AUTOPILOT_INSTALLED)
			end
		end},
}

local radial_menu_actions_systembody = {
	{icon=ui.theme.icons.autopilot_low_orbit, tooltip=lc.AUTOPILOT_ENTER_LOW_ORBIT_AROUND,
		action=function(target)
	 		if next(Game.player:GetEquip('autopilot')) ~= nil then
		 		Game.player:SetFlightControlState("CONTROL_AUTOPILOT")
		 		Game.player:AIEnterLowOrbit(target)
		 		Game.player:SetNavTarget(target)
				ui.playSfx("OK")
			else
				Game.AddCommsLogLine(lc.NO_AUTOPILOT_INSTALLED)
			end
		end},
	{icon=ui.theme.icons.autopilot_medium_orbit, tooltip=lc.AUTOPILOT_ENTER_MEDIUM_ORBIT_AROUND,
		action=function(target)
	 		if next(Game.player:GetEquip('autopilot')) ~= nil then
		 		Game.player:SetFlightControlState("CONTROL_AUTOPILOT")
		 		Game.player:AIEnterMediumOrbit(target)
		 		Game.player:SetNavTarget(target)
				ui.playSfx("OK")
			else
				Game.AddCommsLogLine(lc.NO_AUTOPILOT_INSTALLED)
			end
		end},
	{icon=ui.theme.icons.autopilot_high_orbit, tooltip=lc.AUTOPILOT_ENTER_HIGH_ORBIT_AROUND,
		action=function(target)
	 		if next(Game.player:GetEquip('autopilot')) ~= nil then
		 		Game.player:SetFlightControlState("CONTROL_AUTOPILOT")
		 		Game.player:AIEnterHighOrbit(target)
		 		Game.player:SetNavTarget(target)
				ui.playSfx("OK")
			else
				Game.AddCommsLogLine(lc.NO_AUTOPILOT_INSTALLED)
			end
		end},
}

function ui.openDefaultRadialMenu(body)
	if body then
		local actions = {}
		for _,v in pairs(radial_menu_actions_all_bodies) do
			table.insert(actions, v)
		end
		if body:IsStation() then
			for _,v in pairs(radial_menu_actions_station) do
				table.insert(actions, v)
			end
		elseif body:GetSystemBody() then
			for _,v in pairs(radial_menu_actions_systembody) do
				table.insert(actions, v)
			end
		end
		ui.openRadialMenu(body, 1, 30, actions)
	end
end

local radialMenuWasOpen = {}
function ui.radialMenu(id)
	if not radialMenuActions or #radialMenuActions == 0 then
		return
 	end
	local icons = {}
	local tooltips = {}
	for _,action in pairs(radialMenuActions) do
		local uv0, uv1 = ui.get_icon_tex_coords(action.icon)
		table.insert(icons, { id = ui.get_icons_texture(radialMenuIconSize), uv0 = uv0, uv1 = uv1 })
		-- TODO: don't just assume that radialMenuTarget is a Body
		table.insert(tooltips, string.interp(action.tooltip, { target = radialMenuTarget and radialMenuTarget.label or "UNKNOWN" }))
	end
	local n = pigui.RadialMenu(radialMenuPos, "##radialmenupopup", radialMenuMouseButton, icons, radialMenuSize, tooltips)
	if n == -1 then
		pigui.DisableMouseFacing(true)
		radialMenuWasOpen[id] = true
	elseif n >= 0 or n == -2 then
		radialMenuWasOpen[id] = false
		shouldShowRadialMenu = false
		local target = radialMenuTarget
		radialMenuTarget = nil
		pigui.DisableMouseFacing(false)
		-- hack, imgui lets the press go through, but eats the release, so Pi still thinks rmb is held
		pigui.SetMouseButtonState(3, false);
		-- ui.setMousePos(radialMenuMousePos)
		-- do this last, so it can theoretically open a new radial menu
		-- though we can't as no button is pressed that could be released :-/
		if n >= 0 then
			radialMenuActions[n+1].action(target)
		end
	end
	if n == -3 and radialMenuWasOpen[id] then
		pigui.SetMouseButtonState(3, false)
		pigui.DisableMouseFacing(false)
		radialMenuWasOpen[id] = false
	end
	return n
end
