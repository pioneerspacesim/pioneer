
local crank_flavours = {
	{
		bbmsg = "DONATE! The Church of The Celestial Flying Spaghetti Monster needs YOUR money to spread the word of god.",
		dlgmsg = "Please select an amount to donate to the Church of the Celestial Flying Spaghetti Monster.\n",
	},{
		bbmsg = "DONATE. The Guardians of the Free Spirit humbly request your charity to support our monasteries.",
		dlgmsg = "Peace be with you, brother. Please select an amount to donate to the Guardians of the Free Spirit.\n",
	},{
		bbmsg = "FEELING GENEROUS? War Orphan's Support needs your help to keep up its essential work.",
		dlgmsg = "Please select an amount to donate to War Orphan's Support, and end the suffering of children all over the galaxy.\n"
	}
}

Module:new {
	__name='DonateToCranks', 
	
	Init = function(self)
		--self:EventListen("onCreateBB")
		--self:EventListen("onUpdateBB")
		self.ads = {}
	end,

	onCreateBB = function(self, args)
		local station = args[1]
		
		local t = Pi.rand:Int(1, #crank_flavours)
		table.insert(self.ads, {id=#self.ads+1, bb=station, flavour=t})
		station:SpaceStationAddAdvert(self.__name, #self.ads, crank_flavours[t].bbmsg)

		t = Pi.rand:Int(1, #crank_flavours)
		table.insert(self.ads, {id=#self.ads+1, bb=station, flavour=t})
		station:SpaceStationAddAdvert(self.__name, #self.ads, crank_flavours[t].bbmsg)
	end,

	onUpdateBB = function(self, args)
		-- insert or delete new ads at random
		--print("Updating bb adverts for " .. args[1]:GetLabel())
	end,

	onChatBB = function(self, dialog, optionClicked)
		local ad_ref = dialog:GetAdRef()
		local t = self.ads[ad_ref].flavour

		if optionClicked == 0 then
			dialog:Clear()
			print("dialog stage is " .. dialog:GetStage())
			--dialog:SetTitle(crank_flavours[t].bbmsg)
			dialog:SetMessage(crank_flavours[t].dlgmsg)
			dialog:AddOption("$1", 1);
			dialog:AddOption("$10", 10);
			dialog:AddOption("$100", 100);
			dialog:AddOption("$1000", 1000);
			dialog:AddOption("$10000", 10000);
			dialog:AddOption("$100000", 100000);
			dialog:AddOption("Hang up.", -1);
		elseif optionClicked == -1 then
			dialog:Close()
		else
			local player = Pi.GetPlayer()
			if player:GetMoney() < optionClicked then
				Pi.Message("", "You do not have enough money.")
			else
				if optionClicked > 10000 then
					Pi.Message("", "Wow! That was very generous.")
				else
					Pi.Message("", "Thank you. All donations are welcome.")
				end
				player:SetMoney( player:GetMoney() - optionClicked )
				dialog:UpdateBaseDisplay()
			end
		end
	end,
}

