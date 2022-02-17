-- Copyright © 2008-2022 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Format = require 'Format'
local Game = require 'Game'
local Rand = require "Rand"
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
local Color = _G.Color
local rescaleVector = ui.rescaleUI(Vector2(1, 1), Vector2(1600, 900), true)

local pionillium = ui.fonts.pionillium

local numWelcomeMessages = 5
local modelSpinner = ModelSpinner()
local previewPattern
local previewSkin
local previewColors
local changesMade = false
local price = 0.0

local function determineAvailability()
	local player = Game.player
	local station = player:GetDockedWith()
	
	-- faction homeworlds always have paintshops
	if Game.system.faction ~= nil and Game.system.faction.hasHomeworld and Game.system.faction.homeworld == station.path:GetSystemBody().parent.path then
		return true
	end

	-- high population stations sometimes have them
	pop = station:GetSystemBody().population
	if pop > 0.0001 then -- Mars is about 0.0002
		stationSeed = station.seed
		rand = Rand.New(station.seed .. '-paintshop')
		if rand:Number(0,1) < 0.5 then 
			return true
		else
			return false
		end
	else
		return false
	end
end

local function refreshModelSpinner()
	local player = Game.player
	local shipDef = ShipDef[player.shipId]
	modelSpinner:setModel(shipDef.modelName, previewSkin, previewPattern)
	modelSpinner.spinning = false
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

-- TO DO: not do this abomination below
local function reformatColor()
	local newColor = {}
	
	newColor["primary"] = {}
	newColor["primary"]["r"] = previewColors[1].r/255
	newColor["primary"]["g"] = previewColors[1].g/255
	newColor["primary"]["b"] = previewColors[1].b/255
	newColor["primary"]["a"] = previewColors[1].a/255
	
	newColor["secondary"] = {}
	newColor["secondary"]["r"] = previewColors[2].r/255
	newColor["secondary"]["g"] = previewColors[2].g/255
	newColor["secondary"]["b"] = previewColors[2].b/255
	newColor["secondary"]["a"] = previewColors[2].a/255
	
	newColor["trim"] = {}
	newColor["trim"]["r"] = previewColors[3].r/255
	newColor["trim"]["g"] = previewColors[3].g/255
	newColor["trim"]["b"] = previewColors[3].b/255
	newColor["trim"]["a"] = previewColors[3].a/255
	
	return newColor
end

local function changeColor()
	local player = Game.player
	local shipDef = ShipDef[player.shipId]
	newColor = reformatColor()
	previewSkin = ModelSkin.New():SetColors(newColor):SetDecal(shipDef.manufacturer)
	refreshModelSpinner()
end

local function changePattern(increment)
	local player = Game.player
	local patterns = player.model.numPatterns
	
	if patterns < 2 then return end
	
	previewPattern = previewPattern + increment
	
	if previewPattern > patterns then
		previewPattern = 1
	elseif previewPattern < 1 then
		previewPattern = patterns - 1
	end

	refreshModelSpinner()
	changesMade = true
end

local function updatePrice()
	local player = Game.player
	local shipDef = ShipDef[player.shipId]
	
	if changesMade then
		local approxSurfaceArea = (shipDef.frontCrossSec + shipDef.sideCrossSec + shipDef.topCrossSec) * 2
		-- round to 10
		price = math.floor(approxSurfaceArea / 10 + 0.5) * 10 
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
	previewColors = previewSkin:GetColors()
	
	-- convert to a color format that works with ui.colorEdit
	previewColors[1] = Color(string.format("%x%x%x", previewColors[1]["r"], previewColors[1]["g"], previewColors[1]["b"]))
	previewColors[2] = Color(string.format("%x%x%x", previewColors[2]["r"], previewColors[2]["g"], previewColors[2]["b"]))
	previewColors[3] = Color(string.format("%x%x%x", previewColors[3]["r"], previewColors[3]["g"], previewColors[3]["b"]))
	
	refreshModelSpinner()
	changesMade = false
end

local function paintshop()
	local player = Game.player
	local shipDef = ShipDef[player.shipId]
	local station = player:GetDockedWith()
	
	local available = determineAvailability()
	if not available then
		ui.text(l.PAINTSHOP_NOT_AVAILABLE)
		return
	end
	
	updatePrice()
	
	local rand = Rand.New(station.seed)
	
	local columnWidth = ui.getContentRegion().x/2
	local itemSpacing = Vector2(8, 6)
	local verticalDummy = Vector2(0, 50)
	ui.withStyleVars({ItemSpacing = itemSpacing}, function ()
		ui.child("PaintshopControls", Vector2(columnWidth, 0), {}, function ()
			ui.text(l["PAINTSHOP_WELCOME_" .. rand:Integer(numWelcomeMessages - 1)])
			ui.dummy(verticalDummy)
			ui.text(l.PRICE.. ": " ..Format.Money(price, false))
			
			local priChanged, secChanged, triChanged
			priChanged, previewColors[1] = ui.colorEdit((l.COLOR.." 1"), previewColors[1], false)
			secChanged, previewColors[2] = ui.colorEdit((l.COLOR.." 2"), previewColors[2], false)
			triChanged, previewColors[3] = ui.colorEdit((l.COLOR.." 3"), previewColors[3], false)
	
			local colorChanged = (priChanged or secChanged or triChanged)
			if colorChanged then
				changesMade = true
			end
	
			if colorChanged then
				changeColor()
			end
	
			ui.withFont(pionillium.medlarge, function()
			
				if ui.button("<", Vector2(20, 20)) then
					changePattern(-1)
				end
				ui.sameLine()
				ui.text(l.PATTERN.. " " ..previewPattern)
				ui.sameLine()
				if ui.button(">", Vector2(20, 20)) then
					changePattern(1)
				end
				
				ui.dummy(verticalDummy)
				
				if ui.button(l.APPLY_CHANGES, Vector2(200, 36)) then
					applyChanges()
				end
				if ui.button(l.RESET_PREVIEW, Vector2(200, 36)) then
					resetPreview()
				end
			end)
		end)
	
		ui.sameLine()

		ui.child("PaintshopModelSpinner", Vector2(columnWidth, 0), {}, function()
			modelSpinner:setSize(ui.getContentRegion())
			modelSpinner:draw()
		end)
	end)
end

StationView:registerView({
	id = "paintshopView",
	name = l.PAINTSHOP,
	icon = ui.theme.icons.ship,
	showView = true,
	draw = function()
		paintshop()
	end,
	refresh = function()
		resetPreview()
	end,
})
