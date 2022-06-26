-- Copyright © 2008-2022 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local Space = require 'Space'
local Lang = require 'Lang'

local ui = require 'pigui'
local lui = Lang.GetResource("ui-core");
local Vector2 = _G.Vector2

local width_fraction = 5
local height_fraction = 1.6

local style = {
	buttonSize = ui.theme.styles.MainButtonSize,
	buttonPadding = ui.theme.styles.MainButtonPadding
}

local systemOverview = require 'pigui.modules.system-overview-window'.New()
systemOverview.shouldDisplayPlayerDistance = true

local icons = ui.theme.icons

function systemOverview:onBodySelected(sbody, body)
	Game.player:SetNavTarget(body)
	ui.playSfx("OK")
end

function systemOverview:onBodyContextMenu(sbody, body)
	ui.openDefaultRadialMenu("systemoverviewspacetargets", body)
end

function systemOverview:overrideDrawButtons()
	if ui.mainMenuButton(icons.distance, lui.TOGGLE_OVERVIEW_SORT_BY_PLAYER_DISTANCE, self.shouldSortByPlayerDistance) then
		self.shouldSortByPlayerDistance = not self.shouldSortByPlayerDistance
	end
	ui.sameLine()

	self:drawControlButtons()

	ui.sameLine(ui.getWindowSize().x - (style.buttonSize.x + style.buttonPadding * 2 + ui.getWindowPadding().x))
	if ui.mainMenuButton(icons.system_overview, lui.TOGGLE_OVERVIEW_WINDOW) then
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
			ui.setNextWindowPos(Vector2(ui.screenWidth - ui.getWindowPadding().x, ui.getWindowPadding().y), "Always", Vector2(1, 0))
			ui.window("SystemTargetsSmall", windowFlags, function()
				if ui.mainMenuButton(icons.system_overview, lui.TOGGLE_OVERVIEW_WINDOW) then
					systemOverview.visible = true
				end
			end)
		else
			ui.setNextWindowSize(Vector2(ui.screenWidth / width_fraction, ui.screenHeight / height_fraction), "Always")
			ui.setNextWindowPos(Vector2(ui.screenWidth - ui.getWindowPadding().x, ui.getWindowPadding().y) , "Always", Vector2(1, 0))
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
