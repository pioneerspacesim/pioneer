-- Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = import 'pigui/pigui.lua'
local InfoView = import 'pigui/views/info-view'
local Lang = import 'Lang'
local Engine = import 'Engine'
local Game = import 'Game'
local Vector = import 'Vector'
local Character = import 'Character'

local pionillium = ui.fonts.pionillium
local orbiteer = ui.fonts.orbiteer
local colors = ui.theme.colors

local drawTable = import 'pigui/libs/table.lua'

local l = Lang.GetResource("ui-core")

local function drawPlayerInfo()
    local player = Character.persistent.player
    drawTable.withHeading(l.COMBAT, orbiteer.large, {
        2, "playerCombatInfo", false, {
            { l.RATING, l[player:GetCombatRating()] },
            { l.KILLS,  string.format('%d',player.killcount) },
        }
    })
    ui.text("")

    drawTable.withHeading(l.REPUTATION, orbiteer.large, {
        2, "playerReputationInfo", false, {
            { l.STATUS..":", l[player:GetReputationRating()] },
        }
    })
    ui.text("")

    drawTable.withHeading(l.MILITARY, orbiteer.large, {
        2, "playerMilitaryInfo", false, {
            { l.ALLEGIANCE, l.NONE }, -- XXX
            { l.RANK,      l.NONE }, -- XXX
        }
    })
end

local function drawPlayerView()
    local player = Character.persistent.player
    local faceFlags = { player.female and "FEMALE" or "MALE" }

    -- local nameEntry = ui:TextEntry(player.name):SetFont("HEADING_LARGE")
    -- nameEntry.onChange:Connect(function (newName)
    --     player.name = newName
    --     faceWidget:UpdateInfo(player)
    -- end )
    --
    -- local genderToggle = SmallLabeledButton.New(l.TOGGLE_MALE_FEMALE)
    -- genderToggle.button.onClick:Connect(function ()
    --     faceWidget = InfoFace.New(player)
    --     faceWidgetContainer:SetInnerWidget(faceWidget.widget)
    -- end)
    --
    -- local generateFaceButton = SmallLabeledButton.New(l.MAKE_NEW_FACE)
    -- generateFaceButton.button.onClick:Connect(function ()
    --     faceWidget = InfoFace.New(player)
    --     faceWidgetContainer:SetInnerWidget(faceWidget.widget)
    -- end)
    ui.columns(2, "", false)
    ui.withFont(orbiteer.large.name, orbiteer.large.size, function()
        ui.text(player.name)
    end)
    local text, entered = ui.inputText("", player.name)
    if entered then
        player.name = text
        -- TODO: update the face display
    end
    ui.nextColumn()
    local buttonSize = Vector(ui.GetColumnWidth, 30 * ui.screenHeight / 1200)
    if ui.coloredSelectedButton(l.TOGGLE_MALE_FEMALE, buttonSize, false, colors.buttonBlue, "") then
        -- TODO: update the face display
        player.female = not player.female
    end

    if ui.coloredSelectedButton(l.MAKE_NEW_FACE, buttonSize, false, colors.buttonBlue, "") then
        -- TODO: update the face display
        player.seed = Engine.rand:Integer()
    end
    ui.columns(1, "", false)
    ui.text("")

    -- TODO: draw the face display
end

InfoView.registerView("personalInfo", {
    name = l.PERSONAL_INFORMATION,
    icon = ui.theme.icons.personal_info,
    draw = function()
        local info_column_width = 400 * ui.screenWidth / 1200
		local null_column_width = 300 * ui.screenWidth / 1200

		ui.withStyleVars({WindowPadding = Vector(10,10)}, function()
			ui.withFont(pionillium.medlarge.name, pionillium.medlarge.size, function()
				ui.child("PlayerInfoDetails", Vector(info_column_width, 0), drawPlayerInfo)
                ui.sameLine()
                -- Ugly little hack
                ui.child("", Vector(null_column_width, 0), function() end)
				ui.sameLine()
				ui.child("PlayerView", Vector(0, 0), drawPlayerView)
			end)
		end)
    end
})
