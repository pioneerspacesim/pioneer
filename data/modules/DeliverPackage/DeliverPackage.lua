-- Get the translator function
local t = Translate:GetTranslator()

-- don't produce missions for further than this many light years away
local max_delivery_dist = 30
-- typical time for travel to a system max_delivery_dist away
local typical_travel_time = 0.9 * max_delivery_dist * 24 * 60 * 60 
-- typical reward for delivery to a system max_delivery_dist away
local typical_reward = 25 * max_delivery_dist

local ads = {}
local missions = {}

local onChat = function (form, ref, option)
	local delivery_flavours = Translate:GetFlavours('DeliverPackage')
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

		local introtext = string.interp(delivery_flavours[ad.flavour].introtext, {
			name     = ad.client,
			cash     = Format.Money(ad.reward),
			starport = sbody.name,
			system   = sys.name,
			sectorx  = ad.location.sectorX,
			sectory  = ad.location.sectorY,
			sectorz  = ad.location.sectorZ,
		})

		form:SetMessage(introtext)

	elseif option == 1 then
		form:SetMessage(delivery_flavours[ad.flavour].whysomuchtext)
	
	elseif option == 2 then
		form:SetMessage(t("It must be delivered by ")..Format.Date(ad.due))

	elseif option == 4 then
		if ad.risk <= 0.1 then
			form:SetMessage(t("I highly doubt it."))
		elseif ad.risk > 0.1 and ad.risk <= 0.3 then
			form:SetMessage(t("Not any more than usual."))
		elseif ad.risk > 0.3 and ad.risk <= 0.6 then
			form:SetMessage(t("This is a valuable package, you should keep your eyes open."))
		elseif ad.risk > 0.6 and ad.risk <= 0.8 then
			form:SetMessage(t("It could be dangerous, you should make sure you're adequately prepared."))
		elseif ad.risk > 0.8 and ad.risk <= 1 then
			form:SetMessage(t("This is very risky, you will almost certainly run into resistance."))
		end
	
	elseif option == 3 then
		form:RemoveAdvertOnClose()

		ads[ref] = nil

		local mission = {
			type	 = t("Delivery"),
			client	 = ad.client,
			location = ad.location,
			risk	 = ad.risk,
			reward	 = ad.reward,
			due	 = ad.due,
			flavour	 = ad.flavour
		}

		local mref = Game.player:AddMission(mission)
		missions[mref] = mission

		form:SetMessage(t("Excellent. I will let the recipient know you are on your way."))
		form:AddOption(t('HANG_UP'), -1)

		return
	end

	form:AddOption(t("Why so much money?"), 1)
	form:AddOption(t("How soon must it be delivered?"), 2)
	form:AddOption(t("Will I be in any danger?"), 4)
	form:AddOption(t("Could you repeat the original request?"), 0)
	form:AddOption(t("Ok, agreed."), 3)
	form:AddOption(t('HANG_UP'), -1)
end

local onDelete = function (ref)
	ads[ref] = nil
end

local nearbysystems
local makeAdvert = function (station)
	local reward, due, location, nearbysystem
	local delivery_flavours = Translate:GetFlavours('DeliverPackage')
	local isfemale = Engine.rand:Integer(1) == 1
	local client = NameGen.FullName(isfemale)
	local flavour = Engine.rand:Integer(1,#delivery_flavours)
	local urgency = delivery_flavours[flavour].urgency
	local risk = delivery_flavours[flavour].risk

	if delivery_flavours[flavour].localdelivery == 1 then
		nearbysystem = Game.system
		local nearbystations = Game.system:GetStationPaths()
		location = nearbystations[Engine.rand:Integer(1,#nearbystations)]
		if location ==  station.path then return end
		local locdist = Space.GetBody(location.bodyIndex)
		local dist = station:DistanceTo(locdist)
		if dist < 1000 then return end
		reward = 25 + (math.sqrt(dist) / 15000) * (1+urgency)
		due = Game.time + ((4*24*60*60) * (Engine.rand:Number(1.5,3.5) - urgency))
	else
		if nearbysystems == nil then
			nearbysystems = Game.system:GetNearbySystems(max_delivery_dist, function (s) return #s:GetStationPaths() > 0 end)
		end
		if #nearbysystems == 0 then return end
		nearbysystem = nearbysystems[Engine.rand:Integer(1,#nearbysystems)]
		local dist = nearbysystem:DistanceTo(Game.system)
		local nearbystations = nearbysystem:GetStationPaths()
		location = nearbystations[Engine.rand:Integer(1,#nearbystations)]
		reward = ((dist / max_delivery_dist) * typical_reward * (1+risk) * (1.5+urgency) * Engine.rand:Number(0.8,1.2))
		due = Game.time + ((dist / max_delivery_dist) * typical_travel_time * (1.5-urgency) * Engine.rand:Number(0.9,1.1))
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

	ad.desc = string.interp(delivery_flavours[flavour].adtext, {
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
	local delivery_flavours = Translate:GetFlavours('DeliverPackage')
	for ref,ad in pairs(ads) do
		if delivery_flavours[ad.flavour].localdelivery == 0
			and ad.due < Game.time + 5*60*60*24 then -- five day timeout for inter-system
			ad.station:RemoveAdvert(ref)
		elseif delivery_flavours[ad.flavour].localdelivery == 1
			and ad.due < Game.time + 2*60*60*24 then -- two day timeout for locals
			ad.station:RemoveAdvert(ref)
		end
	end
	if Engine.rand:Integer(12*60*60) < 60*60 then -- roughly once every twelve hours
		makeAdvert(station)
	end
end

local onEnterSystem = function (player)
	local delivery_flavours = Translate:GetFlavours('DeliverPackage')
	if (not player:IsPlayer()) then return end

	local syspath = Game.system.path

	for ref,mission in pairs(missions) do
		if not mission.status and mission.location:IsSameSystem(syspath) then
			local risk = delivery_flavours[mission.flavour].risk
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

					ship = Space.SpawnShipNear(shipname, Game.player, 50, 100)
					ship:AddEquip(default_drive)
					ship:AddEquip(laser)
					ship:AIKill(Game.player)
				end
			end

			if ship then
				local pirate_greeting = string.interp(t('PIRATE_TAUNTS')[Engine.rand:Integer(1,#(t('PIRATE_TAUNTS')))], {
					client = mission.client, location = mission.location,})
				UI.ImportantMessage(pirate_greeting, ship.label)
			end
		end

		if not mission.status and Game.time > mission.due then
			mission.status = 'FAILED'
			player:UpdateMission(ref, mission)
		end
	end
end

local onLeaveSystem = function (ship)
	if ship:IsPlayer() then
		nearbysystems = nil
	end
end

local onShipDocked = function (player, station)
	local delivery_flavours = Translate:GetFlavours('DeliverPackage')
	if not player:IsPlayer() then return end

	for ref,mission in pairs(missions) do

		if mission.location == station.path then

			if Game.time > mission.due then
				UI.ImportantMessage(delivery_flavours[mission.flavour].failuremsg, mission.client)
			else
				UI.ImportantMessage(delivery_flavours[mission.flavour].successmsg, mission.client)
				player:AddMoney(mission.reward)
			end

			player:RemoveMission(ref)
			missions[ref] = nil

		elseif not mission.status and Game.time > mission.due then
			mission.status = 'FAILED'
			player:UpdateMission(ref, mission)
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
EventQueue.onEnterSystem:Connect(onEnterSystem)
EventQueue.onLeaveSystem:Connect(onLeaveSystem)
EventQueue.onShipDocked:Connect(onShipDocked)
EventQueue.onGameStart:Connect(onGameStart)

Serializer:Register("DeliverPackage", serialize, unserialize)
