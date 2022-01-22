-- Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local Space = require 'Space'
local Lang = require 'Lang'

local ui = require 'pigui'
local lui = Lang.GetResource("ui-core");

local width_fraction = ui.rescaleFraction(5)
local height_fraction = 2

local style = ui.rescaleUI {
	screenPadding = 10,
	buttonSize = Vector2(32, 32),
}

local systemOverview = require 'pigui.modules.system-overview-window'.New()
systemOverview.shouldDisplayPlayerDistance = true

local icons = ui.theme.icons

local frame_padding = 1
local bg_color = ui.theme.colors.buttonBlue
local fg_color = ui.theme.colors.white

function systemOverview:onBodySelected(sbody, body)
	Game.player:SetNavTarget(body)
	ui.playSfx("OK")
end

function systemOverview:onBodyContextMenu(sbody, body)
	ui.openDefaultRadialMenu(body)
end

function systemOverview:overrideDrawButtons()
	if ui.coloredSelectedIconButton(icons.distance, style.buttonSize, self.shouldSortByPlayerDistance, frame_padding, bg_color, fg_color, lui.TOGGLE_OVERVIEW_SORT_BY_PLAYER_DISTANCE) then
		self.shouldSortByPlayerDistance = not self.shouldSortByPlayerDistance
	end
	ui.sameLine()

	self:drawControlButtons()

	ui.sameLine(ui.getWindowSize().x - (style.buttonSize.x + style.screenPadding))
	if ui.coloredSelectedIconButton(icons.system_overview, style.buttonSize, false, frame_padding, bg_color, fg_color, lui.TOGGLE_OVERVIEW_WINDOW) then
		self.visible = false
	end
end

local windowFlags = ui.WindowFlags {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus", "NoSavedSettings"}
local function showInfoWindow()
	if Game.CurrentView() == "world" then
		if Game.InHyperspace() or not Game.system.explored then
			systemOverview.visible = false
		end
		if not systemOverview.visible then
			ui.setNextWindowPos(Vector2(ui.screenWidth - style.screenPadding , style.screenPadding), "Always", Vector2(1, 0))
			ui.window("SystemTargetsSmall", windowFlags, function()
				if ui.coloredSelectedIconButton(icons.system_overview, style.buttonSize, false, frame_padding, bg_color, fg_color, lui.TOGGLE_OVERVIEW_WINDOW) then
					systemOverview.visible = true
				end
			end)
		else
			ui.setNextWindowSize(Vector2(ui.screenWidth / width_fraction, ui.screenHeight / height_fraction) , "Always")
			ui.setNextWindowPos(Vector2(ui.screenWidth - (ui.screenWidth / width_fraction) - style.screenPadding , style.screenPadding) , "Always")
			ui.withStyleColorsAndVars({ WindowBg = ui.theme.colors.commsWindowBackground }, { WindowRounding = 0.0 }, function()
				ui.window("SystemTargets", windowFlags, function()
					ui.withFont(ui.fonts.pionillium.medium, function()
						local root = Space.rootSystemBody
						local selected = { [Game.player:GetNavTarget() or 0] = true }

						systemOverview:display(Game.system, root, selected)
					end)
				end)
			end)

			if ui.ctrlHeld() and ui.isKeyReleased(ui.keys.delete) then
				package.reimport 'pigui.modules.system-overview-window'
				package.reimport()
			end
		end
	end
end

ui.registerModule("game", {
	id = "system-overview-window",
	draw = showInfoWindow
})
