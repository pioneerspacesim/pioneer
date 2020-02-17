-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = require 'pigui'
local InfoView = require 'pigui.views.info-view'
local Lang = require 'Lang'
local Engine = require 'Engine'
local Character = require 'Character'
local InfoFace = require 'libs.ui.PiguiFace'
local Event = require 'Event'
local pigui = Engine.pigui

local pionillium = ui.fonts.pionillium
local orbiteer = ui.fonts.orbiteer
local colors = ui.theme.colors
local icons = ui.theme.icons

local textTable = require 'pigui/libs/text-table'

local l = Lang.GetResource("ui-core")

local info_column_width = ui.screenWidth / 2
local itemSpacing = Vector2(math.ceil(6 * (ui.screenHeight / 1200)), math.ceil(12 * (ui.screenHeight / 1200)))
local windowPadding = Vector2(math.ceil(18 * (ui.screenHeight / 1200)), math.ceil(18 * (ui.screenHeight / 1200)))
local buttonSize = Vector2(40, 56) * (ui.screenHeight / 1200)
local iconSize = Vector2(56, 56) * (ui.screenHeight / 1200)
local faceSize = Vector2(440,424) * ui.screenWidth / 1200
local facegenSize = Vector2(264, 0) * (ui.screenHeight / 1200)
local facegenSpacing = Vector2(math.ceil(24* (ui.screenHeight / 1200)),math.ceil(6* (ui.screenHeight / 1200)))
local dummySize = Vector2((facegenSize.x - windowPadding.x - facegenSpacing.x*3 - buttonSize.x*2 - iconSize.x)/2, 56 * (ui.screenHeight / 1200))
local buttonSpaceSize = Vector2(math.ceil(facegenSpacing.x*2 + buttonSize.x*2 + iconSize.x), math.ceil(56 * (ui.screenHeight / 1200)))
local inputTextPadding = Vector2(math.ceil(18 * (ui.screenHeight / 1200)), math.ceil(18 * (ui.screenHeight / 1200)))

local face = nil

local function drawPlayerInfo()
    local player = Character.persistent.player

    ui.withStyleVars({WindowPadding = windowPadding, ItemSpacing = itemSpacing}, function()
        ui.child("PlayerInfoDetails", Vector2(info_column_width, 0), {"AlwaysUseWindowPadding"}, function()
            textTable.withHeading(l.COMBAT, orbiteer.xlarge, {
                { l.RATING, l[player:GetCombatRating()] },
                { l.KILLS,  string.format('%d',player.killcount) },
            })
            ui.text("")

            textTable.withHeading(l.REPUTATION, orbiteer.xlarge, {
                { l.STATUS..":", l[player:GetReputationRating()] },
            })
        end)
    end)
end

local faceFeatures = {
    {id = 'FEATURE_SPECIES', icon = icons.sun, tooltip = l.FEATURE_SPECIES},
    {id = 'FEATURE_RACE', icon = icons.planet_grid, tooltip = l.FEATURE_RACE},
    {id = 'FEATURE_GENDER', icon = icons.gender, tooltip = l.FEATURE_GENDER, callback = function(value)
        Character.persistent.player.female = (value % 2 == 1)
    end},
    {id = 'FEATURE_HEAD', icon = icons.personal, tooltip = l.FEATURE_HEAD},
    {id = 'FEATURE_EYES', icon = icons.view_internal, tooltip = l.FEATURE_EYES},
    {id = 'FEATURE_NOSE', icon = icons.nose, tooltip = l.FEATURE_NOSE},
    {id = 'FEATURE_MOUTH', icon = icons.mouth, tooltip = l.FEATURE_MOUTH},
    {id = 'FEATURE_HAIRSTYLE', icon = icons.hair, tooltip = l.FEATURE_HAIRSTYLE},
    {id = 'FEATURE_ACCESSORIES', icon = icons.accessories, tooltip = l.FEATURE_ACCESSORIES},
    {id = 'FEATURE_CLOTHES', icon = icons.clothes, tooltip = l.FEATURE_CLOTHES},
}

local function generateFace()
    return {
        --Fit Integer by 2 into an signed int
        FEATURE_SPECIES = Engine.rand:Integer() % 2^31,
        FEATURE_RACE = Engine.rand:Integer() % 2^31,
        FEATURE_GENDER = Character.persistent.player.female and 1 or 0,
        FEATURE_HEAD = Engine.rand:Integer() % 2^31,
        FEATURE_EYES = Engine.rand:Integer() % 2^31,
        FEATURE_NOSE = Engine.rand:Integer() % 2^31,
        FEATURE_MOUTH = Engine.rand:Integer() % 2^31,
        FEATURE_HAIRSTYLE = Engine.rand:Integer() % 2^31,
        FEATURE_ACCESSORIES = Engine.rand:Integer() % 2^31,
        FEATURE_CLOTHES = Engine.rand:Integer() % 2^31,
        FEATURE_ARMOUR = 0,
    }
end

local function changeFeature (idx, featureId, callback)
    local player = Character.persistent.player

    player.faceDescription[featureId] = (player.faceDescription[featureId] + idx) % 2^31
    if callback then callback(player.faceDescription[featureId]) end
    face = InfoFace.New(player, {windowPadding = windowPadding, itemSpacing = itemSpacing, size = faceSize})
end

local function faceGenButton(feature)
    ui.dummy(dummySize)
    ui.sameLine()
    if (ui.coloredSelectedButton('<##' .. feature.id, buttonSize, false, colors.buttonBlue, nil, true)) then
        changeFeature(-1, feature.id, feature.callback)
    end
    ui.sameLine()
    pigui.PushID(feature.tooltip)
    ui.icon(feature.icon, iconSize, colors.white)
    pigui.PopID()
    if pigui.IsItemHovered() then
        pigui.SetTooltip(feature.tooltip)
    end
    ui.sameLine()
    if (ui.coloredSelectedButton('>##' .. feature.id, buttonSize, false, colors.buttonBlue, nil, true)) then
        changeFeature(1, feature.id, feature.callback)
    end
end

local function drawPlayerView()
    local player = Character.persistent.player

    if player.faceDescription == nil then
        player.faceDescription = generateFace()
        player.faceDescription.FEATURE_GENDER = player.female and 1 or 0
    end

    ui.withStyleVars({WindowPadding = windowPadding, ItemSpacing = itemSpacing}, function()
        ui.child("PlayerInfoDetails", Vector2(0, 0), {"AlwaysUseWindowPadding"}, function()

            ui.withFont(orbiteer.xlarge.name, orbiteer.xlarge.size, function()
                ui.withStyleVars({FramePadding = inputTextPadding}, function()
                    ui.pushItemWidth(info_column_width - windowPadding.x)
                    local text, entered = ui.inputText("", player.name, {})
                    if entered then
                        player.name = text
                    end
                end)
            end)

            ui.child("Face", Vector2(info_column_width-windowPadding.x-itemSpacing.x*2-facegenSize.x, 0), {}, function()
                if(face == nil) then
                    face = InfoFace.New(player, {windowPadding = windowPadding, itemSpacing = itemSpacing, size = faceSize})
                end

                face:render()
            end)

            ui.sameLine()

            ui.withStyleVars({ItemSpacing = facegenSpacing}, function()
                ui.child("FaceGen", facegenSize, {}, function()
                    for i, v in ipairs(faceFeatures) do
                        faceGenButton(v)
                    end

                    ui.dummy(dummySize)
                    ui.sameLine()
                    if (ui.coloredSelectedIconButton(icons.random, buttonSpaceSize, false, 0, colors.buttonBlue, colors.white, l.RANDOM_FACE, iconSize)) then
                        player.faceDescription = generateFace()
                        face = InfoFace.New(player, {windowPadding = windowPadding, itemSpacing = itemSpacing, size = faceSize})
                    end
                end)
            end)
        end)
    end)
end

InfoView:registerView({
    id = "personalInfo",
    name = l.PERSONAL_INFORMATION,
    icon = icons.personal_info,
    showView = true,
    draw = function()
        info_column_width = ui.getWindowSize().x / 2
        ui.withStyleVars({ItemSpacing = itemSpacing}, function()
            ui.withFont(pionillium.medlarge.name, pionillium.large.size, function()
                ui.child("PlayerInfoDetails", Vector2(info_column_width, 0), {}, drawPlayerInfo)
                ui.sameLine()
                ui.child("PlayerView", Vector2(info_column_width - itemSpacing.x, 0), {}, drawPlayerView)
            end)
        end)
    end,
    refresh = function() end,
})

Event.Register("onGameEnd", function ()
    face = nil
end)
