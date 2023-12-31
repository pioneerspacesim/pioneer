-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = require 'pigui'
local Game = require 'Game'
local Vector2 = _G.Vector2

-- cache ui
local pionillium = ui.fonts.pionillium
local colors = ui.theme.colors

local lc = require 'Lang'.GetResource("core")
local lui = require 'Lang'.GetResource("ui-core")

local gameView = require 'pigui.views.game'

local function displayHyperspace()
	local uiPos = Vector2(ui.screenWidth / 2, ui.screenHeight / 2 - 10)
	local path,destName = Game.player:GetHyperspaceDestination()
	local label = string.interp(lui.HUD_IN_TRANSIT_TO_N_X_X_X, { system = destName, x = path.sectorX, y = path.sectorY, z = path.sectorZ })
	local r = ui.addStyledText(uiPos, ui.anchor.center, ui.anchor.bottom, label, colors.hyperspaceInfo, pionillium.large, nil, colors.lightBlackBackground)
	uiPos.y = uiPos.y + r.y + 20
	local percent = Game.GetHyperspaceTravelledPercentage() * 100
	label = string.interp(lui.HUD_JUMP_COMPLETE, { percent = string.format("%2.1f", percent) })
	ui.addStyledText(uiPos, ui.anchor.center, ui.anchor.top, label, colors.hyperspaceInfo, pionillium.large, nil, colors.lightBlackBackground)
end

local function displayHyperspaceCountdown()
	local player = Game.player
	if player:IsHyperspaceActive() then
		local countdown = math.ceil(player:GetHyperspaceCountdown())
                local path,destName = player:GetHyperspaceDestination()
		local uiPos = Vector2(ui.screenWidth / 2, ui.screenHeight / 3)
		ui.addStyledText(uiPos, ui.anchor.center, ui.anchor.bottom, string.interp(lui.HUD_HYPERSPACING_TO_N_IN_N_SECONDS ,{ destination = destName, countdown = countdown }), colors.hyperspaceInfo, pionillium.large)
	end
end

gameView.registerModule("hyperspace-percentage", {
    showInHyperspace = true,
    draw = function(self, dT)
        colors = ui.theme.colors
        if Game.InHyperspace() then
            displayHyperspace()
        elseif gameView.player:IsHyperspaceActive() then
            displayHyperspaceCountdown()
        end
    end
})
