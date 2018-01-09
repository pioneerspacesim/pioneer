-- Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Lang = import("Lang")
local Game = import("Game")
local Format = import("Format")
local Equipment = import("Equipment")

local EquipmentTableWidgets = import("EquipmentTableWidgets")
local ui = Engine.ui
local l = Lang.GetResource("ui-core")

local marketColumnValue = {
	icon  = function (e) return e.icon_name and ui:Image("icons/goods/"..e.icon_name..".png") or "" end,
	name  = function (e) return e:GetName() end,
	desc  = function (e) return e:GetDescription() end,
	price = function (e) return Format.Money(Game.player:GetDockedWith():GetEquipmentPrice(e)) end,
	stock = function (e) return Game.player:GetDockedWith():GetEquipmentStock(e) end,
	cargo = function (e)
		local n = Game.player:CountEquip(e)
		return n > 0 and n or '' --lua logic, returns n if n > 0, otherwise returns ''
	end,
}

local commodityMarket = function (args)

	local equipTypes = {}
	for k,e in pairs(Equipment.cargo) do
		-- if its purchasable, a cargo type equipment and a legal commodity in this system then we can list it in the commodity market
		if e.purchasable and e:IsValidSlot("cargo") and Game.system:IsCommodityLegal(e) then
			-- its ok, put it in the list
			table.insert(equipTypes, e)
		end
	end

	table.sort(equipTypes, function(e1,e2)
		return e1:GetName() < e2:GetName()        -- cargo sorted on translated name
	end)

	local marketTable =
		ui:Table()
			:SetRowSpacing(5)			--margin between rows
			:SetColumnSpacing(10)		--margin between columns
			:SetHeadingRow({
				'',						--blank header column used for the icons in the list
				ui:Label(l.NAME_OBJECT):SetFont("LARGE"),
				ui:Align("RIGHT",l.PRICE),
				"",						--blank column for import/export icons
				l.IN_STOCK,
				ui:Margin(24,"RIGHT",l.CARGO) -- because a later expand() expands it too much we have to pad the table with some blank space at the right edge or the text will be cropped by the scrollbar
			})
			--:SetHeadingFont("LARGE")	--large font for header
			:SetRowAlignment("CENTER")	--align the row to the center relative to up and down
			:SetMouseEnabled(true)		--mouse can be used on this list

	local commodityTrade =
		ui:Expand("VERTICAL")			--blank right pane gets filled in by code once a row in the list is clicked

	local pricemod = nil
	local priceicon = nil
	local trac = Game.player:GetEquipCountOccupied('trade_computer')
	local majex = "icons/market/export-major.png"
	local majim = "icons/market/import-major.png"
	local minex = "icons/market/export-minor.png"
	local minim = "icons/market/import-minor.png"
	
	local fillMarketTable = function()
		marketTable:ClearRows()
		local rowCommodity = {}
		for i,e in ipairs(equipTypes) do
			pricemod = nil
			if trac > 0 then
				local mod = Game.system:GetCommodityBasePriceAlterations(e)
				if mod >= 10 then
					pricemod = majim
				elseif mod >= 2 then
					pricemod = minim
				elseif mod <= -10 then
					pricemod = majex
				elseif mod <= -2 then
					pricemod = minex
				end
			end
			marketTable:AddRow({
				marketColumnValue["icon"](e),
				ui:Expand("HORIZONTAL",marketColumnValue["name"](e)),	--names are aligned to the left (default)
																		--expand the bame to make the whole row fill the horizontal space,
																		--expands too much and cargo column is clipped by scrollbar, but fixed with margins on cargo column header (above) and rows (3 lines down)
				ui:Align("RIGHT", marketColumnValue["price"](e)),		--numbers are aligned to the right for decimal places to be in the same spot
				pricemod and ui:Image(pricemod) or "",
				ui:Align("RIGHT", marketColumnValue["stock"](e)),
				ui:Align("RIGHT", ui:Margin(32,"RIGHT",marketColumnValue["cargo"](e))), --margin is fix for greedy expand() 3 lines up
			})
			table.insert(rowCommodity, e)	--cant use the widgeted rows to get commodity objects, so this duplicate table is same line by line but only contains the commodity object
		end
		return rowCommodity
	end
	local marketRows = fillMarketTable()

	local trade_mode_buy = 22			--magic number for trade_mode = buy
	local trade_mode_sell = 33			--magic number for trade_mode = sell
										--not using 0 or 1 to avoid confusion with true and false
	local trade_mode = trade_mode_buy	--start off right pane is buy mode
	local tradeamount = 0;				--initial number of commodity to buy
	local tradecommodity = ''			--blank value, uses commodity objects, unset until user clicks a row in left pane
	local buysell =
		ui:Label(""):SetFont("LARGE")	--blank widget
	local tradeflipflop =
		ui:Expand()						--blank widget

	local sub100 = ui:Button("-100")
	local subten = ui:Button("-10")
	local subone = ui:Button("-1")
	local tradereset = ui:Button(l.RESET)
	local addone = ui:Button("+1")
	local addten = ui:Button("+10")
	local add100 = ui:Button("+100")
	local confirmtradebuy = ui:Button(l.CONFIRM_PURCHASE):SetFont("HEADING_LARGE")
	local confirmtradesell = ui:Button(l.CONFIRM_SALE):SetFont("HEADING_LARGE")
	local nobutton = nil
	local showbuysellbutton = confirmtradebuy
	local sellfromcargo = ui:Button(l.SELL)
	local buyfrommarket = ui:Button(l.BUY)

	local commonHeader = ui:HBox(24)		--blank header for right pane, filled in by code once user has selected which commodity to trade in
	local computerInfo = ui:HBox(24)
	local commonDescript = ui:HBox()
	local hasCargoLabel = ui:Label("")
	local commonButtons =
		ui:HBox():PackEnd({					--pack all the buttons into one widget for future use, hbox lines up elements horizontally
			sub100,							--first button does not need a left margin
			ui:Margin(16,"LEFT",subten),	--all the following buttons needs a margin to separate it from the previous one
			ui:Margin(16,"LEFT",subone),
			ui:Margin(16,"LEFT",tradereset),
			ui:Margin(16,"LEFT",addone),
			ui:Margin(16,"LEFT",addten),
			ui:Margin(16,"LEFT",add100)
		})

	-- calls to this function alter traded amount up or down, not absolute values
	local changeTradeamount = function (delta)

		--get price of commodity after applying local effects of import/export modifiers
		local price = Game.player:GetDockedWith():GetEquipmentPrice(tradecommodity)

		--do you have any money?
		local playercash = Game.player:GetMoney()

		--blank value, needs to be initialized or later on lua will complain
		local stock

		if trade_mode == trade_mode_buy then
			stock = Game.player:GetDockedWith():GetEquipmentStock(tradecommodity)
			if stock == 0 then
				buysell:SetText(l.NONE_FOR_SALE_IN_THIS_STATION)
				buysell:SetColor({ r = 1.0, g = 0.0, b = 0.0 }) --set the color of the message to fullblown red
				showbuysellbutton:Disable()
				return
			end
			if price > playercash then
				buysell:SetText(l.INSUFFICIENT_FUNDS)
				buysell:SetColor({ r = 1.0, g = 1.0, b = 0.0 }) --set the color of the message to lovely yellow
				showbuysellbutton:Disable()
				return
			end
		else
			stock = Game.player:CountEquip(tradecommodity)
		end
		showbuysellbutton:Enable()

		--dont alter tradeamount before checks have been made
		local wantamount = tradeamount + delta

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
		local tradetext

		if trade_mode == trade_mode_buy then
			local playerfreecargo = Game.player.totalCargo - Game.player.usedCargo
			if tradecost > playercash then
				wantamount = math.floor(playercash / price)
			end
			local tradecargo = tradecommodity.capabilities.mass * wantamount
			if playerfreecargo < tradecargo then
				wantamount = math.floor(playerfreecargo / tradecommodity.capabilities.mass)
			end
			tradetext = l.MARKET_BUYLINE
		else --mode = sell
			--if market price is negative make sure player wont go below zero credits after the deal
			if (playercash + tradecost) < 0 then
				wantamount = tradeamount --kludge, ignore the delta unless player has finances to cover the deal
				--if player starts at 0 quantity, presses +100 to "sell" radioactives but only has
				--enough credits to sell 5, this kludge will ignore the +100 completely
				--todo: change amount to 5 instead
			end
			tradetext = l.MARKET_SELLINE
		end

		--wantamount is now checked and modified to a safe bounded amount
		tradeamount = wantamount

		--current cost of market order if user confirms the deal
		tradecost = tradeamount * price

		--its possible to get to this line without tradetext being initialized unless done 30 rows up
		buysell:SetText(string.interp(tradetext,{ amount = string.format("%d", tradeamount), price = Format.Money(tradecost)}))
		buysell:SetColor({ r = 1.0, g = 1.0, b = 1.0 }) --set the color of the message to default white
	end

	--attach click actions to all the buttons
	sub100.onClick:Connect(function () changeTradeamount(-100) end)
	subten.onClick:Connect(function () changeTradeamount(-10) end)
	subone.onClick:Connect(function () changeTradeamount(-1) end)
	-- tradeamount minus tradeamount is zero, as the argument needs to be a delta, not an absolute number
	tradereset.onClick:Connect(function () changeTradeamount(-tradeamount) end)
	addone.onClick:Connect(function () changeTradeamount(1) end)
	addten.onClick:Connect(function () changeTradeamount(10) end)
	add100.onClick:Connect(function () changeTradeamount(100) end)

	--this is the layout for the right pane, only shown once user selects a commodity to trade
	local buyorsell = function()
		--buy pane is complex layout because of checking if player has any commodity to sell
		if trade_mode == trade_mode_buy then
			local n = Game.player:CountEquip(tradecommodity)
			if n > 0 then
				sellfromcargo:Enable()
				hasCargoLabel:SetText(string.interp(l.YOU_HAVE_X_UNITS_IN_YOUR_CARGOHOLD, {units = n}))
			else
				hasCargoLabel:SetText("")
				sellfromcargo:Disable()
			end
				
			commodityTrade:SetInnerWidget(
				ui:VBox(15):PackEnd({
					ui:HBox(30):PackEnd({
						buyfrommarket,
						sellfromcargo
					}),
					commonHeader, --common header contains icon and commodity name
					computerInfo, -- info from trade computer, if the player has one, otherwise blank
					--contains description, if there is one
					ui:Expand("VERTICAL",commonDescript),
					commonButtons, -- prepared widget with all the buttons -100 -10 -1 reset +1 +10 +100
					buysell,
					--this widgetset is only show if the player has an amount of the commodity in cargo to sell
					ui:Align("LEFT",hasCargoLabel),
					showbuysellbutton --button to confirm buying the selected amount
				})
			)
			return
		end

		--sell pane is simpler
		if trade_mode == trade_mode_sell then
			commodityTrade:SetInnerWidget(
				ui:VBox(15):PackEnd({
					ui:HBox(30):PackEnd({
						buyfrommarket,
						sellfromcargo
					}),
					commonHeader, --icon, commodity name
					computerInfo, -- info from trade computer, if the player has one, otherwise blank
					commonDescript, --description (optional)
					ui:Expand("VERTICAL"),
					commonButtons, --prepared buttons -100 to +100
					buysell,
					ui:Align("LEFT",hasCargoLabel),
					showbuysellbutton, --confirm selling
				})
			)
			return
		end
	end

	--player clicked confirm purchase button
	local doBuy = function ()
		local price = Game.player:GetDockedWith():GetEquipmentPrice(tradecommodity)
		local stock = Game.player:GetDockedWith():GetEquipmentStock(tradecommodity)
		local playerfreecargo = Game.player.totalCargo - Game.player.usedCargo
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
		changeTradeamount(0) --update trade amount text
		buyorsell() --update right pane
		marketRows = fillMarketTable() --rows needs to be recalculated since now the amounts in stock have changed
	end

	--player clicked the confirm sale button
	local doSell = function ()
		local price = Game.player:GetDockedWith():GetEquipmentPrice(tradecommodity)
		local stock = Game.player:GetDockedWith():GetEquipmentStock(tradecommodity)
		local playerfreecargo = Game.player.totalCargo - Game.player.usedCargo
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
			buyfrommarket:Hide()
			buyfrommarket:SetFont("LARGE")
			sellfromcargo:Enable()
			sellfromcargo:SetFont("SMALL")
		end

		--update market rows
		tradeamount = 0;
		changeTradeamount(0) --update trade amount text
		buyorsell() --update right pane
		marketRows = fillMarketTable() --stock has changed, update rows
	end

	--code to change from buy to sell mode, attached to button onclick
	sellfromcargo.onClick:Connect(function ()
		trade_mode = trade_mode_sell
		tradeamount = 0
		showbuysellbutton = confirmtradesell --change which confirm button to show
		sellfromcargo:Hide()
		sellfromcargo:SetFont("LARGE")
		buyfrommarket:Enable()
		buyfrommarket:SetFont("SMALL")
		changeTradeamount(0)
		buyorsell()
	end)

	--code to change from sell mode to buy, attached to button onclick
	buyfrommarket.onClick:Connect(function ()
		trade_mode = trade_mode_buy
		tradeamount = 0
		showbuysellbutton = confirmtradebuy --change which confirm button to show
		buyfrommarket:Hide()
		buyfrommarket:SetFont("LARGE")
		sellfromcargo:Enable()
		sellfromcargo:SetFont("SMALL")
		changeTradeamount(0)
		buyorsell()
	end)

	--attach code to confirm buttons
	confirmtradebuy.onClick:Connect(doBuy)
	confirmtradesell.onClick:Connect(doSell)

	-- when you click on a row in the stations commodity list
	marketTable.onRowClicked:Connect(function(row)
		local e = marketRows[row+1] --skip header row with the +1 (?)

		tradeamount = 0; --start off trade amount at zero
		tradecommodity = e
		trade_mode = trade_mode_buy --start off in trade_mode = buy

		--clear the header because previous calls might have filled it
		commonHeader:Clear()
		computerInfo:Clear()
		commonDescript:Clear()
		
		pricemod = ""
		priceicon = nil
		if trac > 0 then						--trac = if player has trade computer
			local mod = Game.system:GetCommodityBasePriceAlterations(e)
			if mod >= 10 then
				pricemod = l.MAJOR_IMPORTS_ITEM
				priceicon = majim
			elseif mod >= 2 then
				pricemod = l.MINOR_IMPORTS_ITEM
				priceicon = minim
			elseif mod <= -10 then
				pricemod = l.MAJOR_EXPORTS_ITEM
				priceicon = majex
			elseif mod <= -2 then
				pricemod = l.MINOR_EXPORTS_ITEM
				priceicon = minex
			end
		end

		--update common header to the commodity that was clicked
		commonHeader:PackEnd({
			marketColumnValue["icon"](e),
			--simple text label with the commodity name
			ui:Expand("HORIZONTAL",ui:Label(marketColumnValue["name"](e)):SetFont("HEADING_LARGE")),
			--if player has trade computer, show major/minor export/import, otherwise blank
			""		--cheap way of adding a right margin								
		})
		
		computerInfo:PackEnd({
			priceicon and ui:Image(priceicon,{ "PRESERVE_ASPECT" }) or "",
			ui:Label(pricemod)
		})
		
		commonDescript:PackEnd({
			ui:Expand("HORIZONTAL",ui:MultiLineText(marketColumnValue["desc"](e)):SetFont("SMALL")) --expand textbox to fill horizontal space
		})

		showbuysellbutton = confirmtradebuy
		buyfrommarket:Hide()
		buyfrommarket:SetFont("LARGE")
		sellfromcargo:Enable()
		sellfromcargo:SetFont("SMALL")
		changeTradeamount(0)
		buyorsell()

		marketRows = fillMarketTable()
	end)

	return
		ui:Grid({48,2,50},1) --make a simple grid layout with 3 columns, 48%, 2% and 50% the width of the space, 1 row
			:SetColumn(0, {marketTable})
			--column 1 is empty
			:SetColumn(2, {commodityTrade})
end

return commodityMarket
