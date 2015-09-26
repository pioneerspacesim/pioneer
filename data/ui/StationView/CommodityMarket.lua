-- Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Lang = import("Lang")
local Game = import("Game")
local Format = import("Format")
local Equipment = import("Equipment")
local SmallLabeledButton = import("ui/SmallLabeledButton")

local EquipmentTableWidgets = import("EquipmentTableWidgets")
local ui = Engine.ui
local l = Lang.GetResource("ui-core")

local marketColumnValue = {
	icon  = function (e) return e.icon_name and ui:Image("icons/goods/"..e.icon_name..".png") or "" end,
	name  = function (e) return e:GetName() end,
	price = function (e) return Format.Money(Game.player:GetDockedWith():GetEquipmentPrice(e)) end,
	stock = function (e) return Game.player:GetDockedWith():GetEquipmentStock(e) end,
	cargo = function (e)
		local n = Game.player:CountEquip(e)
		if n == 0 then
			n = ''
		end
		return n
	end,
}

local commodityMarket = function (args)

	local equipTypes = {}
	for k,e in pairs(Equipment.cargo) do
		if e.purchasable and e:IsValidSlot("cargo") and Game.system:IsCommodityLegal(e) then
			table.insert(equipTypes, e)
		end
	end

	local sortingFunction = function(e1,e2)
		return e1:GetName() < e2:GetName()        -- cargo sorted on translated name
	end
	table.sort(equipTypes, sortingFunction)

	local marketTable =
		ui:Table()
			:SetRowSpacing(5)
			:SetColumnSpacing(10)
			:SetHeadingRow({'', l.NAME_OBJECT, l.PRICE, l.IN_STOCK, ui:Margin(32,"RIGHT",l.CARGO) })
			:SetHeadingFont("LARGE")
			:SetRowAlignment("CENTER")
			:SetMouseEnabled(true)

	local commodityTrade =
		ui:Expand("VERTICAL")

	local fillMarketTable = function()
		marketTable:ClearRows()
		local rowCommodity = {}
		for i,e in ipairs(equipTypes) do
			marketTable:AddRow({
				marketColumnValue["icon"](e),
				ui:Expand("HORIZONTAL",marketColumnValue["name"](e)),
				ui:Align("RIGHT", marketColumnValue["price"](e)),
				ui:Align("RIGHT", marketColumnValue["stock"](e)),
				ui:Align("RIGHT", ui:Margin(32,"RIGHT",marketColumnValue["cargo"](e))),
			})
			table.insert(rowCommodity, e)
		end
		return rowCommodity
	end
	local marketRows = fillMarketTable()

	local trade_mode_buy = 22
	local trade_mode_sell = 33
	local trade_mode = trade_mode_buy
	local tradeamount = 0;
	local tradecommodity = ''
	local buysell =
		ui:Expand("VERTICAL")
	local tradeflipflop =
		ui:Expand()

	local sub100 = ui:Button("-100")
	local subten = ui:Button("-10")
	local subone = ui:Button("-1")
	local tradereset = ui:Button("Reset")
	local addone = ui:Button("+1")
	local addten = ui:Button("+10")
	local add100 = ui:Button("+100")
	local confirmtradebuy = ui:Button("Confirm purchase"):SetFont("HEADING_LARGE")
	local confirmtradesell = ui:Button("Confirm sale"):SetFont("HEADING_LARGE")
	local nobutton = nil
	local showbuysellbutton = confirmtradebuy
	local sellfromcargo = ui:Button("Sell")
	local buyfrommarket = ui:Button("Buy")

	local commonHeader =
		ui:HBox()
	local commonButtons =
		ui:HBox():PackEnd({
			sub100,
			ui:Margin(16,"LEFT",subten),
			ui:Margin(16,"LEFT",subone),
			ui:Margin(16,"LEFT",tradereset),
			ui:Margin(16,"LEFT",addone),
			ui:Margin(16,"LEFT",addten),
			ui:Margin(16,"LEFT",add100),
		})
	
	local changeTradeamount = function (delta)
		local price = Game.player:GetDockedWith():GetEquipmentPrice(tradecommodity)
		local playercash = Game.player:GetMoney()
		local stock
		if trade_mode == trade_mode_buy then
			stock = Game.player:GetDockedWith():GetEquipmentStock(tradecommodity)
			if stock == 0 then
				buysell:SetInnerWidget(ui:Label("None for sale in this station.")
					:SetFont("LARGE")
					:SetColor({ r = 1.0, g = 0.0, b = 0.0 })
				)
				showbuysellbutton = nobutton
				return
			end
			if price > playercash then
				buysell:SetInnerWidget(ui:Label("Insufficient funds.")
					:SetFont("LARGE")
					:SetColor({ r = 1.0, g = 1.0, b = 0.0 })
				)
				showbuysellbutton = nobutton
				return
			end
		else
			stock = Game.player:CountEquip(tradecommodity)
		end
		local wantamount = tradeamount + delta
		local tradecost = wantamount * price
		
		--we cant trade more units than we have in stock
		if delta > 0 and wantamount > stock then
			wantamount = stock
		end

		--we dont trade in negative quantities
		if wantamount < 0 then
			wantamount = 0
		end

		local tradeword
		if trade_mode == trade_mode_buy then
			local playerfreecargo = Game.player.freeCapacity
			if tradecost > playercash then
				wantamount = math.floor(playercash / price)
			end
			local tradecargo = tradecommodity.capabilities.mass * wantamount
			if playerfreecargo < tradecargo then
				wantamount = math.floor(playerfreecargo / tradecommodity.capabilities.mass)
			end
			tradeword = "Buy "
		else --mode = sell
			--if market price is negative make sure player wont go below zero credits after the deal
			if (playercash + tradecost) < 0 then
				wantamount = tradeamount --kludge, ignore the delta unless player has finances to cover the deal
				--if player starts at 0 quantity, presses +100 to "sell" radioactives but only has
				--enough credits to sell 5, this kludge will ignore the +100 completely
				--todo: change amount to 5 instead
			end
			tradeword = "Sell "
		end
		tradeamount = wantamount
		tradecost = tradeamount * price
		buysell:SetInnerWidget(ui:Label(tradeword..tradeamount.." units for "..Format.Money(tradecost)):SetFont("LARGE"))
	end

	sub100.onClick:Connect(function () changeTradeamount(-100) end)
	subten.onClick:Connect(function () changeTradeamount(-10) end)
	subone.onClick:Connect(function () changeTradeamount(-1) end)
	tradereset.onClick:Connect(function () changeTradeamount(-tradeamount) end)
	addone.onClick:Connect(function () changeTradeamount(1) end)
	addten.onClick:Connect(function () changeTradeamount(10) end)
	add100.onClick:Connect(function () changeTradeamount(100) end)

	local buyorsell = function()
		if trade_mode == trade_mode_buy then
			local n = Game.player:CountEquip(tradecommodity)
			if n > 0 then
			commodityTrade:SetInnerWidget(
				ui:Expand("VERTICAL",ui:VBox():PackEnd({
					commonHeader,
					ui:Margin(32,"VERTICAL",
						ui:VBox():PackEnd({
							commonButtons,
							ui:Margin(16,"TOP",buysell),
						})
					),
					ui:Margin(32,"VERTICAL",
						ui:HBox():PackEnd({
							ui:Align("MIDDLE",ui:Label("You have "..n.." units in your cargohold")),
							ui:Margin(16,"LEFT",sellfromcargo),
						})
					),
					showbuysellbutton,
				}))
			)
			else
			commodityTrade:SetInnerWidget(
				ui:Expand("VERTICAL",ui:VBox():PackEnd({
					commonHeader,
					ui:Margin(32,"VERTICAL",
						ui:VBox():PackEnd({
							commonButtons,
							ui:Margin(16,"TOP",buysell),
						})
					),
					showbuysellbutton,
				}))
			)
			end
			return
		end
		if trade_mode == trade_mode_sell then
			commodityTrade:SetInnerWidget(
				ui:VBox():PackEnd({
					commonHeader,
					ui:Margin(32,"VERTICAL",
						ui:VBox():PackEnd({
							commonButtons,
							ui:Margin(16,"TOP",buysell),
						})
					),
					ui:Margin(32,"VERTICAL",buyfrommarket),
					showbuysellbutton,
				})
			)
			return
		end
	end

	local doBuy = function ()
		local price = Game.player:GetDockedWith():GetEquipmentPrice(tradecommodity)
		local stock = Game.player:GetDockedWith():GetEquipmentStock(tradecommodity)
		local playerfreecargo = Game.player.freeCapacity
		local orderamount = price * tradeamount
	--check cash (should never happen since trade amount buttons wont let it happen)
		if orderamount > Game.player:GetMoney() then
			MessageBox.Message(l.YOU_NOT_ENOUGH_MONEY)
			return
		end
	--check stock
		if tradeamount > stock then
			MessageBox.Message(l.ITEM_IS_OUT_OF_STOCK)
			return
		end
	--check cargo limit
		local tradecargo = tradecommodity.capabilities.mass * tradeamount
		if playerfreecargo < tradecargo then
			MessageBox.Message(l.SHIP_IS_FULLY_LADEN)
			return
		end
	--all checks passed
		assert(Game.player:AddEquip(tradecommodity, tradeamount, "cargo") == tradeamount)
		Game.player:AddMoney(-orderamount) --grab the money
		Game.player:GetDockedWith():AddEquipmentStock(tradecommodity, -tradeamount)
		changeTradeamount(-tradeamount) --reset the trade amount
		--update market rows
		tradeamount = 0;
		changeTradeamount(0)
		buyorsell()
		marketRows = fillMarketTable()
	end

	local doSell = function ()
		local price = Game.player:GetDockedWith():GetEquipmentPrice(tradecommodity)
		local stock = Game.player:GetDockedWith():GetEquipmentStock(tradecommodity)
		local playerfreecargo = Game.player.freeCapacity
		local orderamount = price * tradeamount
	--if commodity price is negative (radioactives, garbage), player needs to have enough cash
	--todo: checks! needed? changetradeamount() already does checking
	--all checks passed
		Game.player:RemoveEquip(tradecommodity, tradeamount, "cargo")
		Game.player:AddMoney(orderamount) --grab the money
		Game.player:GetDockedWith():AddEquipmentStock(tradecommodity, tradeamount)
		changeTradeamount(-tradeamount) --reset the trade amount
		--if player sold all his cargo, switch to buy panel
		if Game.player:CountEquip(tradecommodity) == 0 then
			trade_mode = trade_mode_buy
			showbuysellbutton = confirmtradebuy
		end
		--update market rows
		tradeamount = 0;
		changeTradeamount(0)
		buyorsell()
		marketRows = fillMarketTable()
	end

	
	sellfromcargo.onClick:Connect(function ()
		trade_mode = trade_mode_sell
		tradeamount = 0
		showbuysellbutton = confirmtradesell
		changeTradeamount(0)
		buyorsell()
	end)
	
	buyfrommarket.onClick:Connect(function ()
		trade_mode = trade_mode_buy
		tradeamount = 0
		showbuysellbutton = confirmtradebuy
		changeTradeamount(0)
		buyorsell()
	end)

	confirmtradebuy.onClick:Connect(doBuy)
	confirmtradesell.onClick:Connect(doSell)
	
	-- when you click on a row in the stations commodity list
	marketTable.onRowClicked:Connect(function(row)
		local e = marketRows[row+1]

		tradeamount = 0;
		tradecommodity = e
		trade_mode = trade_mode_buy

		--clear the header because previous calls might have filled it
		commonHeader:Clear()
		--update common header to the commodity that was clicked
		if e.description then
			commonHeader:PackEnd({
				ui:VBox():PackEnd({
					ui:Margin(16,"VERTICAL",
						ui:HBox():PackEnd({
							ui:Margin(32,"RIGHT",marketColumnValue["icon"](e)),
							ui:Label(marketColumnValue["name"](e)):SetFont("HEADING_LARGE"),
						})
					),
					ui:Expand("HORIZONTAL",ui:MultiLineText(e.description):SetFont("SMALL"))
				})
			})
		else
			commonHeader:PackEnd({
				ui:Margin(16,"TOP",ui:HBox():PackEnd({
					ui:Margin(32,"RIGHT",marketColumnValue["icon"](e)),
					ui:Label(marketColumnValue["name"](e)):SetFont("HEADING_LARGE"),
				}))
			})
		end
		showbuysellbutton = confirmtradebuy
		changeTradeamount(0)
		buyorsell()

		marketRows = fillMarketTable()
	end)

	return
		ui:Grid({48,4,48},1)
			:SetColumn(0, {marketTable})
			:SetColumn(2, {commodityTrade})
end

return commodityMarket

