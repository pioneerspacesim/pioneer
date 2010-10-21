-- ass flavours indeed ;-)
local ass_flavours = {
	{
		adtext = "~~UNFINISHED MISSION - WON'T WORK ~~ WANTED: Removal of %1 from the %2 system.",
		introtext = "Hi, I'm %1. I'll pay you %2 to get rid of %3.",
		wheretext = "%1 will be leaving %2 at %3.",
		successmsg = "News of %1's long vacation gratefully received. Well done, I have initiated your full payment.",
		failuremsg = "I am most displeased to find that %1 is still alive. Needless to say you will receive no payment.",
		danger = 0,
		time = 3,
		money = 1.0,
	}
}

Module:new {
	__name = 'Assassination',

	Init = function(self)
		self:EventListen("onCreateBB")
		self:EventListen("onUpdateBB")
		self:EventListen("onEnterSystem")
		self:EventListen("onPlayerDock")
		self.ads = {}
		self.missions = {}
	end,

	Unserialize = function(self, data)
		Module.Unserialize(self, data)
		for i,mission in pairs(self.missions) do
			self:_setupTimersForMission(mission)
		end
	end,

	GetPlayerMissions = function(self)
		return self.missions
	end,

	_TryAddAdvert = function(self, station)
		local gender = Pi.rand:Int(0,1) == 1
		local targetGender = Pi.rand:Int(0,1) == 1
		local flavour = Pi.rand:Int(1, #ass_flavours)
		local title = { "Senator", "General", "Colonel", "Comandante",
"Cardinal", "Professor", "Ambassador", "Judge" }
		ad = {
			flavour = flavour,
			targetGender = targetGender,
			target = title[Pi.rand:Int(1, #title)] .. " " .. Pi.rand:PersonName(gender),
			clientGender = gender,
			client = Pi.rand:PersonName(gender),
			reward = Pi.rand:Real(2000, 15000) * ass_flavours[flavour].money,
			due = Pi.GetGameTime() + Pi.rand:Real(0, ass_flavours[flavour].time * 60*60*24*31),
			bb = station,
			location = Pi.GetCurrentSystem():GetRandomStarportNearButNotIn(),
			id = #self.ads+1
		}
		-- if we found a location
		if ad.location ~= nil then
			table.insert(self.ads, ad)
			local addescription = _(ass_flavours[ad.flavour].adtext, {
					ad.target, ad.location:GetSystemName() } )
			station:SpaceStationAddAdvert(self.__name, #self.ads, addescription)
		end
	end,

	onCreateBB = function(self, args)
		local station = args[1]
		for i = 1,10 do --Pi.rand:Int(0, 5) do
			self:_TryAddAdvert(station)
		end
	end,

	_launchTargetShip = function(mission)
		print("Hm. should launch ship now: " .. mission.target_ship:GetLabel())
		mission.target_ship:ShipAIDoKill(Pi.GetPlayer())
	end,

	_setupTimersForMission = function(self, mission)
		if mission.status == 'active' and
		   mission.target_ship ~= nil and
		   mission.due > Pi.GetGameTime() then
			print("Adding timer for " .. Date.Format(mission.due))
			Pi.AddTimer(mission.due, self._launchTargetShip, mission)
		end
	end,

	onEnterSystem = function(self)
		for k,mission in pairs(self.missions) do
			if mission.status == 'active' and
			   mission.location:GetSystem() == Pi:GetCurrentSystem() then
				ship, e = Pi.SpawnRandomDockedShip(Pi.FindBodyForSBody(mission.location), 10, 50, 500)
				ship:SetLabel("Bag 'o shit")
				mission.target_ship = ship
				self:_setupTimersForMission(mission)
			end
		end
	end,

	onPlayerDock = function(self)
--[[		local station = Pi.GetPlayer():GetDockedWith():GetSBody()
		print('player docked with ' .. station:GetBodyName())
		for k,mission in pairs(self.missions) do
			if mission.status == 'active' then
				if mission.location == station then
					if Pi.GetGameTime() > mission.due then
						Pi.ImportantMessage(ass_flavours[mission.flavour].failuremsg, mission.client)
						mission.status = 'failed'
					else
						Pi.ImportantMessage(ass_flavours[mission.flavour].successmsg, mission.client)
						Pi.GetPlayer():AddMoney(mission.reward)
						mission.status = 'completed'
					end
				elseif Pi.GetGameTime() > mission.due then
					mission.status = 'failed'
				end
			end
		end
--]]	end,
	
	onUpdateBB = function(self, args)
		local station = args[1]
		for k,ad in pairs(self.ads) do
			if (ad.bb == station) and (ad.due < Pi.GetGameTime() + 60*60*24*1) then
				self.ads[k] = nil
				ad.bb:SpaceStationRemoveAdvert(self.__name, ad.id)
			end	
		end
		if Pi.rand:Int(0,12*60*60) < 60*60 then -- roughly once every twelve hours
			self:_TryAddAdvert(station)
		end
	end,
	
	onChatBB = function(self, dialog, optionClicked)
		local ad = self.ads[dialog:GetAdRef()]
		dialog:Clear()
		if optionClicked == -1 then
			dialog:Close()
			return
		elseif optionClicked == 0 then
			dialog:SetMessage(_(ass_flavours[ad.flavour].introtext, {
				ad.client, format_money(ad.reward), ad.target }))
		elseif optionClicked == 1 then
			dialog:SetMessage(_("%1 will be leaving %2 in the %3 system at %4.",
					{ ad.target, ad.location:GetBodyName(),
					ad.location:GetSystemName(), Date.Format(ad.due) }
			))
		elseif optionClicked == 2 then
			dialog:SetMessage(_('It must be done after %1 leaves %2. Do not miss this opportunity.',
					{ ad.target, ad.location:GetBodyName() }))
		elseif optionClicked == 3 then
			dialog:RemoveAdvertOnClose()
			self.ads[ad.id] = nil
			ad.description = _("Kill %1 at %2 in the %3 system.",
					{ ad.target, ad.location:GetBodyName(), ad.location:GetSystemName() })
			ad.status = "active"
			table.insert(self.missions, ad)
			dialog:SetMessage("Excellent.")
			dialog:AddOption("Hang up.", -1)
			return
		end
		dialog:AddOption(_("Where can I find %1?", {ad.target}), 1);
		dialog:AddOption("Could you repeat the original request?", 0);
		dialog:AddOption("How soon must it be done?", 2);
		dialog:AddOption("Ok, agreed.", 3);
		dialog:AddOption("Hang up.", -1);
	end,
}
--]]
