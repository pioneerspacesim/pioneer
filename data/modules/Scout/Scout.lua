-- Get the translator function
local t = Translate:GetTranslator()

 -- don't produce missions for further than this many light years away
    local max_scout_dist = 30
 -- typical time for travel to a system max_scout_dist away
    local typical_travel_time = 0.9 * max_scout_dist * 24 * 60 * 60 * 2
 -- typical reward for delivery to a system max_scout_dist away
    local typical_reward = 25 * max_scout_dist
 -- local MissionState = 0

local ads = {}
local missions = {}

local onChat = function (form, ref, option)
	local ad = ads[ref]

	form:Clear()

	if option == -1 then
		form:Close()
		return
	end

	if option == 0 then
		form:SetFace({ female = ad.isfemale, seed = ad.faceseed, name = ad.client })

		local sys   = ad.location:GetStarSystem()
		local sbody = ad.location:GetSystemBody()
		local scout_flavours = Translate:GetFlavours('Scout')
		local introtext = string.interp(scout_flavours[ad.flavour].introtext, {
			name     = ad.client,
			cash     = Format.Money(ad.reward),
			starport = sbody.name,
			systembody = sbody.name,
			system   = sys.name,
			sectorx  = ad.location.sectorX,
			sectory  = ad.location.sectorY,
			sectorz  = ad.location.sectorZ,
		})
		form:SetMessage(introtext)

	elseif option == 1 then
		local scout_flavours = Translate:GetFlavours('Scout')
		form:SetMessage(scout_flavours[ad.flavour].whysomuchtext)

	elseif option == 2 then
		form:SetMessage(t("I need the information by ")..Format.Date(ad.due))

	elseif option == 4 then
		if ad.risk <= 0.1 then
			form:SetMessage(t("I suspect that there is some unregistered activity going on. Nothing big probably, but you'd better be prepared."))
		elseif ad.risk > 0.1 and ad.risk <= 0.3 then
			form:SetMessage(t("This is just a routine check. If there was a substantial risk, I think we would have heard of attacks in the area."))
		elseif ad.risk > 0.3 and ad.risk <= 0.6 then
			form:SetMessage(t("A ship has vanished in the area. I suspect pirate activity."))
		elseif ad.risk > 0.6 and ad.risk <= 0.8 then
			form:SetMessage(t("Several ships have been lost in the area, including my last scout. I really need to know what's going on."))
		elseif ad.risk > 0.8 and ad.risk <= 1 then
			form:SetMessage(t("I have reports from passing ships that confirm pirate attacks. What I need to know is how strong they are. You are certain to meet hostiles."))
		end

	elseif option == 3 then
		form:RemoveAdvertOnClose()

		ads[ref] = nil

		local mission = {
			type	 = t("Recon"),
			client	 = ad.client,
			location = ad.location,
			risk	 = ad.risk,
			reward	 = ad.reward,
			due	 = ad.due,
			flavour	 = ad.flavour,
			state = 0
		}

		local mref = Game.player:AddMission(mission)
		missions[mref] = mission
		form:SetMessage(t("Excellent. I will await your report."))
		form:AddOption(t('HANG_UP'), -1)
		return
	end

	form:AddOption(t("Why so much money?"), 1)
	form:AddOption(t("When do you need the data?"), 2)
	form:AddOption(t("Will I be in any danger?"), 4)
	form:AddOption(t("Could you repeat the original request?"), 0)
	form:AddOption(t("Ok, agreed."), 3)
	form:AddOption(t('HANG_UP'), -1)
end

local onDelete = function (ref)
	ads[ref] = nil
end

local makeAdvert = function (station)
	local scout_flavours = Translate:GetFlavours('Scout')
	local reward, due, location, nearbysystem
	local isfemale = Engine.rand:Integer(1) == 1
	local client = NameGen.FullName(isfemale)
	local flavour = Engine.rand:Integer(1,#scout_flavours)
	local urgency = scout_flavours[flavour].urgency
	local risk = scout_flavours[flavour].risk

	if scout_flavours[flavour].localscout == 1 then
		nearbysystem = Game.system
		local nearbystations = Game.system:GetBodyPaths()
		local HasPop = 1
		while HasPop > 0 do
			location = nearbystations[Engine.rand:Integer(1,#nearbystations)]
			CurBody = location:GetSystemBody()
			if CurBody.superType ~= "STARPORT" and CurBody.superType ~= "GAS_GIANT" then
				HasPop = 0
			end
		end
		local locdist = Space.GetBody(location.bodyIndex)
		local dist = station:DistanceTo(locdist)
		if dist < 1000 then return end
		reward = 25 + (math.sqrt(dist) / 15000) * (1+urgency)
		due = Game.time + ((4*24*60*60) * (Engine.rand:Number(1.5,3.5) - urgency))
	else
		local nearbysystems = Game.system:GetNearbySystems(max_scout_dist, function (s) return #s:GetStationPaths() > 0 end)
		if #nearbysystems == 0 then return end
		nearbysystem = nearbysystems[Engine.rand:Integer(1,#nearbysystems)]
		local dist = nearbysystem:DistanceTo(Game.system)
		local nearbystations = nearbysystem:GetBodyPaths()
		local HasPop = 1
		while HasPop > 0 do
			location = nearbystations[Engine.rand:Integer(1,#nearbystations)]
			local CurBody = location:GetSystemBody()
			if CurBody.superType ~= "STARPORT" and CurBody.superType ~= "GAS_GIANT" then
				HasPop = 0
			end
		end
		reward = ((dist / max_scout_dist) * typical_reward * (1+risk) * (1.5-urgency) * Engine.rand:Number(0.8,1.2))
		due = Game.time + ((dist / max_scout_dist) * typical_travel_time * (1.5-urgency) * Engine.rand:Number(0.9,1.1))
	end

	local ad = {
		station		= station,
		flavour		= flavour,
		client		= client,
		location	= location,
		due		= due,
		risk		= risk,
		urgency		= urgency,
		reward		= reward,
		isfemale	= isfemale,
		faceseed	= Engine.rand:Integer(),
	}

	local sbody = ad.location:GetSystemBody()

	ad.desc = string.interp(scout_flavours[flavour].adtext, {
		system	= nearbysystem.name,
		cash	= Format.Money(ad.reward),
		starport = sbody.name,
	})

	local ref = station:AddAdvert(ad.desc, onChat, onDelete)
	ads[ref] = ad
end

local onCreateBB = function (station)
	local num = Engine.rand:Integer(0, math.ceil(Game.system.population))
	for i = 1,num do
		makeAdvert(station)
	end
end

local onUpdateBB = function (station)
	for ref,ad in pairs(ads) do
		local scout_flavours = Translate:GetFlavours('Scout')
		if scout_flavours[ad.flavour].localscout == 0
			and ad.due < Game.time + 5*60*60*24 then -- five day timeout for inter-system
			ad.station:RemoveAdvert(ref)
		elseif scout_flavours[ad.flavour].localscout == 1
			and ad.due < Game.time + 2*60*60*24 then -- two day timeout for locals
			ad.station:RemoveAdvert(ref)
		end
	end
	if Engine.rand:Integer(12*60*60) < 60*60 then -- roughly once every twelve hours
		makeAdvert(station)
	end
end

--[[local GetDistToTgt = function ()
  
	local Dist = MissBody:DistanceTo(Game.player)
	if MissBody:isa("SystemBody") then
		if Dist < MissBody.radius * 1.25 then
			UI.ImportantMessage("distance reached", "computer")
		end
	end

end]]--

local onFrameChanged = function (body)
	if body:isa("Ship") and body:IsPlayer() then
		for ref,mission in pairs(missions) do
			local CurBody = body.frameBody
			local PhysBody = CurBody.path:GetSystemBody()
			if CurBody.path == mission.location then
				local ShouldSpawn
				local TimeUp = 0
				local ShipSpawned = false
				Timer:CallEvery(10, function ()
					local MinChance = 0
					local Dist = CurBody:DistanceTo(Game.player)
					if Dist < PhysBody.radius * 1.3 and mission.state == 0 then
						UI.ImportantMessage(t("Distance reached, starting long range sensor sweep. Maintain orbit for at least 60 minutes"), t("computer"))
						mission.state = 1
					end
					if Dist > PhysBody.radius * 1.4 and mission.state == 1 then
						UI.ImportantMessage(t("sensor sweep interrupted, too far from target!"), t("computer"))
						mission.state = 0
						TimeUp = 0
					end
					if mission.state == 1 then
						TimeUp = TimeUp + 10
						if not ShipSpawned then
							ShouldSpawn = Engine.rand:Number(MinChance, 1)
							-------------------------------------------------------------
							if 	ShouldSpawn > 0.9 then
								ShipSpawned = true
								local scout_flavours = Translate:GetFlavours('Scout')
								local risk = scout_flavours[mission.flavour].risk
								local ships = 0
								local riskmargin = Engine.rand:Number(-0.3,0.3) -- Add some random luck
								if risk >= (1 + riskmargin) then ships = 3
								elseif risk >= (0.7 + riskmargin) then ships = 2
								elseif risk >= (0.5 + riskmargin) then ships = 1
								end
								-- if there is some risk and still no ships, flip a tricoin
								if ships < 1 and risk >= 0.2 and Engine.rand:Integer(2) == 1 then ships = 1 end
									local shiptypes = ShipType.GetShipTypes('SHIP', function (t)
									local mass = t.hullMass
									return mass >= 100 and mass <= 400
									end)
								if #shiptypes == 0 then return end
								local ship
								while ships > 0 do
									ships = ships-1
									if Engine.rand:Number(1) <= risk then
										local shipname = shiptypes[Engine.rand:Integer(1,#shiptypes)]
										local shiptype = ShipType.GetShipType(shipname)
										local default_drive = shiptype.defaultHyperdrive
										local max_laser_size = shiptype.capacity - EquipType.GetEquipType(default_drive).mass
										local lasers = EquipType.GetEquipTypes('LASER', function (e,et)
										return et.mass <= max_laser_size and string.sub(e,0,11) == 'PULSECANNON'
										end)
										local laser = lasers[Engine.rand:Integer(1,#lasers)]
										ship = Space.SpawnShipNear(shipname,Game.player, 10, 15)
										ship:AddEquip(default_drive)
										ship:AddEquip(laser)
										ship:AIKill(Game.player)
									end
								end
								if ship then
									local pirate_greeting = string.interp(t('PIRATE_TAUNTS')[Engine.rand:Integer(1,#(t('PIRATE_TAUNTS')))], {
									client = mission.client, location = mission.location:GetSystemBody().name,})
									UI.ImportantMessage(pirate_greeting, ship.label)
								end
							end
							-------------------------------------------------------------
							if not ShipSpawned then
								MinChance = MinChance + 0.1
							end
						end
						if TimeUp > 3600 then
							mission.state = 2
							UI.ImportantMessage(t("Sensor sweep complete, data stored."), t("computer"))
						end
					end
					if mission.state == 2 then
						return true
					end
				end)
			end
		end
	end
end

local onShipDocked = function (player, station)
	if not player:IsPlayer() then return end
	for ref,mission in pairs(missions) do
		if Game.time > mission.due then
			mission.state = 3
		end
		if mission.state == 2 then
			local scout_flavours = Translate:GetFlavours('Scout')
			UI.ImportantMessage(scout_flavours[mission.flavour].successmsg, mission.client)
			player:AddMoney(mission.reward)
			player:RemoveMission(ref)
			missions[ref] = nil
		elseif mission.state == 3 then
			local scout_flavours = Translate:GetFlavours('Scout')
			UI.ImportantMessage(scout_flavours[mission.flavour].failuremsg, mission.client)
			player:RemoveMission(ref)
			missions[ref] = nil
		end
	end
end

local loaded_data

local onGameStart = function ()
	ads = {}
	missions = {}
	if not loaded_data then return end
	for k,ad in pairs(loaded_data.ads) do
		local ref = ad.station:AddAdvert(ad.desc, onChat, onDelete)
		ads[ref] = ad
	end
	for k,mission in pairs(loaded_data.missions) do
		local mref = Game.player:AddMission(mission)
		missions[mref] = mission
	end
	loaded_data = nil
end

local serialize = function ()
	return { ads = ads, missions = missions }
end

local unserialize = function (data)
	loaded_data = data
end

EventQueue.onCreateBB:Connect(onCreateBB)
EventQueue.onUpdateBB:Connect(onUpdateBB)
EventQueue.onFrameChanged:Connect(onFrameChanged)
EventQueue.onShipDocked:Connect(onShipDocked)
EventQueue.onGameStart:Connect(onGameStart)

Serializer:Register("ScoutArea", serialize, unserialize)
