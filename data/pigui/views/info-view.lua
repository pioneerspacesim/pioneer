-- Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import 'Engine'
local Game = import 'Game'
local ui = import 'pigui/pigui.lua'
local Lang = import 'Lang'
local Color = import 'Color'
local Vector = import 'Vector'

local infoView = {
	currentTab = 0,
	views = {},
}

local lc = Lang.core
local pionillium = ui.fonts.pionillium
local orbiteer = ui.fonts.orbiteer

local colors = ui.theme.colors

local mainButtonSize = Vector(32,32) * (ui.screenHeight / 1200)
local mainButtonFramePadding = 3
local function infoButton(icon, selected, tooltip, color)
	if color == nil then
		color = colors.white
	end
	return ui.coloredSelectedIconButton(icon, mainButtonSize, selected, mainButtonFramePadding, colors.buttonBlue, color, tooltip)
end

function infoView.registerView(name, view)
	table.insert(infoView.views, view)
end

ui.registerModule("game", function()
	if Game.CurrentView() ~= "info" then return end
	local view = infoView.views[infoView.currentTab] or infoView.views[1]
	if not view then return end

	local button_window_size = Vector(
		(mainButtonSize.x + mainButtonFramePadding * 2) * 8,
		(mainButtonSize.y + mainButtonFramePadding * 2) * 1.5)
	local button_window_pos = Vector(0, 0)

	local view_window_padding = Vector(4, 8)
	local view_window_size = Vector(
		ui.screenWidth - view_window_padding.x * 2,
		ui.screenHeight - button_window_size.y * 3.5 - view_window_padding.y * 2)
	local view_window_pos = Vector(
		view_window_padding.x,
		button_window_size.y + view_window_padding.y)

	ui.withStyleColors({
		["WindowBg"] = colors.blueBackground,
		["Border"] = colors.blueFrame
	}, function()
		ui.withStyleVars({
			WindowRounding = 0,
			WindowBorderSize = 1.0
		}, function()
			ui.setNextWindowPos(view_window_pos, "Always")
			ui.setNextWindowSize(view_window_size, "Always")
			ui.window("InfoView", {"NoResize", "NoTitleBar"}, view.draw)
		end)
	end)

	ui.withFont(orbiteer.large.name, orbiteer.large.size * 1.5, function()
		local text_window_padding = 12
		local text_window_size = Vector(
			ui.calcTextSize(view.name).x + text_window_padding * 2,
			button_window_size.y + 6)
		local text_window_pos = Vector(ui.screenWidth - text_window_size.x, 3)

		ui.setNextWindowPos(text_window_pos, "Always")
		ui.setNextWindowSize(text_window_size, "Always")
		ui.window("InfoViewName", {"NoResize", "NoTitleBar", "NoMove", "NoFocusOnAppearing"}, function()
			ui.text(view.name)
		end)
	end)

	ui.setNextWindowSize(button_window_size, "Always")
	ui.setNextWindowPos(button_window_pos, "Always")
	ui.window("InfoViewButtons", {"NoResize", "NoTitleBar", "NoMove", "NoFocusOnAppearing"}, function()
		for i, v in ipairs(infoView.views) do
			if infoButton(v.icon, i == infoView.currentTab, v.name) then
				infoView.currentTab = i
			end
			ui.sameLine()
		end
	end)
end)

return infoView
