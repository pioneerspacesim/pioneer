
local t = Translate:GetTranslator()

local ads = {}
local stockneeded = {}

local onChat = function (form, ref, option)
	local ad = ads[ref]
	local commodityneeded = EquipType.GetEquipType(ad.commodityneeded).name
	
	local setMessage = function (message)
		form:SetMessage(message:interp({
			CommodityNeeded = commodityneeded,
			Price = ad.commodityneededprice,
			Name = ad.Trader.name,
		}))
	end

	form:Clear()
	if option == -1 then
		form:Close()
		return
	end
	if option == 0 then
		form:Clear()
		form:SetTitle(string.interp(t('{CommodityNeeded} needed.'), {CommodityNeeded = commodityneeded}))
		form:SetFace(ad.Trader)
		setMessage(t("Hi, I'm {Name}. I'll need {CommodityNeeded}, I will pay {Price} for ton."))
		form:AddOption((t('Sell {CommodityNeeded}'):interp({CommodityNeeded = commodityneeded})),1)
		form:AddOption(t('HANG_UP'),-1)
	end
	if option == 1 then
		form:Clear()
		form:AddGoodsTrader({
			canTrade = function (ref, commodity)
				return ad.stock[commodity]
			end,
			getStock = function (ref, commodity)
				return ad.stock[commodity]
			end,
			getPrice = function (ref, commodity)
				return ad.price[commodity]
			end,
			onClickBuy = function (ref, commodity)
				return ad.price[commodity]
			end,
			onClickSell = function (ref, commodity)
				return ad.price[commodity]
			end,
			bought = function (ref, commodity)
				ad.stock[commodity] = ad.stock[commodity] + 1
			end,
			sold = function (ref, commodity)
				ad.stock[commodity] = ad.stock[commodity] - 1
			end,
		})
	end
end

local onDelete = function (ref)
	ads[ref].Trader:Save()
	ads[ref].Trader.lastSavedSystemPath = ads[ref].station.path
	ads[ref] = nil
end

local makeAdvert = function (station)

	if #stockneeded == 0 then return end
	
	local commodityneeded = stockneeded
	local Trader = Character.New()
	local commodityneededprice = station:GetEquipmentPrice(commodityneeded) * (1 + (Engine.rand:Integer(5, 10)) / 10)
	local stock = {[commodityneeded] = 0}
	local price = {[commodityneeded] = commodityneededprice}
	local advert = string.interp(t('{CommodityNeeded} needed.'), {CommodityNeeded = EquipType.GetEquipType(commodityneeded).name})

	local ad = {
		adverttime = Game.time + 24*60*60,
		Trader = Trader,
		advert = advert,
		price = price,
		stock = stock,
		commodityneeded = commodityneeded,
		commodityneededprice = commodityneededprice,
		station = station,
	}

	local ref = station:AddAdvert(ad.advert, onChat, onDelete)
	ads[ref] = ad
end

local onShipDocked = function (player, station)
	if not player:IsPlayer() then return end
	
	if Engine.rand:Integer(1,3) == 1 then
		local stock = {}
		for k,v in pairs(Game.system:GetCommodityBasePriceAlterations()) do
			if v > 10	then
				table.insert(stock, k)
				elseif v > 10 == nil and v > 2 then
				table.insert(stock, k)
				elseif v > 10 == nil and v > 2 == nil then
				table.insert(stock, k)
			end
		end
		if #stock == 0 then return end
		stockneeded = stock[Engine.rand:Integer(1, #stock)]
		-- station:SetEquipmentPrice(stockneeded, 0)
		makeAdvert(station)
	end
end

local onUpdateBB = function (station)
	for ref,ad in pairs(ads) do
		if ad.adverttime < Game.time then
			ad.station:RemoveAdvert(ref)
		end
	end
end

local onShipUndocked = function (player, station)
	if not player:IsPlayer() then return end

	for ref,ad in pairs(ads) do
		ad.station:RemoveAdvert(ref)
	end
end

local loaded_data

local onGameStart = function ()
	ads = {}
	if not loaded_data then return end

	for k,ad in pairs(loaded_data.ads) do
		local ref = ad.station:AddAdvert(ad.advert, onChat, onDelete)
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

EventQueue.onShipDocked:Connect(onShipDocked)
EventQueue.onUpdateBB:Connect(onUpdateBB)
EventQueue.onShipUndocked:Connect(onShipUndocked)
EventQueue.onGameStart:Connect(onGameStart)

Serializer:Register("Bargain", serialize, unserialize)
