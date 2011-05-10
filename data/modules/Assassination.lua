--[[
-- ass flavours indeed ;-)
local ass_flavours = {
	{
		adtext = "WANTED: Removal of %1 from the %2 system.",
		introtext = "Hi, I'm %1. I'll pay you %2 to get rid of %3.",
		wheretext = "%1 will be leaving %2 at %3.",
		successmsg = "News of %1's long vacation gratefully received. Well done, I have initiated your full payment.",
		failuremsg = "I am most displeased to find that %1 is still alive. Needless to say you will receive no payment.",
		danger = 0,
		time = 2,
		money = 1.0,
	},
	{
		adtext = "WANTED: Someone to kill %1 from the %2 system.",
		introtext = "I need %3 taken out of the picture. I'll pay you %2 to do this.",
		wheretext = "%1 will be leaving %2 at %3.",
		successmsg = "I am most sad to hear of %1's demise. You have been paid in full.",
		failuremsg = "I hear that %1 is in good health. This pains me.",
		danger = 0,
		time = 2,
		money = 1.0,
	},
	{
		adtext = "REMOVALS: %1 is no longer wanted in the %2 system.",
		introtext = "I am %1, and I will pay you %2 to terminate %3",
		wheretext = "%1 will be leaving %2 at %3.",
		successmsg = "You have been paid in full for the completion of that important contract.",
		failuremsg = "It is most regrettable that %1 is still live and well. You will receive no payment as you did not complete your contract.",
		danger = 0,
		time = 2,
		money = 1.0,
	}
}

Module:new {
	__name = 'Assassination',

	Init = function(self)
		--self:EventListen("onCreateBB")
		--self:EventListen("onUpdateBB")
		--self:EventListen("onEnterSystem")
		--self:EventListen("onPlayerDock")
		self.ads = {}
		self.missions = {}
	end,

	Unserialize = function(self, data)
		Module.Unserialize(self, data)
		for i,mission in pairs(self.missions) do
			self:_setupHooksForMission(mission)
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
			target_shipregid = Pi.RandomShipRegId(),
			clientGender = gender,
			client = Pi.rand:PersonName(gender),
			reward = Pi.rand:Real(2000, 15000) * ass_flavours[flavour].money,
			due = Pi.GetGameTime() + Pi.rand:Real(0, ass_flavours[flavour].time * 60*60*24*31),
			bb = station,
			location = Pi.GetCurrentSystem():GetRandomStarportNearButNotIn(),
			base = station:GetSBody(),
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

	onShipAttacked = function(self, args)
		-- When the player attacks the target, make it fight back
		local s = args[1]
		for k,mission in pairs(self.missions) do
			if mission.status == 'active' and
			   mission.target_ship == s then
				s:ShipAIDoKill(Pi.GetPlayer())
				-- stop listening for the event (our work is done)
				s:OnShipAttacked(self, nil)
			end
		end
	end,

	onShipKilled = function(self, args)
		local s = args[1]
		for k, mission in pairs(self.missions) do
			if mission.status == 'active' and
			   mission.target_ship == s and
			   mission.due < Pi.GetGameTime()
			then
				-- well done, comrade
				mission.status = 'completed'
				-- stop listening for event
				s:OnShipKilled(self, nil)
			end
		end	   
	end,

	_launchTargetShip = function(args)
		local self = args[1]
		local mission = args[2]
		print("Hm. should launch ship now: " .. mission.target_ship:GetLabel())
		-- send the target on a wee journey
		local destination = Pi.GetCurrentSystem():GetRandomStarportNearButNotIn()
		-- GetRandomStarportNearButNotIn can fail and return nil
		if destination ~= nil then
			print("Gave it a journey")
			mission.target_ship:ShipAIDoJourney(destination)
		else
			print("Fuck. just sending it off do fly around a turd")
			-- this should always work. the SBody parent of a
			-- ground or space station will be a planet. orbit it
			destination = mission.target_ship:GetDockedWith():GetSBody():GetParent()
			mission.target_ship:ShipAIDoMediumOrbit(Pi.FindBodyForSBody(destination))
			print("Orbiting " .. destination:GetBodyName())
		end
		-- Set up event hooks (onShipAttacked, onShipKilled, etc)
		self:_setupHooksForMission(mission)
	end,

	_setupHooksForMission = function(self, mission)
		if mission.status == 'active' and
		   mission.target_ship ~= nil then
			if mission.due > Pi.GetGameTime() then
				-- Target hasn't launched yet. set up a timer to do this
				print("Adding timer for " .. Date.Format(mission.due))
				Pi.AddTimer(mission.due, self._launchTargetShip, {self, mission})
			else
				-- Target has launched. set up event hooks
				mission.target_ship:OnShipAttacked(self, "onShipAttacked")
				mission.target_ship:OnShipKilled(self, "onShipKilled")
			end
		end
	end,

	onEnterSystem = function(self)
		for k,mission in pairs(self.missions) do
			if mission.status == 'active' then
				if mission.location:GetSystem() == Pi:GetCurrentSystem() then
					if mission.due > Pi.GetGameTime() then
						-- spawn our target ship
						ship, e = Pi.SpawnRandomDockedShip(Pi.FindBodyForSBody(mission.location), 10, 50, 500)
						ship:SetLabel(mission.target_shipregid)
						mission.target_ship = ship
						self:_setupHooksForMission(mission)
					else
						-- too late
						mission.status = 'failed'
					end
				elseif mission.target_ship then
					-- if we are in the wrong system, but target_ship is set indicating the 
					-- target has been spawned. This means we are probably leaving the target system
					if not mission.target_ship:IsValid() then
						-- The above condition is true if we are too late for the mission,
						-- or have hyperspaced away from our target and lost it (if we had
						-- followed it through hyperspace then IsValid() would be true
						mission.status = 'failed'
					else
						-- can still do mission (followed target through hyperspace)
						self:_setupHooksForMission(mission)
					end
				end
			end
		end
	end,

	onPlayerDock = function(self)
		local station = Pi.GetPlayer():GetDockedWith():GetSBody()
		for k,mission in pairs(self.missions) do
			if mission.base == station then
				if mission.status == 'completed' then
					Pi.ImportantMessage(mission.client, _(ass_flavours[mission.flavour].successmsg, {mission.target}))
					Pi.GetPlayer():AddMoney(mission.reward)
					-- erase the mission
					self.missions[k] = nil
				elseif mission.status == 'failed' then
					Pi.ImportantMessage(mission.client, _(ass_flavours[mission.flavour].failuremsg, {mission.target}))
					-- erase the mission
					self.missions[k] = nil
				end
			end
		end
	end,
	
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
		dialog:ClearOptions()
		if optionClicked == -1 then
			dialog:Close()
			return
		elseif optionClicked == 0 then
			dialog:SetMessage(_(ass_flavours[ad.flavour].introtext, {
				ad.client, format_money(ad.reward), ad.target }))
		elseif optionClicked == 1 then
			dialog:SetMessage(_("%1 will be leaving %2 in the %3 system at %4. The ship has registration id %5.",
					{ ad.target, ad.location:GetBodyName(),
					ad.location:GetSystemName(), Date.Format(ad.due), ad.target_shipregid }
			))
		elseif optionClicked == 2 then
			dialog:SetMessage(_('It must be done after %1 leaves %2. Do not miss this opportunity.',
					{ ad.target, ad.location:GetBodyName() }))
		elseif optionClicked == 3 then
			dialog:RemoveAdvertOnClose()
			self.ads[ad.id] = nil
			ad.description = _("Kill %1 (on the ship %6) at %2 in the %3 system. Return to %4 in the %5 system for payment.",
					{ ad.target, ad.location:GetBodyName(), ad.location:GetSystemName(),
				          ad.base:GetBodyName(), ad.base:GetSystemName(), ad.target_shipregid })
			ad.status = "active"
			table.insert(self.missions, ad)
			dialog:SetMessage("Excellent.")
			dialog:AddOption("Hang up.", -1)
			return
		elseif optionClicked == 4 then
			dialog:SetMessage("Return here on the completion of the contract and you will be paid.")
		end
		dialog:AddOption(_("Where can I find %1?", {ad.target}), 1);
		dialog:AddOption("Could you repeat the original request?", 0);
		dialog:AddOption("How soon must it be done?", 2);
		dialog:AddOption("How will I be paid?", 4);
		dialog:AddOption("Ok, agreed.", 3);
		dialog:AddOption("Hang up.", -1);
	end,
}
]]
