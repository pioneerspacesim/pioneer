
Module:new {
	__name='mymod', 
	x=123,
	
	Init = function(self)
		print('init() mymod')
		print(self.x)
		self:EventListen("onPlayerChangeTarget")
		self:EventListen("onShipKilled")
		self:EventListen("onCreateBB")
		self:EventListen("onUpdateBB")
		self.ads = {}
	end,

	GetPlayerMissions = function(self)
		return { { description="Hello world", client="Mr Morton", reward=1234, status='completed' },
	                 { description="Eat your own head", client="God", reward=500, status='failed' } }
	end,

	DoSomething = function(self)
		print('hi')
	end,

	onPlayerChangeTarget = function(self)
		print('mymod got onPlayerChangeTarget');
		print(Pi.GetPlayer():GetLabel())
		SoundEvent:new():Play("Landing.wav", 1, 1, 0)
		print(Pi.GetGameTime())
		--self:EventIgnore("onPlayerChangeTarget")
	end,

	onShipKilled = function(self, args)
		print(args["type"])
		for i,j in pairs(args) do print(i,j) end
		print(args[1]:GetLabel() .. " was killed by " ..  args[2]:GetLabel())
		collectgarbage("collect")
	end,

	onCreateBB = function(self, args)
		print("Creating bb adverts for " .. args[1]:GetLabel())
		local station = args[1]
		
		table.insert(self.ads, {id=#self.ads+1, bb=station})
		station:SpaceStationAddAdvert(self.__name, #self.ads, "Click me!")

		table.insert(self.ads, {id=#self.ads+1, bb=station})
		station:SpaceStationAddAdvert(self.__name, #self.ads, "Another advert")
	end,

	onUpdateBB = function(self, args)
		-- insert or delete new ads at random
		print("Updating bb adverts for " .. args[1]:GetLabel())
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

