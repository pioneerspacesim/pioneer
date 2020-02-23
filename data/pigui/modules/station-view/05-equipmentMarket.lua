-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Lang = import 'Lang'
local Game = import 'Game'
local Format = import 'Format'
local Equipment = import 'Equipment'
local StationView = import 'pigui/views/station-view'
local EquipMarket = import 'pigui/libs/equipment-market.lua'

local ui = import 'pigui/pigui.lua'
local pionillium = ui.fonts.pionillium
local l = Lang.GetResource("ui-core")

local hasTech = function (e)
	local station = Game.player:GetDockedWith()
	local equip_tech_level = e.tech_level or 1 -- default to 1

	if type(equip_tech_level) == "string" then
		if equip_tech_level == "MILITARY" then
			return station.techLevel == 11
		else
			error("Unknown tech level:\t"..equip_tech_level)
		end
	end

	assert(type(equip_tech_level) == "number")
	return station.techLevel >= equip_tech_level
end

local equipmentMarketStation
local equipmentMarketPlayer

equipmentMarketStation = EquipMarket.New("EquipmentMarket", l.AVAILABLE_FOR_PURCHASE, {
	itemTypes = { Equipment.cargo, Equipment.misc, Equipment.laser, Equipment.hyperspace },
	columnCount = 5,
	initTable = function(self)
		ui.setColumnWidth(0, self.style.size.x / 2.5)
		ui.setColumnWidth(3, ui.calcTextSize(l.MASS).x + self.style.itemSpacing.x + self.style.windowPadding.x)
		ui.setColumnWidth(4, ui.calcTextSize(l.IN_STOCK).x + self.style.itemSpacing.x + self.style.windowPadding.x)
	end,
	renderHeaderRow = function(self)
		ui.text(l.NAME_OBJECT)
		ui.nextColumn()
		ui.text(l.BUY)
		ui.nextColumn()
		ui.text(l.SELL)
		ui.nextColumn()
		ui.text(l.MASS)
		ui.nextColumn()
		ui.text(l.IN_STOCK)
		ui.nextColumn()
	end,
	renderItem = function(self, item)
		ui.withTooltip(item:GetDescription(), function()
			ui.text(item:GetName())
			ui.nextColumn()
			ui.text(Format.Money(self.funcs.getBuyPrice(self, item)))
			ui.nextColumn()
			ui.text(Format.Money(self.funcs.getSellPrice(self, item)))
			ui.nextColumn()
			ui.text(item.capabilities.mass.."t")
			ui.nextColumn()
			ui.text(self.funcs.getStock(self, item))
			ui.nextColumn()
		end)
	end,
	canDisplayItem = function (s, e) return e.purchasable and hasTech(e) and not e:IsValidSlot("cargo", Game.player) end,
	onMouseOverItem = function(s, e)
		local tooltip = e:GetDescription()
		if string.len(tooltip) > 0 then ui.setTooltip(tooltip) end
	end,
	onClickItem = function(s,e)
		if s.funcs.onClickBuy(s, e) then
			s.funcs.buy(s, e)
			equipmentMarketStation:refresh()
			equipmentMarketPlayer:refresh()
		end
	end
})

equipmentMarketPlayer = EquipMarket.New("EquipmentMarketPlayer", l.EQUIPPED, {
	itemTypes = { Equipment.cargo, Equipment.misc, Equipment.laser, Equipment.hyperspace },
	columnCount = 4,
	initTable = function(self)
		ui.setColumnWidth(0, self.style.size.x / 3)
	end,
	renderHeaderRow = function(self)
		ui.text(l.NAME_OBJECT)
		ui.nextColumn()
		ui.text(l.AMOUNT)
		ui.nextColumn()
		ui.text(l.MASS)
		ui.nextColumn()
		ui.text(l.TOTAL_MASS)
		ui.nextColumn()
	end,
	renderItem = function(self, item)
		ui.text(item:GetName())
		ui.nextColumn()
		ui.text(self.funcs.getStock(self, item))
		ui.nextColumn()
		ui.text(item.capabilities.mass.."t")
		ui.nextColumn()
		ui.text(tostring(self.funcs.getStock(self, item) * item.capabilities.mass) .. "t")
		ui.nextColumn()
	end,
	onClickSell = function (self, e)
		if e:IsValidSlot("cargo", Game.player) and not e.purchasable then
			self.popup.msg = l.CANNOT_SELL_ITEM
			self.popup:open()
			return false
		end

		if not hasTech(e) then
			self.popup.msg = l.STATION_TECH_TOO_LOW
			self.popup:open()
			return false
		end

		return true
	end,
	canDisplayItem = function (s, e) return e.purchasable and Game.player:CountEquip(e) > 0 and not e:IsValidSlot("cargo", Game.player) end,
	getStock = function (s, e) return Game.player:CountEquip(e) end,
	onMouseOverItem = function(s, e)
		local tooltip = e:GetDescription()
		if string.len(tooltip) > 0 then ui.setTooltip(tooltip) end
	end,
	onClickItem = function(s,e)
		if s.funcs.onClickSell(s, e) then
			s.funcs.sell(s, e)
			equipmentMarketStation:refresh()
			equipmentMarketPlayer:refresh()
		end
	end
})

local function drawEquipmentView()
	ui.withFont(pionillium.medlarge.name, pionillium.medlarge.size, function()

		ui.child("equipmentMarketContainer", Vector2(0, ui.getContentRegion().y - StationView.style.height), {}, function()
			equipmentMarketStation:render()
			ui.sameLine()
			equipmentMarketPlayer:render()
		end)

		StationView:shipSummary()
	end)
end

StationView:registerView({
	id = "equipmentMarketView",
	name = l.EQUIPMENT_MARKET,
	icon = ui.theme.icons.equipment,
	showView = true,
	draw = drawEquipmentView,
	refresh = function()
		equipmentMarketStation:refresh()
		equipmentMarketPlayer:refresh()
		equipmentMarketStation.scrollReset = true
		equipmentMarketPlayer.scrollReset = true
	end,
})
