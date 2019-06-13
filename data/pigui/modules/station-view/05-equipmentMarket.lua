-- Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Lang = import 'Lang'
local Game = import 'Game'
local Format = import 'Format'
local Equipment = import 'Equipment'
local StationView = import 'pigui/views/station-view'
local Market = import 'pigui/libs/market.lua'

local ui = import 'pigui/pigui.lua'
local colors = ui.theme.colors
local icons = ui.theme.icons
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

local equipmentMarket
local equipmentMarketPlayer

equipmentMarket = Market.New("EquipmentMarket", l.AVAILABLE_FOR_PURCHASE, {
	itemTypes = { Equipment.cargo, Equipment.misc, Equipment.laser, Equipment.hyperspace },
	columnCount = 5,
	initTable = function(self)
		ui.setColumnWidth(0, self.style.size.x / 2)

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
	renderRow = function(self, item)
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

	displayItem = function (s, e) return e.purchasable and hasTech(e) and not e:IsValidSlot("cargo", Game.player) end,
	onMouseOverItem = function(s, e)
		local tooltip = e:GetDescription()

		if string.len(tooltip) > 0 then ui.setTooltip(tooltip) end
		if ui.isMouseClicked(0) and s.funcs.onClickBuy(e) then
			s.funcs.buy(s, e)
			s:refresh()
			equipmentMarketPlayer:refresh()
		end
	end
})

equipmentMarketPlayer = Market.New("EquipmentMarketPlayer", l.EQUIPPED, {
	itemTypes = { Equipment.cargo, Equipment.misc, Equipment.laser, Equipment.hyperspace },
	columnCount = 4,
	initTable = function(self)
		ui.setColumnWidth(0, self.style.size.x / 3)

		ui.text(l.NAME_OBJECT)
		ui.nextColumn()
		ui.text(l.AMOUNT)
		ui.nextColumn()
		ui.text(l.MASS)
		ui.nextColumn()
		ui.text(l.TOTAL_MASS)
		ui.nextColumn()
	end,
	renderRow = function(self, item)
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
			self.popupMsg = l.CANNOT_SELL_ITEM
			ui.openPopup(self.popupMsgId)
			return false
		end

		if not hasTech(e) then
			self.popupMsg = l.STATION_TECH_TOO_LOW
			ui.openPopup(self.popupMsgId)
			return false
		end

		return true
	end,
	displayItem = function (s, e) return e.purchasable and Game.player:CountEquip(e) > 0 and not e:IsValidSlot("cargo", Game.player) end,
	getStock = function (s, e) return Game.player:CountEquip(e) end,
	onMouseOverItem = function(s, e)
		local tooltip = e:GetDescription()

		if string.len(tooltip) > 0 then ui.setTooltip(tooltip) end
		if ui.isMouseClicked(0) and s.funcs.onClickSell(s, e) then
			s.funcs.sell(s, e)
			equipmentMarket:refresh()
			equipmentMarketPlayer:refresh()
		end
	end
})

local function drawEquipmentView()
	local inventoryPadding = Vector2(20, 10)
	local player = Game.player
	ui.withFont(pionillium.medlarge.name, pionillium.large.size, function()

		ui.child("equipmentMarketContainer", Vector2(0, ui.getContentRegion().y - 100), {}, function()
			equipmentMarket:render()
			ui.sameLine()
			equipmentMarketPlayer:render()
		end)

		ui.withStyleVars({WindowPadding = inventoryPadding, ItemSpacing = equipmentMarket.style.itemSpacing}, function()
			ui.child("shipInventoryContainer", Vector2(0, 0), {"AlwaysUseWindowPadding"}, function()
				ui.text('')
				ui.columns(4, 'shipInventory', false)
				ui.text(l.CASH .. ': ' .. Format.Money(Game.player:GetMoney()))
				ui.nextColumn()
				ui.text(l.CAPACITY .. ': ')
				ui.sameLine()
				local gaugePos = ui.getWindowPos() + ui.getCursorPos() + inventoryPadding
				local gaugeWidth = ui.getContentRegion().x - inventoryPadding.x - equipmentMarket.style.itemSpacing.x
				ui.gauge(gaugePos, player.usedCapacity, '', string.format('%%it %s / %it %s', l.USED, player.freeCapacity, l.FREE), 0, player.usedCapacity + player.freeCapacity, icons.market, colors.gaugeEquipmentMarket, '', gaugeWidth, ui.getTextLineHeight())
				ui.nextColumn()
				ui.text(l.CABINS .. ': ')
				ui.sameLine()
				local cabins_free = player.cabin_cap or 0
				local cabins = player:GetEquipCountOccupied("cabin")
				gaugePos = ui.getWindowPos() + ui.getCursorPos() + inventoryPadding
				gaugeWidth = ui.getContentRegion().x - inventoryPadding.x - equipmentMarket.style.itemSpacing.x
				ui.gauge(gaugePos, cabins - cabins_free, '', string.format('%%it %s / %it %s', l.USED, cabins, l.FREE), 0, cabins, icons.personal, colors.gaugeEquipmentMarket, '', gaugeWidth, ui.getTextLineHeight())
				ui.nextColumn()
				ui.text(l.LEGAL_STATUS .. ': ' .. l.CLEAN)
				ui.columns(1, '', false)
			end)
		end)
	end)
end

StationView:registerView({
	id = "equipmentMarket",
	name = l.EQUIPMENT_MARKET,
	icon = ui.theme.icons.equipment,
	showView = true,
	draw = drawEquipmentView,
	refresh = function()
		equipmentMarket:refresh()
		equipmentMarketPlayer:refresh()
	end,
})
