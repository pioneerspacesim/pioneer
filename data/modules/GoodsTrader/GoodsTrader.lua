-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Lang = require 'Lang'
local Engine = require 'Engine'
local Game = require 'Game'
local Event = require 'Event'
local NameGen = require 'NameGen'
local Rand = require 'Rand'
local Serializer = require 'Serializer'
local Character = require 'Character'
local Commodities = require 'Commodities'

local l = Lang.GetResource("module-goodstrader")

local num_names = 6 -- number of GOODS_TRADER_N names
local num_slogans = 7 -- number of SLOGAN_N entries
local num_titles = 4 -- number of ADTITLE_N entries

local ads = {}

local onChat = function (form, ref, option)
	local ad = ads[ref]

	if option == -1 then
		form:Close()
		return
	end

	form:Clear()
	form:SetTitle(ad.flavour)
	form:SetFace(ad.trader)
	form:SetMessage(l.WELCOME_TO..ad.flavour..".\n"..ad.slogan)

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

		canDisplayItem = function (ref, commodity)
			if ads[ref].stock[commodity] then
				return true
			end
		end,

		-- how much of this commodity do we have in stock?
		getStock = function (ref, commodity)
			return ads[ref].stock[commodity]
		end,

		-- what do we charge for this commodity?
		getBuyPrice = function (ref, commodity)
			return ads[ref].price[commodity]
		end,

		-- what do we return
		getSellPrice = function (ref, commodity)
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
		bought = function (ref, commodity, market)
			local count = 1
			if market.tradeAmount ~= nil then
				count = market.tradeAmount
			end

			ads[ref].stock[commodity] = ads[ref].stock[commodity] - count
		end,

		-- do something when we sell this commodity
		sold = function (ref, commodity, market)
			local count = 1
			if market.tradeAmount ~= nil then
				count = market.tradeAmount
			end

			ads[ref].stock[commodity] = ads[ref].stock[commodity] + count
		end,
	})

end

local onDelete = function (ref)
	ads[ref] = nil
end

local onCreateBB = function (station)
	local has_illegal_goods = false
	for key in pairs(Commodities) do
		if not Game.system:IsCommodityLegal(key) then
			has_illegal_goods = true
		end
	end
	if not has_illegal_goods then return end

	-- Real trader is persistant for each station, police is random every time
	local rand = Rand.New(station.seed)
	local rand_police = Rand.New()

	local num = rand:Integer(1,3)

	local numPolice = 0
	local maxPolice = 1

	for i = 1,num do
		local ispolice = rand:Integer(1) == 1

		-- if too many fake police, don't place the ad
		if not ispolice or numPolice < maxPolice then
			local r = ispolice and rand_police or rand

			if ispolice then
				numPolice = numPolice + 1
			end

			local title = l["ADTITLE_" .. r:Integer(num_titles - 1)]
			local flavour = string.interp(l["GOODS_TRADER_"..r:Integer(num_names - 1)], { name = NameGen.Surname(r) })
			local slogan = l["SLOGAN_"..r:Integer(num_slogans - 1)]

			local ad = {
				station  = station,
				flavour  = flavour,
				slogan   = slogan,
				ispolice = ispolice,
				trader   = Character.New({title = flavour, armour=false}, r),
				title    = title
			}

			ad.stock = {}
			ad.price = {}
			for key, comm in pairs(Commodities) do
				if not Game.system:IsCommodityLegal(key) then
					ad.stock[comm] = Engine.rand:Integer(1,50)
					-- going rate on the black market will be twice normal
					ad.price[comm] = ad.station:GetCommodityPrice(comm) * 2
				end
			end

			local ref = ad.station:AddAdvert({
				title       = ad.title,
				description = ad.flavour,
				icon        = "goods_trader",
				onChat      = onChat,
				onDelete    = onDelete})
			ads[ref] = ad
		end
	end
end

local loaded_data

local onGameStart = function ()
	ads = {}

	if not loaded_data or not loaded_data.ads then return end

	for k,ad in pairs(loaded_data.ads) do
		local ref = ad.station:AddAdvert({
			title       = ad.title,
			description = ad.flavour,
			icon        = "goods_trader",
			onChat      = onChat,
			onDelete    = onDelete})
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

Event.Register("onCreateBB", onCreateBB)
Event.Register("onGameStart", onGameStart)

Serializer:Register("GoodsTrader", serialize, unserialize)
