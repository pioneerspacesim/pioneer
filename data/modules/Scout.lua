-- Scout.lua by walterar <walterar2@gmail.com>
-- Licensed under the terms of the GPL v3. See GPL-3.txt

-- Get the translator function
local t = Translate:GetTranslator()
-- Get the UI class
local ui = Engine.ui

 -- don't produce missions for further than this many light years away
    local max_scout_dist = 30
 -- typical time for travel to a system max_scout_dist away
   local typical_travel_time = 0.9 * max_scout_dist * 24 * 60 * 60 * 2
 -- typical reward for delivery to a system max_scout_dist away
   local typical_reward = 200 * max_scout_dist
-- scanning time 3600 = 1 h
local scan_time = 600

local xTimeUp = 5

local ads = {}
local missions = {}

local onChat = function (form, ref, option)
  local ad          = ads[ref]
	local backstation = Game.player:GetDockedWith().path
	local faction     = Game.system.faction

	form:Clear()

	if option == -1 then
		form:Close()
		return
	end

	if option == 0 then
		form:SetFace(ad.client)

		local sys   = ad.location:GetStarSystem()
		local sbody = ad.location:GetSystemBody()

		local scout_flavours = Translate:GetFlavours('Scout')
		local introtext = string.interp(scout_flavours[ad.flavour].introtext, {
			name       = ad.client.name,
			faction    = faction.name,
			police     = faction.policeName,
			military   = faction.militaryName,
			cash       = Format.Money(ad.reward),
			systembody = sbody.name,
			system     = sys.name,
			sectorx    = ad.location.sectorX,
			sectory    = ad.location.sectorY,
			sectorz    = ad.location.sectorZ,
			dist       = string.format("%.2f", ad.dist),

		})
		form:SetMessage(introtext)

local introtext2 = string.interp(scout_flavours[ad.flavour].introtext2, {
			name       = ad.client.name,
			faction    = faction.name,
			police     = faction.policeName,
			military   = faction.militaryName,
			cash       = Format.Money(ad.reward),
			systembody = sbody.name,
			system     = sys.name,
			sectorx    = ad.location.sectorX,
			sectory    = ad.location.sectorY,
			sectorz    = ad.location.sectorZ,
		})

	elseif option == 1 then
		local scout_flavours = Translate:GetFlavours('Scout')
		form:SetMessage(scout_flavours[ad.flavour].whysomuchtext)

	elseif option == 2 then
		form:SetMessage(t("I need the information by ") .. Format.Date(ad.due))

	elseif option == 4 then

		if ad.risk < 0.3 then
			form:SetMessage(t("message risk 00-02-" .. Engine.rand:Integer(1,2)))
		elseif ad.risk >= 0.3 and ad.risk < 0.6 then
			form:SetMessage(t("message risk 03-05-1")) --" .. Engine.rand:Integer(1,x)))
		elseif ad.risk >= 0.6 and ad.risk < 0.9 then
			form:SetMessage(t("message risk 06-08-1")) --" .. Engine.rand:Integer(1,x)))
		elseif ad.risk >= 0.9 then
			form:SetMessage(t("message risk 09-10-" .. Engine.rand:Integer(1,2)))
		end

	elseif option == 5 then
			form:SetMessage(t("additional information"))

	elseif option == 3 then
		if Game.player:GetEquip('RADARMAPPER',1) == "NONE" then
			form:SetMessage(t("You have not installed RADAR MAPPER"))
			form:AddOption(t('HANG_UP'), -1)
			return
		end
		form:RemoveAdvertOnClose()

		ads[ref] = nil

		local mission = {
			type        = "Scout",
			faction     = faction.name,
			police      = faction.policeName,
			military    = faction.militaryName,
			backstation = backstation,
			client      = ad.client,
			location    = ad.location,
			risk        = ad.risk,
			reward      = ad.reward,
			due         = ad.due,
			flavour     = ad.flavour,
			state       = 0,
			status      = 'ACTIVE',
		}

		table.insert(missions,Mission.New(mission))

		form:SetMessage(t("Excellent. I await your report."))
		form:AddOption(t('HANG_UP'), -1)

		return
	end

	form:AddOption(t("Why so much money"), 1)
	form:AddOption(t("When do you need the data"), 2)
	form:AddOption(t("What is the risk"), 4)
	form:AddOption(t("Have additional information"), 5)
	form:AddOption(t("Repeat the original request"), 0)
	form:AddOption(t("Ok"), 3)
	form:AddOption(t('HANG_UP'), -1)
end

local onDelete = function (ref)
	ads[ref] = nil
end

local makeAdvert = function (station)
	local scout_flavours = Translate:GetFlavours('Scout')
	local reward, due, location, nearbysystem
	local client = Character.New()
	local flavour = Engine.rand:Integer(1,#scout_flavours)
	local urgency = scout_flavours[flavour].urgency
	local risk = scout_flavours[flavour].risk
	local	faction = Game.system.faction
-- local system
	if scout_flavours[flavour].localscout == 1 then
		nearbysystem = Game.system
		local nearbystations = nearbysystem:GetBodyPaths()
		local HasPop = 1
		while HasPop > 0 do
			if HasPop > #nearbystations then return end
			location = nearbystations[Engine.rand:Integer(1,#nearbystations)]
			local CurBody = location:GetSystemBody()
			if CurBody.superType == "ROCKY_PLANET"
				and CurBody.type ~= "PLANET_ASTEROID"
			then break end
			HasPop = HasPop + 1
		end
		local dist = station:DistanceTo(Space.GetBody(location.bodyIndex))
		if dist < 1000 then return end
		reward = 360 + (math.sqrt(dist) / 15000) * (1.5+urgency) * (1+nearbysystem.lawlessness) 
		due = Game.time + ((4*24*60*60) * (Engine.rand:Number(1.5,3.5) - urgency))
	else
-- remote system
		local nearbysystems =	Game.system:GetNearbySystems(max_scout_dist,
			function (s) return #s:GetBodyPaths() > 0 and s.population == 0 end)
		if #nearbysystems == 0 then return end
		nearbysystem = nearbysystems[Engine.rand:Integer(1,#nearbysystems)]
		local dist = nearbysystem:DistanceTo(Game.system)
		local nearbybodys = nearbysystem:GetBodyPaths()

		local HasPop = 1
		while HasPop > 0 do
			if HasPop > #nearbybodys then return end
			location = nearbybodys[Engine.rand:Integer(1,#nearbybodys)]
			local CurBody = location:GetSystemBody()
			if CurBody.superType == "ROCKY_PLANET"
				and CurBody.type ~= "PLANET_ASTEROID"
			then break end
			HasPop = HasPop + 1
		end
		reward = ((dist / max_scout_dist) * typical_reward * (1+risk) * (1.5-urgency) * Engine.rand:Number(0.8,1.2))
		due = Game.time + ((dist / max_scout_dist) * typical_travel_time * (1.5-urgency) * Engine.rand:Number(0.9,1.1))
	end

	local ad = {
		station  = station,
		flavour  = flavour,
		client   = client,
		location = location,
		dist     = Game.system:DistanceTo(location),
		due      = due,
		risk     = risk,
		urgency  = urgency,
		reward   = reward,
		isfemale = isfemale,
		faceseed = Engine.rand:Integer(),
	}

	local sbody = ad.location:GetSystemBody()

	ad.desc = string.interp(scout_flavours[flavour].adtext, {
		faction    = faction.name,
		police     = faction.policeName,
		military   = faction.militaryName,
		system     = nearbysystem.name,
		cash       = Format.Money(ad.reward),
		dist       = string.format("%.2f", ad.dist),
		systembody = sbody.name,
	})

	local ref = station:AddAdvert(ad.desc, onChat, onDelete)
	ads[ref] = ad
end

local onCreateBB = function (station)
	local num = Engine.rand:Integer(math.ceil(Game.system.population))
	for i = 1,num do
		makeAdvert(station)
	end
end

local onUpdateBB = function (station)
	for ref,ad in pairs(ads) do
		local scout_flavours = Translate:GetFlavours('Scout')
		if scout_flavours[ad.flavour].localscout == 0
			and ad.due < Game.time + 5*60*60*24 then
			ad.station:RemoveAdvert(ref)
		elseif scout_flavours[ad.flavour].localscout == 1
			and ad.due < Game.time + 2*60*60*24 then
			ad.station:RemoveAdvert(ref)
		end
	end
	if Engine.rand:Integer(12*60*60) < 60*60 then
		makeAdvert(station)
	end
end

local onFrameChanged = function (body)

	local	faction = Game.system.faction
	local mission
	
	if body:isa("Ship") and body:IsPlayer() then
		for ref,mission in pairs(missions) do
			if Game.time > mission.due then
				mission.state = 3
				mission.status = "FAILED"
				mission:Remove()
				missions[ref] = nil
				return
			end
			local CurBody = body.frameBody
			if CurBody == nil then return end
			local PhysBody = CurBody.path:GetSystemBody()
			if CurBody.path == mission.location then
				local ShouldSpawn
				local TimeUp = 0
				local ShipSpawned = false
				Timer:CallEvery(xTimeUp, function ()
					if not Game.player:IsPlayer() or CurBody == nil then return end
					local MinChance = 0
					local Dist = CurBody:DistanceTo(Game.player)
					if Dist < PhysBody.radius * 1.4 and mission.state == 0 then
						local lapse =	scan_time / 60
						Comms.ImportantMessage(t("Distance reached") .. lapse .. t("minutes"), t("computer"))
						mission.state = 1
					end
					if Dist > PhysBody.radius * 1.5 and mission.state == 1 then
						Comms.ImportantMessage(t("MAPPING interrupted."), t("computer"))
						mission.state = 0
						TimeUp = 0
					end
					if mission.state == 1 then
						TimeUp = TimeUp + xTimeUp
						if not ShipSpawned then
							ShouldSpawn = Engine.rand:Number(MinChance, 1)
							if 	ShouldSpawn > 0.8 then
								ShipSpawned = true
								local scout_flavours = Translate:GetFlavours('Scout')
								local risk = scout_flavours[mission.flavour].risk
--								local ships = math.floor((risk * 10) / 3) -- inplacable
								local ships = Engine.rand:Integer(math.floor((risk * 10) / 3)) -- Te sientes con suerte hoy?
								if ships < 1 and risk >= 0.1 and Engine.rand:Integer(2) == 1 then ships = 1 end
								local shiptypes =
									ShipType.GetShipTypes('SHIP', function (t)
											local mass = t.hullMass
--											return mass >= 30 and mass <= 60 -- peligroso :)
												return mass >= 100 and mass <= 400 -- más "jugable"
									end)
								if #shiptypes == 0 then ships = 0 end
-- ships = 0 -- anula ataques
								local ship
								while ships > 0 do
									ships = ships-1
									if Engine.rand:Number(1) <= risk then
										local shipname = shiptypes[Engine.rand:Integer(1,#shiptypes)]
										local shiptype = ShipType.GetShipType(shipname)
										local default_drive = shiptype.defaultHyperdrive
										local max_laser_size = shiptype.capacity - EquipType.GetEquipType(default_drive).mass
										local lasers = EquipType.GetEquipTypes('LASER', function (e,et)
											return et.mass <= max_laser_size and string.sub(e,0,11) == 'PULSECANNON' end)
										local laser = lasers[Engine.rand:Integer(1,#lasers)]
										ship = Space.SpawnShipNear(shipname,Game.player, 30, 60)
										ship:AddEquip(default_drive)
										ship:AddEquip(laser)
										ship:AIKill(Game.player)
									end
								end
								if ship then
									local hostile_greeting =
										string.interp(t('HostileMessages')[Engine.rand:Integer(1,#(t('HostileMessages')))],{
											client = mission.client.name, location = mission.location:GetSystemBody().name })
									Comms.ImportantMessage(hostile_greeting, ship.label)
								end
							end

							if not ShipSpawned then MinChance = MinChance + 0.1 end

						end
						if TimeUp > scan_time then
							mission.state = 2
							mission.status = "COMPLETED"
							Comms.ImportantMessage(t("COMPLETE MAPPING"), t("computer"))
-- decisión de destino de entrega
							local iflocal = Translate:GetFlavours('Scout')[mission.flavour].localscout
							if iflocal == 0 and (((mission.faction == faction.name) and Engine.rand:Integer(2) == 1)
															or Engine.rand:Integer(3) == 1) then
								local NewTargetSystems = Game.system:GetNearbySystems(Engine.rand:Integer(10,20),
									function (s) return #s:GetStationPaths() > 0 end)
								if #NewTargetSystems == 0 then
									mission.location = mission.backstation
									mission.status = "COMPLETED"
								else
									local NewTargetSystem = NewTargetSystems[Engine.rand:Integer(1,#NewTargetSystems)]
									local NewTargetStations = NewTargetSystem:GetStationPaths()
									mission.location = NewTargetStations[Engine.rand:Integer(1,#NewTargetStations)]
									mission.status = "COMPLETED"
									Comms.ImportantMessage(t("You will be paid on my behalf in new destination."),
	 									mission.client.name)
								end	
							
							else
								mission.location = mission.backstation
--								Mission.Update(ref,mission)
							end
----------------------
						end
					end
					if mission.state == 2 then return true end
				end)
			end
		end
	end
end

local onShipDocked = function (player, station)
	if not player:IsPlayer() then return end
	local mission
	local faction = Game.system.faction
	for ref, mission in pairs(missions) do
		if mission.state == 2 then
			if mission.faction == faction.name then
				if station.path == mission.location then
					local scout_flavours = Translate:GetFlavours('Scout')
					Comms.ImportantMessage((scout_flavours[mission.flavour].successmsg), mission.client.name)
					player:AddMoney(mission.reward)
					mission:Remove()
					missions[ref] = nil
				end
			else
				local multiplier = Game.system.lawlessness
				if multiplier < .02 then multiplier = 1 + multiplier end -- si son muy chantas te dejan seco
				local money = math.floor(Game.player:GetMoney() * multiplier)
				Game.player:AddCrime("TRADING_ILLEGAL_GOODS", money)
				Comms.ImportantMessage(t("Unauthorized data here is REMOVED") , faction.militaryName)
				Comms.ImportantMessage(t("You have been fined ") .. Format.Money(money), faction.policeName)
				mission:Remove()
				missions[ref] = nil
			end
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

	missions = loaded_data.missions
	loaded_data = nil
end

local onClick = function (mission)
	local scout_flavours = Translate:GetFlavours('Scout')

	if mission.status ~="COMPLETED" then

		return

		ui:Grid(2,1):SetColumn(0,{
			ui:VBox(10):PackEnd({
				ui:MultiLineText(
					(scout_flavours[mission.flavour].introtext):interp(
						{
							name       = mission.client.name,
							faction    = mission.faction,
							police     = mission.police,
							military   = mission.military,
							systembody = mission.location:GetSystemBody().name,
							system     = mission.location:GetStarSystem().name,
							sectorx    = mission.location.sectorX,
							sectory    = mission.location.sectorY,
							sectorz    = mission.location.sectorZ,
							dist       = math.ceil(Game.system:DistanceTo(mission.location)),
							cash       = Format.Money(mission.reward),
						})
					),
						ui:Grid(2,1):SetColumn(0,{ui:VBox():PackEnd(ui:MultiLineText(t("scoutmissiondetail")))})
						:SetColumn(1,{ui:VBox():PackEnd({
								ui:Label(mission.location:GetSystemBody().name),
								ui:Label(mission.location:GetStarSystem().name.." ("..mission.location.sectorX..","..mission.location.sectorY..","..mission.location.sectorZ..")"),
								ui:Label(Format.Date(mission.due)),
								ui:Margin(10),
								ui:Label(math.ceil(Game.system:DistanceTo(mission.location)).." "..t("ly"))
							})
						})
				})
		})
		:SetColumn(1,{ui:VBox(10):PackEnd(UI.InfoFace.New(mission.client))})
	else
		return

		ui:Grid(2,1):SetColumn(0,{
			ui:VBox(10):PackEnd({
				ui:MultiLineText(
					(scout_flavours[mission.flavour].introtext2):interp(
						{
							name       = mission.client.name,
							faction    = mission.faction,
							police     = mission.police,
							military   = mission.military,
							systembody = mission.location:GetSystemBody().name,
							system     = mission.location:GetStarSystem().name,
							sectorx    = mission.location.sectorX,
							sectory    = mission.location.sectorY,
							sectorz    = mission.location.sectorZ,
							cash       = Format.Money(mission.reward)
						})
					),
						ui:Grid(2,1):SetColumn(0,{ui:VBox():PackEnd(ui:MultiLineText(t("scoutmissiondetail2")))})
						:SetColumn(1,{ui:VBox():PackEnd({
								ui:Label(mission.location:GetSystemBody().name),
								ui:Label(mission.location:GetStarSystem().name.." ("..mission.location.sectorX..","..mission.location.sectorY..","..mission.location.sectorZ..")"),
								ui:Label(Format.Date(mission.due)),
								ui:Margin(10),
								ui:Label(math.ceil(Game.system:DistanceTo(mission.location)).." "..t("ly"))
							})
						})
				})
		})
		:SetColumn(1,{ui:VBox(10):PackEnd(UI.InfoFace.New(mission.client))})
	end
end

local serialize = function ()
	return { ads = ads, missions = missions }
end

local unserialize = function (data)
	loaded_data = data
end

Event.Register("onCreateBB", onCreateBB)
Event.Register("onUpdateBB", onUpdateBB)
Event.Register("onFrameChanged", onFrameChanged)
Event.Register("onShipDocked", onShipDocked)
Event.Register("onGameStart", onGameStart)

Mission.RegisterType('Scout','Scout',onClick)

Serializer:Register("Scout", serialize, unserialize)

