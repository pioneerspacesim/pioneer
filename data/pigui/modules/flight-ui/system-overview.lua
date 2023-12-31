-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local Space = require 'Space'
local Lang = require 'Lang'
local Vector2 = _G.Vector2

local ui = require 'pigui'
local gameView = require 'pigui.views.game'

local lc = Lang.GetResource("core")
local lui = Lang.GetResource("ui-core");

local height_fraction = 1.6

local style = {
	buttonSize = ui.theme.styles.MainButtonSize,
	buttonPadding = ui.theme.styles.MainButtonPadding,
	innerSpacing = ui.theme.styles.ItemInnerSpacing,
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

gameView.registerSidebarModule("system-overview", {
	side = "right",
	showInHyperspace = false,
	icon = icons.system_overview,
	tooltip = lui.TOGGLE_OVERVIEW_WINDOW,
	exclusive = true,

	debugReload = function()
		package.reimport 'pigui.modules.system-overview-window'
		package.reimport()
	end,

	drawTitle = function()
		systemOverview:displaySidebarTitle(Game.system)
	end,

	drawBody = function()
		if not Game.system then
			return
		end

		systemOverview:displaySearch()

		systemOverview.size.y = math.max(ui.getContentRegion().y, ui.screenHeight / height_fraction)

		local root = Space.rootSystemBody
		local selected = { [Game.player:GetNavTarget() or 0] = true }

		systemOverview:display(Game.system, root, selected)
	end
})
