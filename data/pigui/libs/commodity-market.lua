-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local Lang = require 'Lang'
local Format = require 'Format'
local Commodities = require 'Commodities'
local Economy     = require 'Economy'

local ui = require 'pigui'
local pionillium = ui.fonts.pionillium
local orbiteer = ui.fonts.orbiteer
local PiImage = require 'pigui.libs.image'

local ModalWindow = require 'pigui.libs.modal-win'
local TableWidget = require 'pigui.libs.table'
local EconView = require 'pigui.modules.system-econ-view'

local l = Lang.GetResource("ui-core")

local Vector2 = _G.Vector2
local Color = _G.Color

local baseWidgetSizes = {
	rescaleVector = Vector2(1, 1),
	buySellSize = Vector2(160, 36),
	fontSizeLarge = 22.5, -- pionillium.large.size,
	fontSizeXLarge = 27, -- pionillium.xlarge.size,
	iconSize = Vector2(0, 22.5 * 1.5),
	smallButton = Vector2(92, 48),
	bigButton = Vector2(128, 48),
	confirmButtonSize = Vector2(384, 48),
	windowGutter = 18
}

local commodityIconSize = Vector2(38.0, 32.0) -- png icons, native resolution

local vZero = Vector2(0, 0)
local textColorDefault = Color(255, 255, 255)
local textColorWarning = Color(255, 255, 0)
local textColorError = Color(255, 0, 0)

local colorVariant = {
	[true] = ui.theme.buttonColors.selected,
	[false] = ui.theme.buttonColors.default
}

local function get_pricemod(itemType, price)
	return (price / itemType.price - 1) * 100
end

local CommodityMarketWidget = {}

function CommodityMarketWidget.New(id, title, config)
	config = config or {}
	config.style = config.style or {}
	config.style.size = config.style.size or Vector2(0,0)
	config.itemTypes = config.itemTypes or { Commodities }
	config.columnCount = config.columnCount or 6

	config.initTable = config.initTable or function(self)
		ui.setColumnWidth(0, commodityIconSize.x + ui.getItemSpacing().x)
		ui.setColumnWidth(1, self.style.size.x / 2.2 - 50 * self.style.widgetSizes.rescaleVector.x)
	end

	config.renderHeaderRow = config.renderHeaderRow or function(_)
		ui.text('')
		ui.nextColumn()
		ui.text(l.NAME_OBJECT)
		ui.nextColumn()
		ui.text(l.PRICE)
		ui.nextColumn()
		ui.text(l.IN_STOCK)
		ui.nextColumn()
		ui.text(l.DEMAND)
		ui.nextColumn()
		ui.text(l.CARGO)
		ui.nextColumn()
	end

	config.renderItem = config.renderItem or function(self, item)
		if(self.icons[item.icon_name] == nil) then
			self.icons[item.icon_name] = PiImage.New("icons/goods/".. item.icon_name ..".png")
		end
		self.icons[item.icon_name]:Draw(commodityIconSize)
		ui.nextColumn()

		ui.withStyleVars({ItemSpacing = (self.style.itemSpacing / 2)}, function()
			local price = self.station:GetCommodityPrice(item)

			ui.dummy(vZero)
			ui.text(item:GetName())

			local pricemod = get_pricemod(item, price) - Game.system:GetCommodityBasePriceAlterations(item.name)
			local cls = EconView.ClassifyPrice(pricemod)

			if cls then
				ui.sameLine()
				ui.addCursorPos(Vector2(ui.getContentRegion().x - ui.getTextLineHeightWithSpacing(), 0))

				ui.icon(cls[1], Vector2(ui.getTextLineHeight()), cls[2])
			end

			ui.nextColumn()
			ui.dummy(vZero)
			ui.text(Format.Money(price))
			ui.nextColumn()
			ui.dummy(vZero)
			ui.text(config.getStock(self, item))
			ui.nextColumn()
			ui.dummy(vZero)
			ui.text(config.getDemand(self, item))
			ui.nextColumn()
			ui.dummy(vZero)
			local n = self.cargoMgr:CountCommodity(item)
			ui.text(n > 0 and n or '')
		end)
		ui.nextColumn()
	end

	config.canDisplayItem = config.canDisplayItem or function (self, commodity)
		return commodity.purchasable and Game.system:IsCommodityLegal(commodity.name)
	end

	-- how much of this item do we have in stock?
    config.getStock = config.getStock or function (self, commodity)
        return self.station:GetCommodityStock(commodity)
    end

	config.getDemand = function (self, commodity)
		return self.station:GetCommodityDemand(commodity)
	end

    -- what do we charge for this item if we are buying
    config.getBuyPrice = config.getBuyPrice or function (self, commodity)
		local price = self.station:GetCommodityPrice(commodity)
		return price * (1.0 + math.sign(price) * Economy.TradeFeeSplit * 0.01)
    end

    -- what do we get for this item if we are selling
    config.getSellPrice = config.getSellPrice or function (self, commodity)
        local price = self.station:GetCommodityPrice(commodity)
		return price * (1.0 - math.sign(price) * Economy.TradeFeeSplit * 0.01)
    end

	config.bought = function (self, commodity, tradeamount)
		local count = tradeamount or 1
        self.station:AddCommodityStock(commodity, -count)
    end

    config.sold = function (self, commodity, tradeamount)
		local count = tradeamount or 1
        self.station:AddCommodityStock(commodity, count)
    end

	config.onClickItem = config.onClickItem or function(s,e,_)
		s.selectedItem = e
		s.tradeModeBuy = true
		s:ChangeTradeAmount(-s.tradeAmount)
		s:Refresh()
	end

	config.sortingFunction = config.sortingFunction or function (c1, c2)
		return c1:GetName() < c2:GetName()
	end

	local self = TableWidget.New(id, title, config)

	self.popup = config.popup or ModalWindow.New('popupMsg' .. id, function()
        ui.text(self.popup.msg)
        ui.dummy(Vector2((ui.getContentRegion().x - 100) / 2, 0))
        ui.sameLine()
        if ui.button("OK", Vector2(100, 0)) then
            self.popup:close()
        end
    end)

	self.icons = {}
	self.tradeModeBuy = true
	self.selectedItem = nil
	self.tradeAmount = 0
	self.tradeText = ''
	self.textColorDefault = Color(255, 255, 255)
	self.textColorWarning = Color(255, 255, 0)
	self.textColorError = Color(255, 0, 0)
	self.tradeTextColor = textColorDefault

	self.cargoMgr = nil
	self.station = nil

	self.funcs.getStock = config.getStock
	self.funcs.getDemand = config.getDemand
	self.funcs.getBuyPrice = config.getBuyPrice
	self.funcs.getSellPrice = config.getSellPrice
	self.funcs.bought = config.bought
	self.funcs.sold = config.sold

	self.style.defaults = {
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
	local price = 0

	--do you have any money?
	local playerCash = Game.player:GetMoney()

	--blank value, needs to be initialized or later on lua will complain
	local stock, demand

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
		stock = self.cargoMgr:CountCommodity(self.selectedItem)
		demand = self.station:GetCommodityDemand(self.selectedItem)
	end

	--we cant trade more units than we have in stock
	--we dont trade in negative quantities
	local wantamount = math.clamp(self.tradeAmount + delta, 0, stock)

	--how much would the desired amount of merchandise cost?
	local tradecost = wantamount * price

	--another empty initialized
	self.tradeText = ''
	if self.tradeModeBuy then
		-- TODO: use a volume-based metric rather than a mass-based metric
		local playerfreecargo = self.cargoMgr:GetFreeSpace()

		if tradecost > playerCash then
			wantamount =  math.min(wantamount, math.floor(playerCash / price))
		end

		local tradecargo = (self.selectedItem.mass or 1) * wantamount
		if playerfreecargo < tradecargo then
			wantamount = math.min(wantamount, math.floor(playerfreecargo / self.selectedItem.mass))
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
		wantamount = math.min(wantamount, demand)

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
	local price = self.funcs.getBuyPrice(self, self.selectedItem)
	local stock = self.funcs.getStock(self, self.selectedItem)
	local playerfreecargo = self.cargoMgr:GetFreeSpace()
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
	local tradecargo = (self.selectedItem.mass or 1) * self.tradeAmount
	if playerfreecargo < tradecargo then
		self.popup.msg = l.SHIP_IS_FULLY_LADEN
		self.popup:open()
		return
	end

	--all checks passed
	assert(self.cargoMgr:AddCommodity(self.selectedItem, self.tradeAmount))
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
	local price = self.funcs.getSellPrice(self, self.selectedItem)
	--if commodity price is negative (radioactives, garbage), player needs to have enough cash
	local orderamount = price * self.tradeAmount

	assert(self.cargoMgr:RemoveCommodity(self.selectedItem, self.tradeAmount) == self.tradeAmount)
	Game.player:AddMoney(orderamount) --grab the money

	self.funcs.sold(self, self.selectedItem, self.tradeAmount)
	self:ChangeTradeAmount(-self.tradeAmount) --reset the trade amount

	--if player sold all of this cargo, switch to buy panel
	if self.cargoMgr:CountCommodity(self.selectedItem) == 0 then self.tradeModeBuy = true end

	--update market rows
	self.tradeAmount = 0;
	self:ChangeTradeAmount(0) --update trade amount text
	self:Refresh() --rows needs to be recalculated since now the amounts in stock have changed
end

function CommodityMarketWidget:TradeMenu()
	if(self.selectedItem) then
		ui.child(self.id .. "TradeMenu", vZero, function()

			ui.withFont(pionillium.heading, function()
				-- center the buy/sell switch buttons
				ui.addCursorPos(Vector2(ui.getContentRegion().x * 0.5 - self.style.widgetSizes.buySellSize.x - ui.getItemSpacing().x, 0))

				if ui.button(l.BUY, self.style.widgetSizes.buySellSize, colorVariant[self.tradeModeBuy]) then
					self.tradeModeBuy = true
					self:ChangeTradeAmount(-self.tradeAmount)
				end

				ui.sameLine()

				if ui.button(l.SELL, self.style.widgetSizes.buySellSize, colorVariant[not self.tradeModeBuy]) then
					self.tradeModeBuy = false
					self:ChangeTradeAmount(-self.tradeAmount)
				end
			end)

			ui.newLine()

			local bottomHalf = ui.getCursorPos()
			bottomHalf.y = bottomHalf.y + ui.getContentRegion().y/1.65
			if(self.icons[self.selectedItem.icon_name] == nil) then
				self.icons[self.selectedItem.icon_name] = PiImage.New("icons/goods/".. self.selectedItem.icon_name ..".png")
			end

			ui.columns(2, "tradeMenuItemTitle", false)
			ui.setColumnWidth(0, commodityIconSize.x + ui.getItemSpacing().x)
			self.icons[self.selectedItem.icon_name]:Draw(commodityIconSize)
			ui.nextColumn()
			ui.withStyleVars({ItemSpacing = self.style.itemSpacing/2}, function()
				ui.withFont(orbiteer.heading, function()
					-- align the height to the center relative to the icon
					ui.alignTextToLineHeight(commodityIconSize.y)
					ui.text(self.selectedItem:GetName())
				end)
			end)
			ui.columns(1, "", false)
			ui.newLine()

			ui.withFont(pionillium.heading, function()
				local price = self.station:GetCommodityPrice(self.selectedItem)
				local pricemod = get_pricemod(self.selectedItem, price)
				local cls = EconView.ClassifyPrice(pricemod)

				if cls and self.tradeComputer > 0 then
					ui.text(l.INTERSTELLAR_TRADE_AVG)
					ui.sameLine()

					ui.icon(cls[1], Vector2(ui.getTextLineHeight()), cls[2])
					ui.sameLine()
					ui.text(cls[3])
					ui.spacing()
				end
			end)

			ui.withFont(pionillium.body, function()
				ui.textWrapped(self.selectedItem:GetDescription())
			end)

			ui.setCursorPos(bottomHalf)
			if ui.button("-100", self.style.widgetSizes.smallButton) then self:ChangeTradeAmount(-100) end
			ui.sameLine()
			if ui.button("-10", self.style.widgetSizes.smallButton) then self:ChangeTradeAmount(-10) end
			ui.sameLine()
			if ui.button("-1", self.style.widgetSizes.smallButton) then self:ChangeTradeAmount(-1) end
			ui.sameLine()
			if ui.button(l.RESET, self.style.widgetSizes.bigButton) then self:ChangeTradeAmount(-self.tradeAmount) end
			ui.sameLine()
			if ui.button("+1", self.style.widgetSizes.smallButton) then self:ChangeTradeAmount(1) end
			ui.sameLine()
			if ui.button("+10", self.style.widgetSizes.smallButton) then self:ChangeTradeAmount(10) end
			ui.sameLine()
			if ui.button("+100", self.style.widgetSizes.smallButton) then self:ChangeTradeAmount(100) end

			ui.dummy(self.style.itemSpacing/2)
			ui.withStyleColors({["Text"] = self.tradeTextColor }, function()
				ui.withFont(pionillium.heading, function()
					ui.text(self.tradeText)
				end)
			end)

			ui.addCursorPos(Vector2(0, ui.getContentRegion().y - self.style.widgetSizes.confirmButtonSize.y))
			ui.withFont(orbiteer.heading, function()
				if ui.button(self.tradeModeBuy and l.CONFIRM_PURCHASE or l.CONFIRM_SALE, self.style.widgetSizes.confirmButtonSize) then
					if self.tradeModeBuy then self:DoBuy()
					else self:DoSell() end
				end
			end)
		end)
	else
		ui.newLine()
	end
end

function CommodityMarketWidget:SetSize(size)
	size = Vector2(math.max(size.x, 100), math.max(size.y, 100))
	if self.style.widgetSize ~= size then
		self.style.widgetSize = size
		self.style.size = Vector2(size.x / 2, size.y)

		self.style.widgetSizes = ui.rescaleUI(
			baseWidgetSizes,
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
	self.items = {}

	for k, comm in pairs(Commodities) do
		if self.funcs.canDisplayItem(self, comm) then
			table.insert(self.items, comm)
		end
	end

	table.sort(self.items, self.funcs.sortingFunction)

	---@type CargoManager
	self.cargoMgr = Game.player:GetComponent('CargoManager')
	self.station = Game.player:GetDockedWith()
	self.tradeComputer = Game.player["trade_computer_cap"] or 0
end

function CommodityMarketWidget:Render(size)
	self:SetSize(size or ui.getContentRegion())

	ui.withStyleVars({ItemSpacing = self.style.itemSpacing}, function()
		ui.withFont(pionillium.heading, function()
			TableWidget.render(self)
		end)
		ui.sameLine(0, self.style.widgetSizes.windowGutter)
		self:TradeMenu()
	end)
end

return CommodityMarketWidget
