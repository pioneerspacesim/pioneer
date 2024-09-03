-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local Lang = require 'Lang'
local utils= require 'utils'

local ui = require 'pigui'
local ModalWindow = require 'pigui.libs.modal-win'
local TableWidget = require 'pigui.libs.table'

local l = Lang.GetResource("ui-core")

local sellPriceReduction = 0.8

local defaultFuncs = {
    -- can we display this item
    canDisplayItem = function (self, e)
        return e.purchasable
    end,

    -- how much of this item do we have in stock?
    getStock = function (self, e)
        return Game.player:GetDockedWith():GetEquipmentStock(e)
    end,

    -- what do we charge for this item if we are buying
    getBuyPrice = function (self, e)
        return Game.player:GetDockedWith():GetEquipmentPrice(e)
    end,

    -- what do we get for this item if we are selling
    getSellPrice = function (self, e)
        local basePrice = Game.player:GetDockedWith():GetEquipmentPrice(e)
        if basePrice > 0 then
			if e:IsValidSlot("cargo") then
				return basePrice
			else
				return sellPriceReduction * basePrice
			end
        else
            return 1.0/sellPriceReduction * basePrice
        end
    end,

    -- do something when a "buy" button is clicked
    -- return true if the buy can proceed
    onClickBuy = function (self, e)
        return true -- allow buy
    end,

    buy = function(self, e)
        if not self.funcs.onClickBuy(self, e) then return end

        if self.funcs.getStock(self, e) <= 0 then
			return self.funcs.onBuyFailed(self, e, l.ITEM_IS_OUT_OF_STOCK)
        end

        local player = Game.player

        -- if this ship model doesn't support fitting of this equip:
        if player:GetEquipSlotCapacity(e:GetDefaultSlot(player)) < 1 then
            return self.funcs.onBuyFailed(self, e, string.interp(l.NOT_SUPPORTED_ON_THIS_SHIP, {equipment = e:GetName(),}))
        end

        -- add to first free slot
        local slot
        for i=1,#e.slots do
            if player:GetEquipFree(e.slots[i]) > 0 then
                slot = e.slots[i]
                break
            end
        end

        -- if ship maxed out in any valid slot for e
        if not slot then
            return self.funcs.onBuyFailed(self, e, l.SHIP_IS_FULLY_EQUIPPED)
        end

        -- if ship too heavy to support more
        if player.freeCapacity < e.capabilities.mass then
            return self.funcs.onBuyFailed(self, e, l.SHIP_IS_FULLY_LADEN)
        end


        local price = self.funcs.getBuyPrice(self, e)
        if player:GetMoney() < self.funcs.getBuyPrice(self, e) then
            return self.funcs.onBuyFailed(self, e, l.YOU_NOT_ENOUGH_MONEY)
        end

        assert(player:AddEquip(e, 1, slot) == 1)
        player:AddMoney(-price)

        self.funcs.bought(self, e)
    end,

    -- do something when we buy this commodity
    bought = function (self, e, tradeamount)
		local count = tradeamount or 1  -- default to 1 for e.g. equipment market
        Game.player:GetDockedWith():AddEquipmentStock(e, -count)
    end,

	onBuyFailed = function (self, e, reason)
		self.popup.msg = reason
		self.popup:open()
	end,

    -- do something when a "sell" button is clicked
    -- return true if the buy can proceed
    onClickSell = function (self, e)
        return true -- allow sell
    end,

    sell = function(self, e)
        if not self.funcs.onClickSell(self, e) then return end

        local player = Game.player

        -- remove from last free slot (reverse table)
        local slot
        for i, s in utils.reverse(e.slots) do
            if player:CountEquip(e, s) > 0 then
                slot = s
                break
            end
        end

        player:RemoveEquip(e, 1, slot)
        player:AddMoney(self.funcs.getSellPrice(self, e))

        self.funcs.sold(self, e)
    end,

    -- do something when we sell this items
    sold = function (self, e, tradeamount)
		local count = tradeamount or 1  -- default to 1 for e.g. equipment market
        Game.player:GetDockedWith():AddEquipmentStock(e, count)
    end,

    initTable = function(self)
        ui.withFont(self.style.headingFont.name, self.style.headingFont.size, function()
            ui.text(self.title)
        end)

        ui.columns(5, self.id, false)
    end,

    renderRow = function(self, item)

    end,

    onMouseOverItem = function(self, item)

    end,

    -- sort items in the market table
    sortingFunction = function(e1,e2)
        if e1:GetDefaultSlot() == e2:GetDefaultSlot() then
            if e1:GetDefaultSlot() == "cargo" then
                return e1:GetName() < e2:GetName()        -- cargo sorted on translated name
            else
                if e1:GetDefaultSlot():find("laser") then -- can be laser_front or _back
                    if e1.l10n_key:find("PULSE") and e2.l10n_key:find("PULSE") or
                            e1.l10n_key:find("PLASMA") and e2.l10n_key:find("PLASMA") then
                        return e1.price < e2.price
                    else
                        return e1.l10n_key < e2.l10n_key
                    end
                else
                    return e1.l10n_key < e2.l10n_key
                end
            end
        else
            return e1:GetDefaultSlot() < e2:GetDefaultSlot()
        end
    end
}

local MarketWidget = {
	defaultFuncs = defaultFuncs
}

function MarketWidget.New(id, title, config)
    local self = TableWidget.New(id, title, config)

    self.popup = config.popup or ModalWindow.New('popupMsg' .. id, function()
        ui.text(self.popup.msg)
        ui.dummy(Vector2((ui.getContentRegion().x - 100) / 2, 0))
        ui.sameLine()
        if ui.button("OK", Vector2(100, 0)) then
            self.popup:close()
        end
    end)

    self.items = {}
    self.itemTypes = config.itemTypes or {}
    self.columnCount = config.columnCount or 0

    self.funcs.getStock        = config.getStock        or defaultFuncs.getStock
    self.funcs.getBuyPrice     = config.getBuyPrice     or defaultFuncs.getBuyPrice
    self.funcs.getSellPrice    = config.getSellPrice    or defaultFuncs.getSellPrice
    self.funcs.onClickBuy      = config.onClickBuy      or defaultFuncs.onClickBuy
    self.funcs.onClickSell     = config.onClickSell     or defaultFuncs.onClickSell
    self.funcs.buy             = config.buy             or defaultFuncs.buy
    self.funcs.bought          = config.bought          or defaultFuncs.bought
    self.funcs.onBuyFailed     = config.onBuyFailed     or defaultFuncs.onBuyFailed
    self.funcs.sell            = config.sell            or defaultFuncs.sell
    self.funcs.sold            = config.sold            or defaultFuncs.sold
    self.funcs.initTable       = config.initTable       or defaultFuncs.initTable
    self.funcs.canDisplayItem  = config.canDisplayItem  or defaultFuncs.canDisplayItem
    self.funcs.sortingFunction = config.sortingFunction or defaultFuncs.sortingFunction
    self.funcs.onMouseOverItem = config.onMouseOverItem or defaultFuncs.onMouseOverItem


    setmetatable(self, {
        __index = MarketWidget,
        class = "UI.MarketWidget",
    })

    return self
end

function MarketWidget:refresh()
    self.items = {}

    for i, type in pairs(self.itemTypes) do
        for j, item in pairs(type) do
            if self.funcs.canDisplayItem(self, item) then
                table.insert(self.items, item)
            end
        end
    end

    table.sort(self.items, self.funcs.sortingFunction)
end

function MarketWidget:render()
    TableWidget.render(self)
end

return MarketWidget
