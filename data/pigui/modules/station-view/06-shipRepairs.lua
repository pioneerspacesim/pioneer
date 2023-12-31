-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = require 'pigui'
local StationView = require 'pigui.views.station-view'
local ShipDef = require "ShipDef"
local Game = require "Game"
local Rand = require "Rand"
local PiGuiFace = require 'pigui.libs.face'
local Format = require "Format"
local Character = require "Character"
local ModalWindow = require 'pigui.libs.modal-win'
local ModelSpinner = require 'PiGui.Modules.ModelSpinner'
local ModelSkin = require 'SceneGraph.ModelSkin'
local Lang = require 'Lang'
local l = Lang.GetResource("ui-core")

local rescaleVector = ui.rescaleUI(Vector2(1, 1), Vector2(1600, 900), true)

local colors = ui.theme.colors
local icons = ui.theme.icons

local pionillium = ui.fonts.pionillium
local orbiteer = ui.fonts.orbiteer

local face = nil
local stationSeed = false
local damageToRepair = nil
local repair_cost = 0

local Color = _G.Color
local Vector2 = _G.Vector2
local NUM_WELCOME_MESSAGES = 5
local modelSpinner = ModelSpinner()
local previewPattern
local previewSkin
local previewColors
local changesMade = false
local price = 0.0
local textColorDefault = Color(255, 255, 255)
local textColorWarning = Color(255, 255, 0)

local activeTab = 0

local widgetSizes = ui.rescaleUI({
	itemSpacing = Vector2(4, 9),
	faceSize = Vector2(586,565),
	buttonSize = Vector2(100, 0),
	verticalDummy = Vector2(0, 50),
	gaugeWidth = 600,
}, Vector2(1600, 900))


local popup = ModalWindow.New('ChiefMechanicPopup', function(self)
	ui.text(l.YOU_DONT_HAVE_ENOUGH_MONEY_FOR_THAT_OPTION)
	ui.dummy(Vector2((ui.getContentRegion().x - 100*rescaleVector.x) / 2, 0))
	ui.sameLine()
	if ui.button(l.OK, Vector2(100*rescaleVector.x, 0)) then
		self:close()
	end
end)

local tryRepair = function (damage, price)
	if Game.player:GetMoney() >= price then
		Game.player:AddMoney(-price)
		Game.player:SetHullPercent(Game.player:GetHullPercent() + damage)
		ui.playSfx("Repairing_Ship")
	else
		popup:open()
	end
end


local getRepairCost = function (percent, shipDef)
	-- repairing 1% hull damage costs 0.1% of ship price
	shipDef = shipDef or ShipDef[Game.player.shipId]
	return math.ceil(shipDef.basePrice * (percent * 0.1)) * 0.01
end


local getRepairMessage = function (damage, price)
	return string.interp(
		l.REPAIR_X_HULL_DAMAGE_FOR_X, {
			damage = string.format('%.1f', damage),
			price = Format.Money(price)
		})
end


local function round(num, numDecimalPlaces)
	local mult = 10^(numDecimalPlaces or 0)
	return math.floor(num * mult + 0.5) / mult
end

local function determinePaintshopAvailability()
	local player = Game.player
	local station = player:GetDockedWith()

	-- faction homeworlds always have paintshops
	if Game.system.faction ~= nil and Game.system.faction.hasHomeworld and Game.system.faction.homeworld == station.path:GetSystemBody().parent.path then
		return true
	end

	-- high population stations often have them
	local pop = station:GetSystemBody().population
	if pop > 0.00005 then -- Mars is about 0.0002
		stationSeed = station.seed
		local rand = Rand.New(station.seed .. '-paintshop')
		if rand:Number(0,1) < 0.75 then
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

local popupChangesApplied = ModalWindow.New('paintshopPopupChangesApplied', function(self)
	ui.text(l.NEW_PAINTJOB_APPLIED)
	ui.dummy(Vector2((ui.getContentRegion().x - 100*rescaleVector.x) / 2, 0))
	ui.sameLine()
	if ui.button(l.OK, Vector2(100*rescaleVector.x, 0)) then
		self:close()
	end
end)

-- prepares color for use on the ship model
local function reformatColor(colors)
	return { primary = colors[1], secondary = colors[2], trim = colors[3] }
end

local function changeColor()
	local player = Game.player
	local shipDef = ShipDef[player.shipId]
	local newColor = reformatColor(previewColors)
	previewSkin = ModelSkin.New():SetColors(newColor):SetDecal(shipDef.manufacturer)
	refreshModelSpinner()
end

local function changePattern(increment)
	local player = Game.player
	local patterns = player.model.numPatterns

	previewPattern = 1 + ((previewPattern - 1 + increment) % patterns)

	refreshModelSpinner()
	changesMade = true
end

local function updatePrice()
	local player = Game.player
	local shipDef = ShipDef[player.shipId]

	local approxSurfaceArea = (shipDef.frontCrossSec + shipDef.sideCrossSec + shipDef.topCrossSec) * 2
	-- round to 10
	price = math.floor(approxSurfaceArea / 10 + 0.5) * 10
end

local function applyChanges()
	local player = Game.player
	if not changesMade then return end
	if price < player:GetMoney() then
		player.model:SetPattern(previewPattern)
		player:SetSkin(previewSkin)
		player:AddMoney(-price)
		ui.playSfx("Painting_Ship")
		popupChangesApplied:open()
		changesMade = false
	else
		popup:open()
	end
end

local function resetPreview()
	local player = Game.player
	previewPattern = player.model.pattern
	previewSkin = player:GetSkin()
	previewColors = previewSkin:GetColors()
	refreshModelSpinner()
	updatePrice()
	changesMade = false
end

local function drawShipRepair()

	local hullPercent = round(Game.player:GetHullPercent())
	local damage = 100 - hullPercent
	local shipDef = ShipDef[Game.player.shipId]

	-- local intro = string.interp(l.YOUR_HULL_IS_AT_X_INTEGRITY, {value = string.format('%.1f', hullPercent)})

	ui.withStyleVars({ItemSpacing = widgetSizes.itemSpacing}, function ()
		local infoColumnWidth = ui.getContentRegion().x - widgetSizes.faceSize.x - widgetSizes.itemSpacing.x

		ui.child("ShipStatus", Vector2(infoColumnWidth, 0), {}, function ()

			ui.withFont(pionillium.body, function ()
				if hullPercent > 99.9 then
					ui.text(l.SHIP_IS_ALREADY_FULLY_REPAIRED)
				else
					ui.text(getRepairMessage(damageToRepair, repair_cost))
					ui.pushItemWidth(widgetSizes.gaugeWidth)
					damageToRepair = ui.sliderInt("##damageToRepair", damageToRepair, 1, damage, "%d%%")
					repair_cost = getRepairCost(damageToRepair, shipDef)

					if  ui.button(l.PAY, widgetSizes.buttonSize) then
						tryRepair(damageToRepair, repair_cost)
						-- If we repaired more than 50% of the damage, reset
						if  damageToRepair > damage / 2 then
							damageToRepair = damage - damageToRepair
						end
					end
				end
			end)

			ui.dummy(widgetSizes.verticalDummy)

			local gaugePos = ui.getCursorScreenPos()
			gaugePos.y = gaugePos.y + ui.getTextLineHeightWithSpacing() * 0.5
			ui.dummy(Vector2(widgetSizes.gaugeWidth, ui.getTextLineHeightWithSpacing()))

			ui.gauge(gaugePos, hullPercent, '', l.HULL_INTEGRITY, 0, 100, icons.hull, colors.gaugeEquipmentMarket, l.HUD_HULL_STRENGTH, widgetSizes.gaugeWidth, ui.getTextLineHeightWithSpacing())

			ui.newLine()

			ui.withFont(pionillium.heading, function()
				local paintshopAvailable = determinePaintshopAvailability()

				if (paintshopAvailable) then
					if ui.button(l.VISIT_PAINTSHOP, Vector2(250, 36)) then
						activeTab = 1
					end
				else
					ui.text(l.PAINTSHOP_NOT_AVAILABLE)
				end
			end)
		end)

		ui.sameLine()

		if(face ~= nil) then
			face:render()
		end
	end)
end

local function drawPaintshop()

	ui.withFont(pionillium.heading, function()
		if ui.button(l.VISIT_REPAIR_SERVICES, Vector2(250, 36)) then
			activeTab = 0
			return
		end
	end)

	local player = Game.player
	local shipDef = ShipDef[player.shipId]
	local station = player:GetDockedWith()

	local available = determinePaintshopAvailability()
	if not available then
		ui.text(l.PAINTSHOP_NOT_AVAILABLE)
		return
	end

	local rand = Rand.New(station.seed)

	local priceColor = textColorDefault
	if price > player:GetMoney() then
		priceColor = textColorWarning
	end

	local columnWidth = ui.getContentRegion().x/2
	local itemSpacing = Vector2(8, 6)
	local verticalDummy = Vector2(0, 50)
	ui.withStyleVars({ ItemSpacing = itemSpacing }, function ()
		ui.child("PaintshopModelSpinner", Vector2(columnWidth, 0), {}, function()
			modelSpinner:setSize(ui.getContentRegion())
			modelSpinner:draw()
		end)

		ui.sameLine()

		ui.child("PaintshopControls", Vector2(columnWidth, 0), {}, function ()
			ui.text(l["PAINTSHOP_WELCOME_" .. rand:Integer(NUM_WELCOME_MESSAGES - 1)])
			ui.dummy(verticalDummy)
			ui.text(l.PLEASE_DESIGN_NEW_PAINTJOB)
			local priChanged, secChanged, triChanged
			priChanged, previewColors[1] = ui.colorEdit((l.COLOR.." 1"), previewColors[1], { "NoAlpha" })
			secChanged, previewColors[2] = ui.colorEdit((l.COLOR.." 2"), previewColors[2], { "NoAlpha" })
			triChanged, previewColors[3] = ui.colorEdit((l.COLOR.." 3"), previewColors[3], { "NoAlpha" })

			local colorChanged = (priChanged or secChanged or triChanged)
			if colorChanged then
				changesMade = true
			end

			if colorChanged then
				changeColor()
			end

			ui.withFont(pionillium.body, function()

				if ui.button("<", Vector2(20, 36)) then
					changePattern(-1)
				end
				ui.sameLine()
				ui.text(l.PATTERN.. " " ..previewPattern)
				ui.sameLine()
				if ui.button(">", Vector2(20, 36)) then
					changePattern(1)
				end

				ui.sameLine()

				if ui.button(l.RESET_PREVIEW, Vector2(200, 36)) then
					resetPreview()
				end

				ui.dummy(verticalDummy)

				ui.withStyleColors({["Text"] = priceColor }, function()
					ui.text(l.PRICE.. ": " ..Format.Money(price, false))
				end)
				if ui.button(l.PURCHASE_PAINTJOB, Vector2(200, 36)) then
					applyChanges()
				end
			end)
		end)
	end)
end


StationView:registerView({
	id = "shipRepairs",
	name = l.SHIP_REPAIRS,
	icon = ui.theme.icons.repairs,
	showView = true,
	draw = function()
		ui.withFont(pionillium.body, function()
			if (activeTab == 0) then
				drawShipRepair()
			else
				drawPaintshop()
			end
		end)
	end,
	refresh = function ()
		if not determinePaintshopAvailability() then
			activeTab = 0
		end

		local station = Game.player:GetDockedWith()
		-- Don't reset player's choice if temporarily leaving ship repair screen
		if not damageToRepair then
			damageToRepair = 1
		end
		if (station) then
			if (stationSeed ~= station.seed) then
				stationSeed = station.seed
				local rand = Rand.New(station.seed .. '-repair-guy')
				face = PiGuiFace.New(Character.New({ title = l.CHIEF_MECHANIC }, rand),
							{itemSpacing = widgetSizes.itemSpacing})
			end
		end
		resetPreview()
	end,
	debugReload = function()
		package.reimport()
	end
})
