local goods_trader_flavour = {
	"Honest %s's Goods Emporium",
	"%s's Trading House",
	"%s's Warehouse",
	"%s's Goods Exchange",
	"%s Holdings",
	"%s & Sons"
}

local ads = {}

local onChat = function (dialog, ref, option)
	local ad = ads[ref]

	if option == -1 then
		dialog:Close()
		return
	end

    local sys = Pi.GetCurrentSystem()

	ad.stock = {}
	ad.price = {}
	for e = Equip.Type.FIRST_COMMODITY, Equip.Type.LAST_COMMODITY do
		if not sys:IsCommodityLegal(e) then
			ad.stock[e] = Pi.rand:Int(1,50)
			-- going rate on the black market will be twice normal
			ad.price[e] = ad.station:GetEquipmentPrice(e) * 2
		end
	end
	--ad.stock = {[Equip.WATER]=20, [Equip.HYDROGEN]=15, [Equip.NERVE_GAS]=0}
	--ad.price = {[Equip.WATER]=120, [Equip.HYDROGEN]=130, [Equip.NERVE_GAS]=1000}

	dialog:Clear()
	dialog:SetTitle(ad.flavour)
	dialog:SetMessage("Welcome to "..ad.flavour)

	local onClick = function (ref)
		if not ads[ref].ispolice then
			return true
		end

		local lawlessness = Pi:GetCurrentSystem():GetLawlessness()
		Pi.AddPlayerCrime(Polit.Crime.TRADING_ILLEGAL_GOODS, 400*(2-lawlessness))
		dialog:GotoPolice()
		return false
	end
	
	dialog:AddGoodsTrader({
		-- can I trade this commodity?
		canTrade = function (ref, commodity)
			if ads[ref].stock[commodity] then
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
			return onClick(ref)
		end,

		-- do something when a "buy" button is clicked
		onClickSell = function (ref, commodity)
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

	dialog:AddOption("Hang up.", -1);

end

local onDelete = function (ref)
	ads[ref] = nil
end

local onCreateBB = function (station)
	local rand = Rand:new(station:GetSeed())
	local num = rand:Int(1,3)
	for i = 1,num do
		local isfemale = rand:Int(0, 1) == 1
		local ispolice = rand:Int(0, 1) == 1

		local flavour = string.format(goods_trader_flavour[rand:Int(1, #goods_trader_flavour)], rand:Surname(isfemale))

		local ad = {
            station  = station,
			flavour  = flavour,
			ispolice = ispolice,
		}

		local ref = station:AddAdvert(ad.flavour, onChat, onDelete)
		ads[ref] = ad;
	end
end

EventQueue.onCreateBB:Connect(onCreateBB)
