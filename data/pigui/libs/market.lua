-- Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = import 'Game'
local Lang = import 'Lang'

local ui = import 'pigui/pigui.lua'

local l = Lang.GetResource("ui-core")

local sellPriceReduction = 0.8

local defaultFuncs = {
    -- can we display this item
    displayItem = function (self, e)
        return e.purchasable and e:IsValidSlot("cargo") and Game.system:IsCommodityLegal(e)
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
            return sellPriceReduction * basePrice
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
            self.popupMsg = l.ITEM_IS_OUT_OF_STOCK
            ui.openPopup(self.popupMsgId)
            return
        end

        local player = Game.player

        -- if this ship model doesn't support fitting of this equip:
        if player:GetEquipSlotCapacity(e:GetDefaultSlot(player)) < 1 then
            self.popupMsg = string.interp(l.NOT_SUPPORTED_ON_THIS_SHIP, {equipment = e:GetName(),})
            ui.openPopup(self.popupMsgId)
            return
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
            self.popupMsg = l.SHIP_IS_FULLY_EQUIPPED
            ui.openPopup(self.popupMsgId)
            return
        end

        -- if ship too heavy to support more
        if player.freeCapacity < e.capabilities.mass then
            self.popupMsg = l.SHIP_IS_FULLY_LADEN
            ui.openPopup(self.popupMsgId)
            return
        end


        local price = self.funcs.getBuyPrice(self, e)
        if player:GetMoney() < self.funcs.getBuyPrice(self, e) then
            self.popupMsg = l.YOU_NOT_ENOUGH_MONEY
            ui.openPopup(self.popupMsgId)
            return
        end

        assert(player:AddEquip(e, 1, slot) == 1)
        player:AddMoney(-price)

        self.funcs.bought(self, e)
    end,

    -- do something when we buy this commodity
    bought = function (self, e)
        Game.player:GetDockedWith():AddEquipmentStock(e, -1)
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
        for i=#e.slots,1,-1 do
            if player:CountEquip(e, e.slots[i]) > 0 then
                slot = e.slots[i]
                break
            end
        end

        player:RemoveEquip(e, 1, slot)
        player:AddMoney(self.funcs.getSellPrice(self, e))

        self.funcs.sold(self, e)
    end,

    -- do something when we sell this items
    sold = function (self, e)
        Game.player:GetDockedWith():AddEquipmentStock(e, 1)
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

local MarketWidget = {}

function MarketWidget.New(id, title, config)
    local defaultSizes = ui.rescaleUI({
        windowPadding = Vector2(14, 14),
        itemSpacing = Vector2(4, 9),
    }, Vector2(1600, 900))

    local self = {
        id = id,
        popupMsgId = 'popupMsg' .. id,
        popupMsg = '',
        title = title,
        items = {},
        itemTypes = config.itemTypes or {},
        columnCount = config.columnCount or 0,
        style = {
            windowPadding = config.windowPadding or defaultSizes.windowPadding,
            itemSpacing = config.itemSpacing or defaultSizes.itemSpacing,
            size = config.size or Vector2(ui.screenWidth / 2,0),
            headingFont = config.headingFont or ui.fonts.orbiteer.xlarge,
            highlightColor = config.highlightColor or Color(0,63,112),
        },
        funcs = {
            displayItem = config.displayItem or defaultFuncs.displayItem,
            getStock = config.getStock or defaultFuncs.getStock,
            getBuyPrice = config.getBuyPrice or defaultFuncs.getBuyPrice,
            getSellPrice = config.getSellPrice or defaultFuncs.getSellPrice,
            onClickBuy = config.onClickBuy or defaultFuncs.onClickBuy,
            onClickSell = config.onClickSell or defaultFuncs.onClickSell,
            buy = config.buy or defaultFuncs.buy,
            bought = config.bought or defaultFuncs.bought,
            sell = config.sell or defaultFuncs.sell,
            sold = config.sold or defaultFuncs.sold,
            initTable = config.initTable or defaultFuncs.initTable,
            renderRow = config.renderRow or defaultFuncs.renderRow,
            sortingFunction = config.sortingFunction or defaultFuncs.sortingFunction,
            onMouseOverItem = config.onMouseOverItem or defaultFuncs.onMouseOverItem,
        },
    }

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
            if self.funcs.displayItem(self, item) then
                table.insert(self.items, item)
            end
        end
    end

    table.sort(self.items, self.funcs.sortingFunction)
end

function MarketWidget:render()
    ui.withStyleVars({WindowPadding = self.style.windowPadding, ItemSpacing = self.style.itemSpacing}, function()
        ui.child("Market##" .. self.id, self.style.size, {"AlwaysUseWindowPadding"}, function()
            if(self.title) then
                ui.withFont(self.style.headingFont.name, self.style.headingFont.size, function()
                    ui.text(self.title)
                end)
            end

            if self.selStart then ui.addRectFilled(self.selStart, self.selEnd, self.style.highlightColor, 0, 0) end

            ui.columns(self.columnCount+1, self.id, false)
            self.funcs.initTable(self)

            ui.text("")
            ui.nextColumn()

            local startPos
            local endPos
            local offset = ui.getWindowPos()
            local selOffset = self.style.itemSpacing.y / 2
            local windowSize = ui.getWindowSize() + offset
            offset.y = offset.y - ui.getScrollY()

            self.selStart = nil
            self.selEnd = nil

            for _, item in pairs(self.items) do
                startPos = ui.getCursorPos() + offset

                self.funcs.renderRow(self, item)

                endPos = ui.getCursorPos()

                ui.text("")
                ui.nextColumn()

                endPos.y = ui.getCursorPos().y
                endPos = offset + endPos

                if ui.isMouseHoveringRect(startPos, endPos, false) and startPos.y < windowSize.y then
                    self.funcs.onMouseOverItem(self, item)

                    self.selStart = startPos
                    self.selEnd = endPos

                    -- center the selection
                    self.selStart.x = self.selStart.x - 4
                    self.selStart.y = self.selStart.y - selOffset
                    self.selEnd.y = self.selEnd.y - selOffset -- selEnd.y points to the beginning of the new row so to center the selection we also move it a bit higher
                end
            end

            ui.columns(1, "", false)

            ui.setNextWindowSize(Vector2(0, 0), "Always")
            ui.popupModal(self.popupMsgId, {"NoTitleBar", "NoResize"}, function ()
                ui.text(self.popupMsg)
                ui.dummy(Vector2((ui.getContentRegion().x - 100) / 2, 0))
                ui.sameLine()
                if ui.button("OK", Vector2(100, 0)) then
                    ui.closeCurrentPopup()
                end
            end)
        end)
    end)
end

return MarketWidget