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

    local sys = Pi.GetCurrentSystem()

	ad.stock = {}
	ad.price = {}
	for e = Equip.FIRST_COMMODITY, Equip.LAST_COMMODITY do
		if not sys:is_commodity_legal(e) then
			ad.stock[e] = Pi.rand:Int(1,50)
			-- going rate on the black market will be twice normal
			ad.price[e] = ad.station:get_equipment_price(e) * 2
		end
	end
	--ad.stock = {[Equip.WATER]=20, [Equip.HYDROGEN]=15, [Equip.NERVE_GAS]=0}
	--ad.price = {[Equip.WATER]=120, [Equip.HYDROGEN]=130, [Equip.NERVE_GAS]=1000}

	dialog:clear()
	dialog:set_title(ad.flavour)
	dialog:set_message("Welcome to "..ad.flavour)

	local onClick = function (ref)
		if not ads[ref].ispolice then
			return true
		end

		local lawlessness = Pi:GetCurrentSystem():get_lawlessness()
		Pi.AddPlayerCrime(Polit.Crime.TRADING_ILLEGAL_GOODS, 400*(2-lawlessness))
		dialog:goto_police()
		return false
	end
	
	dialog:add_goods_trader({
		-- can I trade this commodity?
		canTrade = function (ref, commodity)
			if ads[ref].stock[commodity] ~= nil then
				return true
			end
		end,

		-- how much of this commodity do we have in stock?
		getStock = function (ref, commodity)
			return ads[ref].stock[commodity]
		end,

		-- what do we charge for this commodity?
		getPrice = function (ref, commodity)
			return ads[ref].price[commodity]
		end,

		-- do something when a "buy" button is clicked
		onClickBuy = function (ref, commodity)
			-- allow purchase to proceed
			return onClick(ref)
		end,

		-- do something when a "buy" button is clicked
		onClickSell = function (ref, commodity)
			-- allow sale to proceed
			return onClick(ref)
		end,

		-- do something when we buy this commodity
		bought = function (ref, commodity)
            ads[ref].stock[commodity] = ads[ref].stock[commodity] + 1
		end,

		-- do something when we sell this commodity
		sold = function (ref, commodity)
            ads[ref].stock[commodity] = ads[ref].stock[commodity] - 1
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
