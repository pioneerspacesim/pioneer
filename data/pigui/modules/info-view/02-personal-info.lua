-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = require 'pigui'
local InfoView = require 'pigui.views.info-view'
local Lang = require 'Lang'
local Engine = require 'Engine'
local Character = require 'Character'
local PiGuiFace = require 'pigui.libs.face'
local Event = require 'Event'
local pigui = Engine.pigui

local pionillium = ui.fonts.pionillium
local orbiteer = ui.fonts.orbiteer
local colors = ui.theme.colors
local icons = ui.theme.icons

local textTable = require 'pigui/libs/text-table'

local l = Lang.GetResource("ui-core")

local itemSpacing = ui.rescaleUI(Vector2(6, 12), Vector2(1920, 1200))
local faceSize = Vector2(440,424) * (ui.screenWidth / 1200)

local face = nil

local function drawPlayerInfo()
    local player = Character.persistent.player

	textTable.withHeading(l.COMBAT, orbiteer.xlarge, {
		{ l.RATING, l[player:GetCombatRating()] },
		{ l.KILLS,  string.format('%d',player.killcount) },
	})
	ui.text("")

	textTable.withHeading(l.REPUTATION, orbiteer.xlarge, {
		{ l.STATUS..":", l[player:GetReputationRating()] },
	})
end

InfoView:registerView({
    id = "personalInfo",
    name = l.PERSONAL_INFORMATION,
    icon = icons.personal_info,
    showView = true,
	draw = function()
		local spacing = InfoView.windowPadding.x * 2.0
        info_column_width = (ui.getColumnWidth() - spacing) / 2
        ui.withStyleVars({ItemSpacing = itemSpacing}, function()
            ui.withFont(pionillium.large, function()
                ui.child("PlayerInfoDetails", Vector2(info_column_width, 0), drawPlayerInfo)
                ui.sameLine(0, spacing)
				ui.child("PlayerView", Vector2(info_column_width, 0), function()
					face = face or PiGuiFace.New(Character.persistent.player, nil, true)
					face:render()
				end)
            end)
        end)
    end,
    refresh = function() end,
})

Event.Register("onGameEnd", function ()
    face = nil
end)
