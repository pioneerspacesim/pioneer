
local t = Translate:GetTranslator()

local ads = {}

local onChat = function (form, ref, option)
	local ad = ads[ref]
	local commodityneeded = EquipType.GetEquipType(ad.commodityneeded).name
	
	local setMessage = function (message)
		form:SetMessage(message:interp({
			CommodityNeeded = commodityneeded,
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
		if ad.playerstock == 0 then
			setMessage(t('I need {CommodityNeeded}, but like i see you cannot help me.'))
			form:AddOption(t('HANG_UP'),-1)
			return
		end
		if ad.sellcount == ad.tonsneeded then
			form:SetMessage(t('Announcement outdated.'))
			form:AddOption(t('HANG_UP'),-1)
			form:RemoveAdvertOnClose()
			else
			setMessage(t("Hi, I'm {Name}. I'll need {CommodityNeeded}."))
			form:AddOption(t('Why not buy on the stock market?'),2)
			form:AddOption(t('HANG_UP'),-1)
			return
		end
	end
	if option == 2 then
		setMessage(ad.flavours[ad.flavour].whyneededtext)
		form:AddOption((t('Sell {CommodityNeeded}'):interp({CommodityNeeded = commodityneeded})),1)
		form:AddOption(t('GO_BACK'),0)
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
				return false
			end,
			onClickSell = function (ref, commodity)
				if ad.stock[commodity] == 0 then
   	 			return false
  	 		end
				return ad.price[commodity]
			end,
			bought = function (ref, commodity)
				ad.stock[commodity] = ad.stock[commodity] + 1
				ad.sellcount = ad.sellcount + 1
			end,
			sold = function (ref, commodity)
				return false
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
	local commodity = {}
	
	for k,v in pairs(Game.system:GetCommodityBasePriceAlterations()) do
		if v > 10	then
			table.insert(commodity, k)
			elseif v > 10 == nil and v > 2 then
			table.insert(commodity, k)
			elseif v > 10 == nil and v > 2 == nil then
			table.insert(commodity, k)
		end
	end
	if #commodity == 0 then return end
	
	local commodityneeded = commodity[Engine.rand:Integer(1, #commodity)]
	local flavours = Translate:GetFlavours('Bargain')
	local Trader = Character.New()
	local tonsneeded = Game.player:GetEquipCount('CARGO', commodityneeded)
	local flavour = Engine.rand:Integer(1,#flavours)
	local commodityneededprice = station:GetEquipmentPrice(commodityneeded) * (1 + flavours[flavour].bargain / 10)
	local stock = {[commodityneeded] = -tonsneeded}
	local price = {[commodityneeded] = commodityneededprice}
	local advert = string.interp(t('{CommodityNeeded} needed.'), {CommodityNeeded = EquipType.GetEquipType(commodityneeded).name})
	local playerstock = Game.player:GetEquipCount('CARGO', commodityneeded)
	local sellcount = sellcount or 0

	local ad = {
		adverttime = Game.time + 60*60,
		Trader = Trader,
		playerstock = playerstock,
		sellcount = sellcount,
		advert = advert,
		price = price,
		stock = stock,
		tonsneeded = tonsneeded,
		commodityneeded = commodityneeded,
		flavours = flavours,
		flavour = flavour,
		station = station,
	}

	local ref = station:AddAdvert(ad.advert, onChat, onDelete)
	ads[ref] = ad
end

local onCreateBB = function (station)
	if Engine.rand:Integer(1,2) == 1 then
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

EventQueue.onCreateBB:Connect(onCreateBB)
EventQueue.onUpdateBB:Connect(onUpdateBB)
EventQueue.onShipUndocked:Connect(onShipUndocked)
EventQueue.onGameStart:Connect(onGameStart)

Serializer:Register("Bargain", serialize, unserialize)
