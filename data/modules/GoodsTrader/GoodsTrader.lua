-- Get the translator function
local t = Translate:GetTranslator()

local ads = {}

local onChat = function (form, ref, option)
	local ad = ads[ref]

	if option == -1 then
		form:Close()
		return
	end

	form:Clear()
	form:SetTitle(ad.flavour)
	form:SetFace({ seed = ad.faceseed })
	form:SetMessage(t("Welcome to ")..ad.flavour)

	local onClick = function (ref)
		if not ads[ref].ispolice then
			return true
		end

		local lawlessness = Game.system.lawlessness
		Game.player:AddCrime("TRADING_ILLEGAL_GOODS", 400*(2-lawlessness))
		form:GotoPolice()
		return false
	end
	
	form:AddGoodsTrader({
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

		-- do something when a "sell" button is clicked
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

end

local onDelete = function (ref)
	ads[ref] = nil
end

local onCreateBB = function (station)
	local has_illegal_goods = false
	for i,e in pairs(Constants.EquipType) do
		if not Game.system:IsCommodityLegal(e) then
			has_illegal_goods = true
		end
	end
	if not has_illegal_goods then return end

	local rand = Rand.New(station.seed)
	local num = rand:Integer(1,3)
	for i = 1,num do
		local ispolice = rand:Integer(1) == 1

		local flavour = string.interp(t('GOODS_TRADER')[rand:Integer(1, #(t('GOODS_TRADER')))], {name = NameGen.Surname(rand)})

		local ad = {
			station  = station,
			flavour  = flavour,
			ispolice = ispolice,
			faceseed = rand:Integer(),
		}

		ad.stock = {}
		ad.price = {}
		for i,e in pairs(Constants.EquipType) do
			if not Game.system:IsCommodityLegal(e) then
				ad.stock[e] = Engine.rand:Integer(1,50)
				-- going rate on the black market will be twice normal
				ad.price[e] = ad.station:GetEquipmentPrice(e) * 2
			end
		end

		local ref = station:AddAdvert(ad.flavour, onChat, onDelete)
		ads[ref] = ad
	end
end

local loaded_data

local onGameStart = function ()
	ads = {}

	if not loaded_data then return end

	for k,ad in pairs(loaded_data.ads) do
		local ref = ad.station:AddAdvert(ad.flavour, onChat, onDelete)
		ads[ref] = ad
	end

	loaded_data = nil
end

local serialize = function ()
	return { ads = ads }
end

local unserialize = function (data)
	loaded_data = data
end

EventQueue.onCreateBB:Connect(onCreateBB)
EventQueue.onGameStart:Connect(onGameStart)

Serializer:Register("GoodsTrader", serialize, unserialize)
