local goods_trader_flavour = {
	"Honest %1's Goods Emporium",
	"%1's Trading House",
	"%1's Warehouse",
	"%1's Goods Exchange",
	"%1 Holdings",
	"%1 & Sons"
}

Module:new {
	__name='GoodsTrader', 
	x=123,
	
	Init = function(self)
		self:EventListen("onCreateBB")
		self.ads = {}
	end,

	_create_advert = function(self, rand, station)
		local isfemale = rand:Int(0, 1) == 1
		local ispolice = rand:Int(0, 1) == 1
		local flavour = rand:Int(1, #goods_trader_flavour)
		local ad = {
			client = rand:Surname(isfemale),
			flavour = flavour,
			ispolice = ispolice,
			bb = station,
			id = #self.ads+1
		}
		ad.desc = _(goods_trader_flavour[flavour], {ad.client})
		table.insert(self.ads, ad)
		station:SpaceStationAddAdvert(self.__name, #self.ads, ad.desc)
	end,

	onCreateBB = function(self, args)
		--print("Creating bb adverts for " .. args[1]:GetLabel())
		local station = args[1]
		local seed = station:GetSBody():GetSeed()
		local rand = Rand:new(seed)
		local num = rand:Int(1,3)
		for i = 1,num do
			self:_create_advert(rand, station)
		end
	end,

	onChatBB = function(self, dialog, optionClicked)
		local ad_ref = dialog:GetAdRef()
		local ad = self.ads[ad_ref]
		print("dialog handler for " .. ad_ref .. " clicked " .. optionClicked)
		if optionClicked == -1 then
			dialog:Close()
		else
			self.ads[ad_ref].stock = {}
			self.ads[ad_ref].price = {}
			for e = Equip.FIRST_COMMODITY, Equip.LAST_COMMODITY do
				if not Pi.GetCurrentSystem():IsCommodityLegal(e) then
					self.ads[ad_ref].stock[e] = Pi.rand:Int(1,50)
					-- going rate on the black market will be twice normal
					self.ads[ad_ref].price[e] = 2*ad.bb:GetEquipmentPrice(e)
				end
			end
		--	self.ads[ad_ref].stock = {[Equip.WATER]=20, [Equip.HYDROGEN]=15, [Equip.NERVE_GAS]=0}
		--	self.ads[ad_ref].price = {[Equip.WATER]=120, [Equip.HYDROGEN]=130, [Equip.NERVE_GAS]=1000}
			dialog:Clear()
			dialog:SetTitle(ad.desc)
			dialog:SetMessage(_("Welcome to %1.", {ad.desc}))
			dialog:AddTraderWidget()
			dialog:AddOption("Hang up.", -1);
		end
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
			Pi.AddPlayerCrime(Pi.Crime.CRIME_TRADING_ILLEGAL_GOODS, 400*(2-lawlessness))
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

