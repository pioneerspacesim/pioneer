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
		local flavour = rand:Int(1, #goods_trader_flavour)
		local ad = {
			client = rand:Surname(isfemale),
			flavour = flavour,
			bb = station,
			id = #self.ads+1
		}
		local description = _(goods_trader_flavour[flavour], {ad.client})
		table.insert(self.ads, ad)
		station:SpaceStationAddAdvert(self.__name, #self.ads, description)
	end,

	onCreateBB = function(self, args)
		print("Creating bb adverts for " .. args[1]:GetLabel())
		local station = args[1]
		local seed = station:GetSBody():GetSeed()
		local rand = Rand:new(seed)
		self:_create_advert(rand, station)
		self:_create_advert(rand, station)
	end,

	DialogHandler = function(self, dialog, optionClicked)
		local ad_ref = dialog:GetAdRef()
		print("dialog handler for " .. ad_ref .. " clicked " .. optionClicked)
		if optionClicked == -1 then
			dialog:Close()
			self.ads[ad_ref].bb:SpaceStationRemoveAdvert(self.__name, ad_ref)
		else
			self.ads[ad_ref].stock = {[Equip.WATER]=20, [Equip.HYDROGEN]=15, [Equip.NERVE_GAS]=0}
			self.ads[ad_ref].price = {[Equip.WATER]=120, [Equip.HYDROGEN]=130, [Equip.NERVE_GAS]=1000}
			dialog:Clear()
			print("dialog stage is " .. dialog:GetStage())
			dialog:SetStage("blah")
			dialog:SetTitle("Hello old beans!")
			dialog:SetMessage("Hello you plenty good chap, blah blah")
			dialog:AddTraderWidget()
			dialog:AddOption("Click this and see", 1);
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

--	TraderOnClickSell = function(self, dialog, comType)
--		dialog:Close()
--		return false
--	end,
}

