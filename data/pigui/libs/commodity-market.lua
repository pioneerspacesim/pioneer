-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = import 'Game'
local Lang = import 'Lang'
local Format = import 'Format'
local PiImage = import 'ui/PiImage'
local Equipment = import 'Equipment'

local ui = import 'pigui/pigui.lua'
local pionillium = ui.fonts.pionillium
local orbiteer = ui.fonts.orbiteer
local MarketWidget = import 'pigui/libs/equipment-market.lua'

local l = Lang.GetResource("ui-core")
local colors = ui.theme.colors

local baseCommodityMarketSize = ui.rescaleUI(Vector2(1592, 654), Vector2(1600, 900))
local baseWidgetSizes = {
    rescaleVector = Vector2(1, 1),
	buySellSize = Vector2(128, 48),
	buttonSizeBase = Vector2(64, 48),
	fontSizeLarge = 22.5, -- pionillium.large.size,
    fontSizeXLarge = 27, -- pionillium.xlarge.size,
	iconSize = Vector2(0, 22.5 * 1.5),
	smallButton = Vector2(92, 48),
	bigButton = Vector2(128, 48),
	confirmButtonSize = Vector2(384, 48),
}

local vZero = Vector2(0, 0)
local textColorDefault = Color(255, 255, 255)
local textColorWarning = Color(255, 255, 0)
local textColorError = Color(255, 0, 0)
local tradeMenuFlags = ui.WindowFlags {"AlwaysUseWindowPadding"}
local containerFlags = ui.WindowFlags {"NoScrollbar"}

local CommodityMarketWidget = {}

function CommodityMarketWidget.New(id, title, config)
    local self

    config = config or {}
    config.style = config.style or {}
    config.style.size = config.style.size or Vector2(0,0)
    config.itemTypes = config.itemTypes or { Equipment.cargo }
    config.columnCount = config.columnCount or 5
    config.initTable = config.initTable or function(self)
        ui.setColumnWidth(0, self.style.widgetSizes.buttonSizeBase.x)
        ui.setColumnWidth(1, self.style.size.x / 2 - 50 * self.style.widgetSizes.rescaleVector.x)
    end
    config.renderHeaderRow = config.renderHeaderRow or function(s)
        ui.text('')
        ui.nextColumn()
        ui.text(l.NAME_OBJECT)
        ui.nextColumn()
        ui.text(l.PRICE)
        ui.nextColumn()
        ui.text(l.IN_STOCK)
        ui.nextColumn()
        ui.text(l.CARGO)
        ui.nextColumn()
    end
    config.renderItem = config.renderItem or function(self, item)
        if(self.icons[item.icon_name] == nil) then
            self.icons[item.icon_name] = PiImage.New("icons/goods/".. item.icon_name ..".png")
        end
        self.icons[item.icon_name]:Draw(self.style.widgetSizes.iconSize)
        ui.nextColumn()
        ui.withStyleVars({ItemSpacing = (self.style.itemSpacing / 2)}, function()
            ui.dummy(vZero)
            ui.text(item:GetName())
            ui.nextColumn()
            ui.dummy(vZero)
            ui.text(Format.Money(self.funcs.getBuyPrice(self, item)))
            ui.nextColumn()
            ui.dummy(vZero)
            ui.text(self.funcs.getStock(self, item))
            ui.nextColumn()
            ui.dummy(vZero)
            local n = Game.player:CountEquip(item)
            ui.text(n > 0 and n or '')
        end)
        ui.nextColumn()
    end
    config.canDisplayItem = config.canDisplayItem or function (s, e) return e.purchasable and e:IsValidSlot("cargo") and Game.system:IsCommodityLegal(e) end
    config.onClickItem = config.onClickItem or function(s,e,k)
        s.selectedItem = e
        s.tradeModeBuy = true
        s:ChangeTradeAmount(-s.tradeAmount)
        s:Refresh()
    end

    self = MarketWidget.New(id, title, config)
    self.icons = {}
    self.tradeModeBuy = true;
    self.selectedItem = nil
    self.tradeAmount = 0
    self.tradeText = ''
    self.textColorDefault = Color(255, 255, 255)
    self.textColorWarning = Color(255, 255, 0)
    self.textColorError = Color(255, 0, 0)
    self.tradeTextColor = textColorDefault
    self.style.defaults = {
        windowPadding = self.windowPadding,
        itemSpacing = self.itemSpacing
    }

    setmetatable(self, {
        __index = CommodityMarketWidget,
        class = "UI.CommodityMarketWidget",
    })

    self:SetSize(self.style.size)
    return self
end

function CommodityMarketWidget:ChangeTradeAmount(delta)
    if self.selectedItem == nil then
        return
    end

    --get price of commodity after applying local effects of import/export modifiers
    local price = Game.player:GetDockedWith():GetEquipmentPrice(self.selectedItem)

    --do you have any money?
    local playerCash = Game.player:GetMoney()

    --blank value, needs to be initialized or later on lua will complain
    local stock

    if self.tradeModeBuy then
		price = self.funcs.getBuyPrice(self, self.selectedItem)
        stock = self.funcs.getStock(self, self.selectedItem)
        if stock == 0 then
            self.tradeText = l.NONE_FOR_SALE_IN_THIS_STATION
            self.tradeTextColor = textColorError
            return
        end
        if price > playerCash then
            self.tradeText = l.INSUFFICIENT_FUNDS
            self.tradeTextColor = textColorWarning
            return
        end
    else
		price = self.funcs.getSellPrice(self, self.selectedItem)
        stock = Game.player:CountEquip(self.selectedItem)
    end

    --dont alter tradeamount before checks have been made
    local wantamount = self.tradeAmount + delta

    --how much would the desired amount of merchandise cost?
    local tradecost = wantamount * price

    --we cant trade more units than we have in stock
    if delta > 0 and wantamount > stock then --this line is why stock needs to be initialized up there. its possible to get here without stock being set (?)
        wantamount = stock
    end

    --we dont trade in negative quantities
    if wantamount < 0 then
        wantamount = 0
    end

    --another empty initialized
    self.tradeText = ''
    if self.tradeModeBuy then
        local playerfreecargo = Game.player.totalCargo - Game.player.usedCargo
        if tradecost > playerCash then
            wantamount = math.floor(playerCash / price)
        end
        local tradecargo = self.selectedItem.capabilities.mass * wantamount
        if playerfreecargo < tradecargo then
            wantamount = math.floor(playerfreecargo / self.selectedItem.capabilities.mass)
        end
        self.tradeText = l.MARKET_BUYLINE
    else --mode = sell
        --if market price is negative make sure player wont go below zero credits after the deal
        if (playerCash + tradecost) < 0 then
            wantamount = self.tradeAmount --kludge, ignore the delta unless player has finances to cover the deal
            --if player starts at 0 quantity, presses +100 to "sell" radioactives but only has
            --enough credits to sell 5, this kludge will ignore the +100 completely
            --todo: change amount to 5 instead
        end
        self.tradeText = l.MARKET_SELLINE
    end
    --wantamount is now checked and modified to a safe bounded amount
    self.tradeAmount = wantamount

    --current cost of market order if user confirms the deal
    tradecost = self.tradeAmount * price

    --its possible to get to this line without tradetext being initialized unless done 30 rows up
    self.tradeText = string.interp(self.tradeText,{ amount = string.format("%d", self.tradeAmount), price = Format.Money(tradecost)})
    self.tradeTextColor = self.textColorDefault
end

--player clicked confirm purchase button
function CommodityMarketWidget:DoBuy()
    if not self.funcs.onClickBuy(self, self.selectedItem) then return end

    local price = self.funcs.getBuyPrice(self, self.selectedItem)
    local stock = self.funcs.getStock(self, self.selectedItem)
    local playerfreecargo = Game.player.totalCargo - Game.player.usedCargo
    local orderAmount = price * self.tradeAmount

    --check cash (should never happen since trade amount buttons wont let it happen)
    if orderAmount > Game.player:GetMoney() then
        self.popup.msg = l.YOU_NOT_ENOUGH_MONEY
        self.popup:open()
        return
    end

    --check stock
    if self.tradeAmount > stock then
        self.popup.msg = l.ITEM_IS_OUT_OF_STOCK
        self.popup:open()
        return
    end

    --check cargo limit
    local tradecargo = self.selectedItem.capabilities.mass * self.tradeAmount
    if playerfreecargo < tradecargo then
        self.popup.msg = l.SHIP_IS_FULLY_LADEN
        self.popup:open()
        return
    end

    --all checks passed
    assert(Game.player:AddEquip(self.selectedItem, self.tradeAmount, "cargo") == self.tradeAmount)
    Game.player:AddMoney(-orderAmount) --grab the money
    self.funcs.bought(self, self.selectedItem, self.tradeAmount)
    self:ChangeTradeAmount(-self.tradeAmount) --reset the trade amount

    --update market rows
    self.tradeAmount = 0;
    self:ChangeTradeAmount(0) --update trade amount text
    self:Refresh() --rows needs to be recalculated since now the amounts in stock have changed
end

--player clicked the confirm sale button
function CommodityMarketWidget:DoSell()
    if not self.funcs.onClickSell(self, self.selectedItem) then return end

    local price = self.funcs.getSellPrice(self, self.selectedItem)
    local orderamount = price * self.tradeAmount

    --if commodity price is negative (radioactives, garbage), player needs to have enough cash
    Game.player:RemoveEquip(self.selectedItem, self.tradeAmount, "cargo")
    Game.player:AddMoney(orderamount) --grab the money
	self.funcs.sold(self, self.selectedItem, self.tradeAmount)
    self:ChangeTradeAmount(-self.tradeAmount) --reset the trade amount

    --if player sold all his cargo, switch to buy panel
    if Game.player:CountEquip(self.selectedItem) == 0 then self.tradeModeBuy = true end

    --update market rows
    self.tradeAmount = 0;
    self:ChangeTradeAmount(0) --update trade amount text
    self:Refresh() --rows needs to be recalculated since now the amounts in stock have changed
end

function CommodityMarketWidget:TradeMenu()
    if(self.selectedItem) then
        ui.withStyleVars({WindowPadding = self.style.windowPadding}, function()
            ui.child(self.id .. "TradeMenu", vZero, tradeMenuFlags, function()
                if(ui.coloredSelectedButton(l.BUY, self.style.widgetSizes.buySellSize, self.tradeModeBuy, colors.buttonBlue, nil, true)) then
                    self.tradeModeBuy = true
                    self:ChangeTradeAmount(-self.tradeAmount)
                end
                ui.sameLine()
                if(ui.coloredSelectedButton(l.SELL, self.style.widgetSizes.buySellSize, not self.tradeModeBuy, colors.buttonBlue, nil, true)) then
                    self.tradeModeBuy = false
                    self:ChangeTradeAmount(-self.tradeAmount)
                end

                ui.text('')
                local bottomHalf = ui.getCursorPos()
                bottomHalf.y = bottomHalf.y + ui.getContentRegion().y/1.65
                if(self.icons[self.selectedItem.icon_name] == nil) then
                    self.icons[self.selectedItem.icon_name] = PiImage.New("icons/goods/".. self.selectedItem.icon_name ..".png")
                end

                ui.columns(2, "tradeMenuItemTitle", false)
                ui.setColumnWidth(0, self.style.widgetSizes.buttonSizeBase.x)
                self.icons[self.selectedItem.icon_name]:Draw(self.style.widgetSizes.iconSize)
                ui.nextColumn()
                ui.withStyleVars({ItemSpacing = self.style.itemSpacing/2}, function()
                    ui.withFont(orbiteer.xlarge.name, self.style.widgetSizes.fontSizeLarge, function()
                        ui.dummy(vZero)
                        ui.text(self.selectedItem:GetName())
                    end)
                end)
                ui.columns(1, "", false)
                ui.text('')

                ui.textWrapped(self.selectedItem:GetDescription())

                ui.setCursorPos(bottomHalf)
                if ui.coloredSelectedButton("-100", self.style.widgetSizes.smallButton, false, colors.buttonBlue, nil, true) then self:ChangeTradeAmount(-100) end
                ui.sameLine()
                if ui.coloredSelectedButton("-10", self.style.widgetSizes.smallButton, false, colors.buttonBlue, nil, true) then self:ChangeTradeAmount(-10) end
                ui.sameLine()
                if ui.coloredSelectedButton("-1", self.style.widgetSizes.smallButton, false, colors.buttonBlue, nil, true) then self:ChangeTradeAmount(-1) end
                ui.sameLine()
                if ui.coloredSelectedButton(l.RESET, self.style.widgetSizes.bigButton, false, colors.buttonBlue, nil, true) then self:ChangeTradeAmount(-self.tradeAmount) end
                ui.sameLine()
                if ui.coloredSelectedButton("+1", self.style.widgetSizes.smallButton, false, colors.buttonBlue, nil, true) then self:ChangeTradeAmount(1) end
                ui.sameLine()
                if ui.coloredSelectedButton("+10", self.style.widgetSizes.smallButton, false, colors.buttonBlue, nil, true) then self:ChangeTradeAmount(10) end
                ui.sameLine()
                if ui.coloredSelectedButton("+100", self.style.widgetSizes.smallButton, false, colors.buttonBlue, nil, true) then self:ChangeTradeAmount(100) end

                ui.dummy(self.style.itemSpacing/2)
                ui.withStyleColors({["Text"] = self.tradeTextColor }, function()
                    ui.withFont(pionillium.xlarge.name, self.style.widgetSizes.fontSizeLarge, function()
                        ui.text(self.tradeText)
                    end)
                end)

                ui.setCursorPos(ui.getCursorPos() + Vector2(0, ui.getContentRegion().y - self.style.widgetSizes.confirmButtonSize.y))
                ui.withFont(orbiteer.xlarge.name, self.style.widgetSizes.fontSizeXLarge, function()
                    if ui.coloredSelectedButton(self.tradeModeBuy and l.CONFIRM_PURCHASE or l.CONFIRM_SALE, self.style.widgetSizes.confirmButtonSize, false, colors.buttonBlue, nil, true) then
                        if self.tradeModeBuy then self:DoBuy()
                        else self:DoSell() end
                    end
                end)
            end)
        end)
    end
end

function CommodityMarketWidget:SetSize(size)
    size.x = math.max(size.x, 100)
    size.y = math.max(size.y, 100)
    if self.style.widgetSize ~= size then
        self.style.widgetSize = size
        self.style.size = Vector2(size.x / 2, size.y)

        self.style.widgetSizes = ui.rescaleUI(
                baseWidgetSizes,
                Vector2(1592, 654), --Size the Commodity Market was scaled to during design
                true,
                size
        )

        self.windowPadding = ui.rescaleUI(
                self.style.defaults.windowPadding,
                Vector2(1592, 654), --Size the Commodity Market was scaled to during design
                true,
                size
        )

        self.itemSpacing = ui.rescaleUI(
                self.style.defaults.itemSpacing,
                Vector2(1592, 654), --Size the Commodity Market was scaled to during design
                true,
                size
        )
    end
end

function CommodityMarketWidget:Refresh()
    MarketWidget.refresh(self)
end

function CommodityMarketWidget:Render(size)
    self:SetSize(size or ui.getContentRegion())

    ui.withFont(pionillium.large.name, self.style.widgetSizes.fontSizeLarge, function()
        ui.withStyleVars({WindowPadding = vZero, ItemSpacing = self.style.itemSpacing}, function()
            ui.child(self.id .. "Container", self.style.widgetSize, containerFlags, function()
                MarketWidget.render(self)
                ui.sameLine()
                self:TradeMenu()
            end)
        end)
    end)
end

return CommodityMarketWidget
