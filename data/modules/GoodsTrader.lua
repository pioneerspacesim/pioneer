local goods_trader_flavour = {
	"Honest %s's Goods Emporium",
	"%s's Trading House",
	"%s's Warehouse",
	"%s's Goods Exchange",
	"%s Holdings",
	"%s & Sons"
}

local ads = {}

local onActivate = function (dialog, ref, option)
	local ad = ads[ref]

	if option == -1 then
		dialog:close()
		return
	end

	ad.stock = {}
	ad.price = {}
	--[[
	for e = Equip.FIRST_COMMODITY, Equip.LAST_COMMODITY do
		if not Pi.GetCurrentSystem():IsCommodityLegal(e) then
			ad.stock[e] = Pi.rand:Int(1,50)
			-- going rate on the black market will be twice normal
			ad.price[e] = 2*ad.bb:GetEquipmentPrice(e)
		end
	end
	]]
	--	ad.stock = {[Equip.WATER]=20, [Equip.HYDROGEN]=15, [Equip.NERVE_GAS]=0}
	--	ad.price = {[Equip.WATER]=120, [Equip.HYDROGEN]=130, [Equip.NERVE_GAS]=1000}

	dialog:clear()
	dialog:set_title(ad.flavour)
	dialog:set_message("Welcome to "..ad.flavour)

	dialog:add_goods_trader({
		-- can I trade this commodity?
		canTrade = function (ref, commodity)
			print("can trade "..commodity)
			return true
		end,

		-- how much of this commodity do we have in stock?
		getStock = function (ref, commodity)
			print("get stock "..commodity)
			return 0
		end,

		-- what do we charge for this commodity?
		getPrice = function (ref, commodity)
			print("get price "..commodity)
			return 0
		end,

		-- do something when a "buy" button is clicked
		onClickBuy = function (ref, commodity)
			print("buy "..commodity.." clicked")
			-- allow purchase to proceed
			return true
		end,

		-- do something when a "buy" button is clicked
		onClickSell = function (ref, commodity)
			print("sell "..commodity.." clicked")
			-- allow sale to proceed
			return true
		end,

		-- do something when we buy this commodity
		bought = function (ref, commodity)
			print("bought "..commodity)
		end,

		-- do something when we sell this commodity
		sold = function (ref, commodity)
			print("sold "..commodity)
		end,
	})

	dialog:add_option("Hang up.", -1);

end

local onDelete = function (ref)
	ads[ref] = nil
end

local onCreateBB = function (station)
	--print("Creating bb adverts for " .. station:get_label())
	local rand = Rand:new(station:get_seed())
	local num = rand:Int(1,3)
	for i = 1,num do
		local isfemale = rand:Int(0, 1) == 1
		local ispolice = rand:Int(0, 1) == 1

		local flavour = string.format(goods_trader_flavour[rand:Int(1, #goods_trader_flavour)], rand:Surname(isfemale))

		local ad = {
			id       = #ads+1,
			station  = station,
			flavour  = flavour,
			ispolice = ispolice,
		}

		local ref = station:add_advert(ad.flavour, onActivate, onDelete)
		ads[ref] = ad;
	end
end

Module:new {
	__name='GoodsTrader', 
	
	Init = function(self)
		EventQueue.onCreateBB:connect(onCreateBB)
	end,
}

--[[
	onCreateBB = function(self, args)
	end,

	onChatBB = function(self, dialog, optionClicked)
	end,

	TraderGetStock = function(self, dialog, comType)
		return self.ads[ dialog:GetAdRef() ].stock[comType]
	end,

	TraderGetPrice = function(self, dialog, comType)
		return self.ads[ dialog:GetAdRef() ].price[comType]
	end,

	TraderBought = function(self, dialog, comType)
		local stock = self.ads[ dialog:GetAdRef() ].stock
		stock[comType] = stock[comType] + 1
	end,

	TraderSold = function(self, dialog, comType)
		local stock = self.ads[ dialog:GetAdRef() ].stock
		stock[comType] = stock[comType] - 1
	end,

	TraderCanTrade = function(self, dialog, comType)
		if self.ads[ dialog:GetAdRef() ].stock[comType] ~= nil then
			return true
		end
	end,

	_WhenTradeHappens = function(self, dialog, comType)
		if self.ads[dialog:GetAdRef()].ispolice == true then
			local lawlessness = Pi:GetCurrentSystem():GetSystemLawlessness()
			Pi.AddPlayerCrime(Polit.Crime.TRADING_ILLEGAL_GOODS, 400*(2-lawlessness))
			dialog:GotoPolice()
			return false
		else
			return true
		end
	end,

	TraderOnClickSell = function(self, dialog, comType)
		return self:_WhenTradeHappens(dialog, comType)
	end,
	TraderOnClickBuy = function(self, dialog, comType)
		return self:_WhenTradeHappens(dialog, comType)
	end,
}
]]
