-- Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Lang = import 'Lang'
local Game = import 'Game'
local Format = import 'Format'
local Equipment = import 'Equipment'
local StationView = import 'pigui/views/station-view'
local Market = import 'pigui/libs/market.lua'
local PiImage = import 'ui/PiImage'

local ui = import 'pigui/pigui.lua'
local pionillium = ui.fonts.pionillium
local orbiteer = ui.fonts.orbiteer
local l = Lang.GetResource("ui-core")
local colors = ui.theme.colors

local vZero = Vector2(0,0)
local rescaleVector = ui.rescaleUI(Vector2(1, 1), Vector2(1600, 900), true)
local widgetSizes = ui.rescaleUI({
	buySellSize = Vector2(128, 48),
	buttonSizeBase = Vector2(64, 48),
	iconSize = Vector2(0, pionillium.large.size * 1.5),
	smallButton = Vector2(92, 48),
	bigButton = Vector2(128, 48),
	confirmButtonSize = Vector2(384, 48),
}, Vector2(1600, 900))

local commodityMarket
local icons = {}
local tradeModeBuy = true;
local selectedItem
local tradeAmount = 0
local tradeText = ''
local textColorDefault = Color(255, 255, 255)
local textColorWarning = Color(255, 255, 0)
local textColorError = Color(255, 0, 0)
local tradeTextColor = textColorDefault

local popupId = "commodityPopup"
local popupMsg = "commodityPopup"

-- calls to this function alter traded amount up or down, not absolute values
local changeTradeAmount = function (delta)

	--get price of commodity after applying local effects of import/export modifiers
	local price = Game.player:GetDockedWith():GetEquipmentPrice(selectedItem)

	--do you have any money?
	local playerCash = Game.player:GetMoney()

	--blank value, needs to be initialized or later on lua will complain
	local stock

	if tradeModeBuy then
		stock = Game.player:GetDockedWith():GetEquipmentStock(selectedItem)
		if stock == 0 then
			tradeText = l.NONE_FOR_SALE_IN_THIS_STATION
			tradeTextColor = textColorError
			return
		end
		if price > playerCash then
			tradeText = l.INSUFFICIENT_FUNDS
			tradeTextColor = textColorWarning
			return
		end
	else
		stock = Game.player:CountEquip(selectedItem)
	end

	--dont alter tradeamount before checks have been made
	local wantamount = tradeAmount + delta

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
	tradeText = ''
	if tradeModeBuy then
		local playerfreecargo = Game.player.totalCargo - Game.player.usedCargo
		if tradecost > playerCash then
			wantamount = math.floor(playerCash / price)
		end
		local tradecargo = selectedItem.capabilities.mass * wantamount
		if playerfreecargo < tradecargo then
			wantamount = math.floor(playerfreecargo / selectedItem.capabilities.mass)
		end
		tradeText = l.MARKET_BUYLINE
	else --mode = sell
		--if market price is negative make sure player wont go below zero credits after the deal
		if (playerCash + tradecost) < 0 then
			wantamount = tradeAmount --kludge, ignore the delta unless player has finances to cover the deal
			--if player starts at 0 quantity, presses +100 to "sell" radioactives but only has
			--enough credits to sell 5, this kludge will ignore the +100 completely
			--todo: change amount to 5 instead
		end
		tradeText = l.MARKET_SELLINE
	end
	--wantamount is now checked and modified to a safe bounded amount
	tradeAmount = wantamount

	--current cost of market order if user confirms the deal
	tradecost = tradeAmount * price

	--its possible to get to this line without tradetext being initialized unless done 30 rows up
	tradeText = string.interp(tradeText,{ amount = string.format("%d", tradeAmount), price = Format.Money(tradecost)})
	tradeTextColor = textColorDefault
end

--player clicked confirm purchase button
local doBuy = function ()
	local price = Game.player:GetDockedWith():GetEquipmentPrice(selectedItem)
	local stock = Game.player:GetDockedWith():GetEquipmentStock(selectedItem)
	local playerfreecargo = Game.player.totalCargo - Game.player.usedCargo
	local orderAmount = price * tradeAmount

	--check cash (should never happen since trade amount buttons wont let it happen)
	if orderAmount > Game.player:GetMoney() then
		popupMsg = l.YOU_NOT_ENOUGH_MONEY
		ui.openPopup(popupId)
		return
	end

	--check stock
	if tradeAmount > stock then
		popupMsg = l.ITEM_IS_OUT_OF_STOCK
		ui.openPopup(popupId)
		return
	end

	--check cargo limit
	local tradecargo = selectedItem.capabilities.mass * tradeAmount
	if playerfreecargo < tradecargo then
		popupMsg = l.SHIP_IS_FULLY_LADEN
		ui.openPopup(popupId)
		return
	end

	--all checks passed
	assert(Game.player:AddEquip(selectedItem, tradeAmount, "cargo") == tradeAmount)
	Game.player:AddMoney(-orderAmount) --grab the money
	Game.player:GetDockedWith():AddEquipmentStock(selectedItem, -tradeAmount)
	changeTradeAmount(-tradeAmount) --reset the trade amount

	--update market rows
	tradeAmount = 0;
	changeTradeAmount(0) --update trade amount text
	commodityMarket:refresh() --rows needs to be recalculated since now the amounts in stock have changed
end

--player clicked the confirm sale button
local doSell = function ()
	local price = Game.player:GetDockedWith():GetEquipmentPrice(selectedItem)
	local orderamount = price * tradeAmount

	--if commodity price is negative (radioactives, garbage), player needs to have enough cash
	Game.player:RemoveEquip(selectedItem, tradeAmount, "cargo")
	Game.player:AddMoney(orderamount) --grab the money
	Game.player:GetDockedWith():AddEquipmentStock(selectedItem, tradeAmount)
	changeTradeAmount(-tradeAmount) --reset the trade amount

	--if player sold all his cargo, switch to buy panel
	if Game.player:CountEquip(selectedItem) == 0 then tradeModeBuy = true end

	--update market rows
	tradeAmount = 0;
	changeTradeAmount(0) --update trade amount text
	commodityMarket:refresh() --rows needs to be recalculated since now the amounts in stock have changed
end


local tradeMenu = function()
	if(selectedItem) then
		ui.withStyleVars({WindowPadding = commodityMarket.style.windowPadding, ItemSpacing = commodityMarket.style.itemSpacing}, function()
			ui.child("TradeMenu", Vector2(ui.screenWidth / 2,0), {"AlwaysUseWindowPadding"}, function()
				if(ui.coloredSelectedButton(l.BUY, widgetSizes.buySellSize, tradeModeBuy, colors.buttonBlue, nil, true)) then
					tradeModeBuy = true
					changeTradeAmount(-tradeAmount)
				end
				ui.sameLine()
				if(ui.coloredSelectedButton(l.SELL, widgetSizes.buySellSize, not tradeModeBuy, colors.buttonBlue, nil, true)) then
					tradeModeBuy = false
					changeTradeAmount(-tradeAmount)
				end

				ui.text('')
				local bottomHalf = ui.getCursorPos()
				bottomHalf.y = bottomHalf.y + ui.getContentRegion().y/1.65
				if(icons[selectedItem.icon_name] == nil) then
					icons[selectedItem.icon_name] = PiImage.New("icons/goods/".. selectedItem.icon_name ..".png")
				end

				ui.columns(2, "tradeMenuItemTitle", false)
				ui.setColumnWidth(0, widgetSizes.buttonSizeBase.x)
				icons[selectedItem.icon_name]:Draw(widgetSizes.iconSize)
				ui.nextColumn()
				ui.withStyleVars({ItemSpacing = commodityMarket.style.itemSpacing/2}, function()
					ui.withFont(orbiteer.xlarge.name, orbiteer.xlarge.size, function()
						ui.dummy(vZero)
						ui.text(selectedItem:GetName())
					end)
				end)
				ui.columns(1, "", false)
				ui.text('')

				ui.textWrapped(selectedItem:GetDescription())

				ui.setCursorPos(bottomHalf)
				if ui.coloredSelectedButton("-100", widgetSizes.smallButton, false, colors.buttonBlue, nil, true) then changeTradeAmount(-100) end
				ui.sameLine()
				if ui.coloredSelectedButton("-10", widgetSizes.smallButton, false, colors.buttonBlue, nil, true) then changeTradeAmount(-10) end
				ui.sameLine()
				if ui.coloredSelectedButton("-1", widgetSizes.smallButton, false, colors.buttonBlue, nil, true) then changeTradeAmount(-1) end
				ui.sameLine()
				if ui.coloredSelectedButton(l.RESET, widgetSizes.bigButton, false, colors.buttonBlue, nil, true) then changeTradeAmount(-tradeAmount) end
				ui.sameLine()
				if ui.coloredSelectedButton("+1", widgetSizes.smallButton, false, colors.buttonBlue, nil, true) then changeTradeAmount(1) end
				ui.sameLine()
				if ui.coloredSelectedButton("+10", widgetSizes.smallButton, false, colors.buttonBlue, nil, true) then changeTradeAmount(10) end
				ui.sameLine()
				if ui.coloredSelectedButton("+100", widgetSizes.smallButton, false, colors.buttonBlue, nil, true) then changeTradeAmount(100) end

				ui.dummy(commodityMarket.style.itemSpacing/2)
				ui.withStyleColors({["Text"] = tradeTextColor }, function()
					ui.withFont(pionillium.xlarge.name, pionillium.xlarge.size, function()
						ui.text(tradeText)
					end)
				end)

				ui.setCursorPos(ui.getCursorPos() - Vector2(0, widgetSizes.confirmButtonSize.y - commodityMarket.style.windowPadding.y - ui.getContentRegion().y))
				ui.withFont(orbiteer.xlarge.name, orbiteer.xlarge.size, function()
					if ui.coloredSelectedButton(tradeModeBuy and l.CONFIRM_PURCHASE or l.CONFIRM_SALE, widgetSizes.confirmButtonSize, false, colors.buttonBlue, nil, true) then
						if tradeModeBuy then doBuy()
						else doSell() end
					end
				end)

				ui.setNextWindowSize(Vector2(0, 0), "Always")
				ui.popupModal(popupId, {"NoTitleBar", "NoResize"}, function ()
					ui.text(popupMsg)
					ui.dummy(Vector2((ui.getContentRegion().x - 100 * rescaleVector.x) / 2, 0))
					ui.sameLine()
					if ui.button("OK", Vector2(100 * widgetSizes.rescaleVector.x, 0)) then
						ui.closeCurrentPopup()
					end
				end)
			end)
		end)
	end
end

commodityMarket = Market.New("EquipmentMarket", false, {
	itemTypes = { Equipment.cargo },
	columnCount = 5,
	initTable = function(self)
		ui.setColumnWidth(0, widgetSizes.buttonSizeBase.x)
		ui.setColumnWidth(1, self.style.size.x / 2 - 50 * rescaleVector.x)

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
	end,
	renderRow = function(self, item)
		if(icons[item.icon_name] == nil) then
			icons[item.icon_name] = PiImage.New("icons/goods/".. item.icon_name ..".png")
		end
		icons[item.icon_name]:Draw(widgetSizes.iconSize)
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
	end,

	displayItem = function (s, e) return e.purchasable and e:IsValidSlot("cargo") and Game.system:IsCommodityLegal(e) end,
	onMouseOverItem = function(s, e)
		if ui.isMouseClicked(0) and s.funcs.onClickBuy(e) then
			selectedItem = e
			tradeModeBuy = true
			changeTradeAmount(-tradeAmount)
			s:refresh()
		end
	end
})

local function drawCommoditytView()

	ui.withFont(pionillium.large.name, pionillium.large.size, function()
		ui.child("commodityMarketContainer", Vector2(0, ui.getContentRegion().y - StationView.style.height), {}, function()
			commodityMarket:render()
			ui.sameLine()
			tradeMenu()
		end)

		StationView:shipSummary()
	end)
end

StationView:registerView({
	id = "commodityMarket",
	name = l.COMMODITY_MARKET,
	icon = ui.theme.icons.market,
	showView = true,
	draw = drawCommoditytView,
	refresh = function()
		commodityMarket:refresh()
	end,
})
