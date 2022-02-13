-- Copyright © 2008-2022 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local Engine = require 'Engine'
local Format = require 'Format'
local ShipDef = require 'ShipDef'
local StationView = require 'pigui.views.station-view'
local ModalWindow = require 'pigui.libs.modal-win'
local ModelSpinner = require 'PiGui.Modules.ModelSpinner'
local ModelSkin = require 'SceneGraph.ModelSkin'
local Lang = require 'Lang'
local l = Lang.GetResource("ui-core")

local ui = require 'pigui'
local pionillium = ui.fonts.pionillium
local Vector2 = _G.Vector2
local rescaleVector = ui.rescaleUI(Vector2(1, 1), Vector2(1600, 900), true)

local pionillium = ui.fonts.pionillium

local modelSpinner = ModelSpinner()
local previewPattern
local previewSkin
local changesMade = false
local price = 0.0

local function refreshModelSpinner()
	local player = Game.player
	local shipDef = ShipDef[player.shipId]
	modelSpinner:setModel(shipDef.modelName, previewSkin, previewPattern)
	modelSpinner:setSize(Vector2(300, 300))
	modelSpinner.spinning = false
	modelSpinner:draw()
end

local popupNotEnoughMoney = ModalWindow.New('paintshopPopupNotEnoughMoney', function(self)
	ui.text(l.YOU_NOT_ENOUGH_MONEY)
	ui.dummy(Vector2((ui.getContentRegion().x - 100*rescaleVector.x) / 2, 0))
	ui.sameLine()
	if ui.button(l.OK, Vector2(100*rescaleVector.x, 0)) then
		self:close()
	end
end)

local popupChangesApplied = ModalWindow.New('paintshopPopupChangesApplied', function(self)
	ui.text(l.NEW_PAINTJOB_APPLIED)
	ui.dummy(Vector2((ui.getContentRegion().x - 100*rescaleVector.x) / 2, 0))
	ui.sameLine()
	if ui.button(l.OK, Vector2(100*rescaleVector.x, 0)) then
		self:close()
	end
end)

local function changeColor()
	local player = Game.player
	local shipDef = ShipDef[player.shipId]
	previewSkin = ModelSkin.New():SetRandomColors(Engine.rand):SetDecal(shipDef.manufacturer)
	refreshModelSpinner()
	changesMade = true
end

local function changePattern()
	local player = Game.player
	local patterns = player.model.numPatterns
	
	if patterns < 2 then return end
	
	if previewPattern == (patterns - 1) then
		previewPattern = 0
	else
		previewPattern = previewPattern + 1
	end
	
	refreshModelSpinner()
	changesMade = true
end

local function updatePrice()
	local player = Game.player
	local shipDef = ShipDef[player.shipId]
	
	if changesMade then
		price = (shipDef.hullMass)
	else
		price = 0.0
	end
end

local function applyChanges()
	local player = Game.player
	if not changesMade then return end
	
	if price < player:GetMoney() then
		player.model:SetPattern(previewPattern)
		player:SetSkin(previewSkin)
		player:AddMoney(-price)
		popupChangesApplied:open()
		changesMade = false
	else
		popupNotEnoughMoney:open()
	end
end

local function resetPreview()
	local player = Game.player
	previewPattern = player.model.pattern
	previewSkin = player:GetSkin()
	refreshModelSpinner()
	changesMade = false
end

local function paintshop()
	local player = Game.player
	local shipDef = ShipDef[player.shipId]
	local patterns = player.model.numPatterns
	
	modelSpinner:draw()
	updatePrice()
	
	ui.text(l.PRICE.. ": " ..Format.Money(price, false))
	
	local patternChanged = false
	ui.withFont(pionillium.medlarge, function()
		patternChanged = ui.button(l.CHANGE_PATTERN, Vector2(200, 36))
	end)
	
	local colorChanged = false
	ui.withFont(pionillium.medlarge, function()
		colorChanged = ui.button(l.CHANGE_COLOR, Vector2(200, 36))
	end)
	
	local changesApplied = false
	ui.withFont(pionillium.medlarge, function()
		changesApplied = ui.button(l.APPLY_CHANGES, Vector2(200, 36))
	end)
	
	local discardChanges = false
	ui.withFont(pionillium.medlarge, function()
		discardChanges = ui.button(l.RESET_PREVIEW, Vector2(200, 36))
	end)
	
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
		paintshop()
	end,
	refresh = function()
		resetPreview()
		refreshModelSpinner()
	end,
})
