local VERSION = 1 -- Integer versioning; bump this up if the saved game format changes.

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
	return {
		VERSION = VERSION,
		ads = ads,
	}
end

local unserialize = function (data)
	loaded_data = data
	if data.VERSION then
		if data.VERSION < VERSION then
			print('Old GoodsTrader data loaded, converting...')
			-- No upgrade code yet
			print(('GoodsTrader data converted to internal version {newversion}'):interp({newversion=VERSION}))
			return
		end
		if data.VERSION > VERSION then
			error(([[GoodsTrader load error - saved game is more recent than installed files
			Saved game internal version: {saveversion}
			Installed internal version: {ourversion}]]):interp({saveversion=data.VERSION,ourversion=VERSION}))
		end
	else
		-- Hopefully, a few engine save-game bumps from now,
		-- there will be no instance where this is acceptable,
		-- and we can error() out of here.
		print('Pre-versioning GoodsTrader data loaded')
	end
end

Event.Register("onCreateBB", onCreateBB)
Event.Register("onGameStart", onGameStart)

Serializer:Register("GoodsTrader", serialize, unserialize)
