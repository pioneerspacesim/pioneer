-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Equipment = require 'Equipment'
local Game = require 'Game'
local utils = require 'utils'
local Event = require 'Event'

local Lang = require 'Lang'
local lc = Lang.GetResource("core");
local lui = Lang.GetResource("ui-core");
local lec = Lang.GetResource("equipment-core");

local ui = require 'pigui'

local player = nil
local colors = ui.theme.colors
local icons = ui.theme.icons

local mainIconSize = ui.rescaleUI(Vector2(32,32))
local mainWideIconSize = ui.rescaleUI(Vector2(64,32))
local mainForegroundColor = colors.reticuleCircle
local mainBackgroundColor = colors.lightBlueBackground
local mainHoverColor = colors.lightBlueBackground:tint(0.2)
local mainPressedColor = colors.lightBlueBackground:tint(0.5)
local mainButtonPadding = 3
local function iconEqButton(leftupper, icon, is_wide, icon_size, text, is_disabled, bg_color, fg_color, hover_color, pressed_color, tooltip)
	local textsize = ui.calcTextSize(text)
	local x = icon_size.x + textsize.x + mainButtonPadding * 2
	local y = math.max(icon_size.y, textsize.y) -- + mainButtonPadding * 2
	local delta = Vector2(x, y)
	local clicked = false
	if is_disabled then
		bg_color = colors.grey
	end
	if ui.isMouseHoveringRect(leftupper, leftupper + delta) and not ui.isAnyWindowHovered() then
		if not is_disabled then
			if ui.isMouseDown(0) then
				bg_color = pressed_color
			else
				bg_color = hover_color
			end
			if ui.isMouseReleased(0) then
				clicked = true
			end
		end
		ui.setTooltip(tooltip)
	end
	ui.addRectFilled(leftupper, leftupper + delta, bg_color, 0.1, 15)
	if is_wide then
		ui.addWideIcon(leftupper + Vector2(mainButtonPadding, mainButtonPadding), icon, fg_color, icon_size, ui.anchor.left, ui.anchor.top)
	else
		ui.addIcon(leftupper + Vector2(mainButtonPadding, mainButtonPadding), icon, fg_color, icon_size, ui.anchor.left, ui.anchor.top)
	end
	ui.addText(leftupper + Vector2(icon_size.x + mainButtonPadding, (icon_size.y - textsize.y)/2), fg_color, text) --  + mainButtonPadding
	return delta, clicked
end

local function displayECM(uiPos)
	player = Game.player
	local current_view = Game.CurrentView()
	if current_view == "WorldView" then
		local ecms = player:GetComponent("EquipSet"):GetInstalledOfType("utility.ecm")
		for i,ecm in ipairs(ecms) do
			local size, clicked = iconEqButton(uiPos, icons[ecm.ecm_type], false, mainIconSize, "ECM", not player:IsECMReady(), mainBackgroundColor, mainForegroundColor, mainHoverColor, mainPressedColor, lec[ecm.hover_message])
			uiPos.y = uiPos.y + size.y + 10
			if clicked then
				player:UseECM()
			end
		end
	end
	return uiPos
end

---@param player Player
---@param missile Equipment.MissileType
local function fireMissile(player, missile)
	if not player:GetCombatTarget() then
		Game.AddCommsLogLine(lc.SELECT_A_TARGET, "", 1)
	else
		player:FireMissileAt(missile, player:GetCombatTarget())
	end
end

local function displayMissiles(uiPos)
	if Game.CurrentView() == "WorldView" then

		local paused = Game.paused
		local docked = Game.player:GetDockedWith()

		local missiles = Game.player:GetComponent("EquipSet"):GetInstalledOfType("missile") --[[@as Equipment.MissileType[] ]]

		local groups = utils.automagic()

		for i, missile in ipairs(missiles) do
			local group = groups[missile.id]

			group.count = (group.count or 0) + 1
			group.size = missile.slot.size
			group.proto = missile:GetPrototype()
			group.index = i
		end

		local display = utils.build_array(pairs(groups))
		table.sort(display, function(a, b) return
			a.size < b.size or (a.size == b.size and (not a.guided and b.guided))
		end)

		for _, group in ipairs(display) do
			local count = tostring(group.count)

			-- TODO: slot size indicators should have a translated string at some point
			local tooltip = "{} (S{})" % { group.proto:GetName(), group.size }
			local size, clicked = iconEqButton(uiPos, icons[group.proto.icon_name], false, mainIconSize,
				count, false, mainBackgroundColor, mainForegroundColor, mainHoverColor, mainPressedColor, tooltip)
			uiPos.y = uiPos.y + size.y + 10

			if clicked and not paused and not docked then
				fireMissile(Game.player, missiles[group.index])
			end
		end

	end

	return uiPos
end

local function displayEquipment()
	if ui.optionsWindow.isOpen then return end
	local uiPos = Vector2(15, ui.screenHeight / 3 + 10)
	uiPos = displayMissiles(uiPos)
	uiPos = displayECM(uiPos + Vector2(0, 10))
end

ui.registerModule("game", displayEquipment)

return {}
