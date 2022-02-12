-- Copyright © 2008-2022 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Lang = require 'Lang'
local Game = require 'Game'
local Engine = require 'Engine'
local Format = require 'Format'
local ShipDef = require 'ShipDef'
local StationView = require 'pigui.views.station-view'
local ModelSpinner = require 'PiGui.Modules.ModelSpinner'
local ModelSkin = require 'SceneGraph.ModelSkin'

local ui = require 'pigui'
local pionillium = ui.fonts.pionillium
local Vector2 = _G.Vector2

local pionillium = ui.fonts.pionillium

local modelSpinner = ModelSpinner()
local previewPattern
local previewSkin
local price

local function refreshModelSpinner()
	local player = Game.player
	local shipDef = ShipDef[player.shipId]
	modelSpinner:setModel(shipDef.modelName, previewSkin, previewPattern)
	modelSpinner:setSize(Vector2(300, 300))
	modelSpinner.spinning = false
	modelSpinner:draw()
end

local function changeColor()
	local player = Game.player
	local shipDef = ShipDef[player.shipId]
	previewSkin = ModelSkin.New():SetRandomColors(Engine.rand):SetDecal(shipDef.manufacturer)
	refreshModelSpinner()
end

local function changePattern()
	local player = Game.player
	local patterns = player.model.numPatterns
	
	if patterns == 0 then return end
	
	if previewPattern == (patterns - 1) then
		previewPattern = 0
	else
		previewPattern = previewPattern + 1
	end
	
	refreshModelSpinner()
end

local function applyChanges()
	local player = Game.player
	player.model.pattern = previewPattern
	player:SetSkin(previewSkin)
end

local function resetPreview()
	local player = Game.player
	previewPattern = player.model.pattern
	previewSkin = player:GetSkin()
	refreshModelSpinner()
end

local function getPaintChanged()
	local player = Game.player
	return (player.model.pattern == previewPattern and player:GetSkin() == previewSkin)
end

local function doPaintjob()
	local player = Game.player
	local shipDef = ShipDef[player.shipId]
	local patterns = player.model.numPatterns
	local price = 0
	
	modelSpinner:draw()
	
	local patternChanged = false
	ui.withFont(pionillium.medlarge, function()
		patternChanged = ui.button("Change Pattern", Vector2(200, 36))
	end)
	
	local colorChanged = false
	ui.withFont(pionillium.medlarge, function()
		colorChanged = ui.button("Change Color", Vector2(200, 36))
	end)
	
	local changesApplied = false
	ui.withFont(pionillium.medlarge, function()
		changesApplied = ui.button("Apply Changes", Vector2(200, 36))
	end)
	
	local discardChanges = false
	ui.withFont(pionillium.medlarge, function()
		discardChanges = ui.button("Reset Preview", Vector2(200, 36))
	end)
	
	if getPaintChanged() then
		price = (shipDef.hullMass / 1000)
	end
	ui.text("Price: " ..Format.Money(price, false))
	
	if patternChanged then
		changePattern()
	end
	
	if colorChanged then
		changeColor()
	end
	
	if changesApplied then
		applyChanges()
	end
	
	if discardChanges then
		resetPreview()
	end
	
end

StationView:registerView({
	id = "paintshopView",
	name = "Paintshop",
	icon = ui.theme.icons.ship,
	showView = true,
	draw = function()
		doPaintjob()
	end,
	refresh = function()
		resetPreview()
		refreshModelSpinner()
	end,
})
