-- don't produce missions for further than this many light years away
local max_delivery_dist = 20

-- typical time for travel to a system max_delivery_dist away
local typical_travel_time = 0.9*max_delivery_dist*24*60*60 

-- typical reward for delivery to a system max_delivery_dist away
local typical_reward = 25*max_delivery_dist

local delivery_flavours = {
	{
		-- text shown in the bulletin board list
		adtext = "GOING TO the {system} system? Money paid for delivery of a small package.",

		-- introductory text when the advert is selected (and "Could you repeat request?")
		introtext = "Hi, I'm {name}. I'll pay you {cash} if you will deliver a small package to {starport} in the {system} ({sectorx}, {sectory}) system.",

		-- response to "Why so much?"
		whysomuchtext = "When a friend visited me she left behind some clothes and antique paper books. I'd like to have them returned to her.",

		-- message sent on successful delivery
		successmsg = "Thank you for the delivery. You have been paid in full.",

		-- message sent on failed delivery
		failuremsg = "Jesus wept, you took forever over that delivery. I'm not willing to pay you.",

		-- how urgent the delivery is. 0 is surface mail. 1 is overnight
		urgency = 0,

		-- how risky the mission is. 0 is letters from mother. 1 is certain death
		risk = 0,

	}, {
		adtext = "WANTED. Delivery of a package to the {system} system.",
		introtext = "Hello. I'm {name}. I'm willing to pay {cash} for a ship to carry a package to {starport} in the {system} ({sectorx}, {sectory}) system.",
		whysomuchtext = "It is nothing special.",
		successmsg = "The package has been received and you have been paid in full.",
		failuremsg = "I'm frustrated by the late delivery of my package, and I refuse to pay you.",
		urgency = 0.1,
		risk = 0,
	}, {
		adtext = "URGENT. Fast ship needed to deliver a package to the {system} system.",
		introtext = "Hello. I'm {name}. I'm willing to pay {cash} for a ship to carry a package to {starport} in the {system} ({sectorx}, {sectory}) system.",
		whysomuchtext = "It is a research proposal and must be delivered by the deadline or we may not get funding.",
		successmsg = "You have been paid in full for the delivery. Thank you.",
		failuremsg = "I was quite clear about the deadline and am very disappointed by the late delivery. You will not be paid.",
		urgency = 0.6,
		risk = 0,
	}, {
		adtext = "DELIVERY. Documents to the {system} system. {cash} to an experienced pilot.",
		introtext = "Hello. I'm {name}. I'm willing to pay {cash} for a ship to carry a package to {starport} in the {system} ({sectorx}, {sectory}) system.",
		whysomuchtext = "Some extremely sensitive documents have fallen into my hands, and I have reason to believe that the leak has been traced to me.",
		successmsg = "Your timely and discrete service is much appreciated. You have been paid in full.",
		failuremsg = "Useless! I will never depend on you again! Needless to say, you will not be paid for this.",
		urgency = 0.4,
		risk = 0.75,
	}, {
		adtext = "POSTAL SERVICE. We require a ship for the delivery run to {system} system.",
		introtext = "Greetings. This is an automated message from Bedford and {name} Courier Services. We pay {cash} for the run to {starport} in the {system} ({sectorx}, {sectory}) system.",
		whysomuchtext = "We would be happy to pay you less money.",
		successmsg = "Your timely and discrete service is much appreciated. You have been paid in full.",
		failuremsg = "Your ship registration has been noted, we will reject all further applications for work from you.",
		urgency = 0.1,
		risk = 0.1,
	},
}

local ads = {}
local missions = {}

local onChat = function (dialog, ref, option)
	local ad = ads[ref]

	dialog:Clear()

	if option == -1 then
		dialog:Close()
		return
	end

	if option == 0 then
		local sys   = ad.location:GetStarSystem()
		local sbody = ad.location:GetSystemBody()

		local introtext = string.interp(delivery_flavours[ad.flavour].introtext, {
			name     = ad.client,
			cash     = Format.Money(ad.reward);
			starport = sbody.name,
			system   = sys.name,
			sectorx  = ad.location.sectorX,
			sectory  = ad.location.sectorY,
		})

		dialog:SetMessage(introtext)

	elseif option == 1 then
		dialog:SetMessage(delivery_flavours[ad.flavour].whysomuchtext)
	
	elseif option == 2 then
		dialog:SetMessage("It must be delivered by "..Format.Date(ad.due))
	
	elseif option == 3 then
		dialog:RemoveAdvertOnClose()

		ads[ref] = nil

		local mission = {
			type     = "Delivery",
			client   = ad.client,
			location = ad.location,
			reward   = ad.reward,
			due      = ad.due,
			flavour  = ad.flavour
		}

		local mref = Game.player:AddMission(mission)
		missions[mref] = mission

		dialog:SetMessage("Excellent.")
		dialog:AddOption("Hang up.", -1)

		return
	end

	dialog:AddOption("Why so much money?", 1);
	dialog:AddOption("How soon must it be delivered?", 2);
	dialog:AddOption("Could you repeat the original request?", 0);
	dialog:AddOption("Ok, agreed.", 3);
	dialog:AddOption("Hang up.", -1);
end

local onDelete = function (ref)
	ads[ref] = nil
end

local makeAdvert = function (station)
	local nearbysystems = Game.system:GetNearbySystems(max_delivery_dist, function (s) return #s:GetStationPaths() > 0 end)
	if #nearbysystems == 0 then return end

	local nearbysystem = nearbysystems[Engine.rand:Integer(1,#nearbysystems)]
	local dist = nearbysystem:DistanceTo(Game.system)

	local nearbystations = nearbysystem:GetStationPaths()
	local location = nearbystations[Engine.rand:Integer(1,#nearbystations)]

	local isfemale = Engine.rand:Integer() == 1
	local client = NameGen.FullName(isfemale)

	local flavour = Engine.rand:Integer(1,#delivery_flavours)
	local urgency = delivery_flavours[flavour].urgency
	local risk = delivery_flavours[flavour].risk

	local dist_norm = dist / max_delivery_dist
	local due = Game.time + ((dist / max_delivery_dist) * typical_travel_time * (1.5-urgency) * Engine.rand:Number(0.9,1.1))
	local reward = ((dist / max_delivery_dist) * typical_reward * (1+risk) * (1.5-urgency) * Engine.rand:Number(0.8,1.2))

	local ad = {
		station  = station,
		flavour  = flavour,
		client   = client,
		location = location,
		due      = due,
		reward   = reward,
	}

	ad.desc = string.interp(delivery_flavours[flavour].adtext, {
		system = nearbysystem.name,
		cash   = Format.Money(ad.reward),
	})

	local ref = station:AddAdvert(ad.desc, onChat, onDelete)
	ads[ref] = ad
end

local onCreateBB = function (station)
	local num = Engine.rand:Integer(1, math.ceil(Game.system.population))
	for i = 1,num do
		makeAdvert(station)
	end
end

local onUpdateBB = function (station)
	for ref,ad in pairs(ads) do
		if (ad.due < Game.time + 5*60*60*24) then -- remove with five days left
			ads[ref] = nil
			station:RemoveAdvert(ref)
		end	
	end
	if Engine.rand:Integer(0,12*60*60) < 60*60 then -- roughly once every twelve hours
		makeAdvert(station)
	end
end

local onEnterSystem = function (player)
	if (not player:IsPlayer()) then return end

	local syspath = Game.system.path

	for ref,mission in pairs(missions) do
		if not mission.status and mission.location:IsSameSystem(syspath) then
			local risk = delivery_flavours[mission.flavour].risk

			local ships = 1
			if risk >= 0.8 then ships = 2 end
			if risk == 1.0 then ships = 3 end

			local shiptypes = ShipType.GetShipTypes('SHIP', function (t)
				local mass = t.hullMass
				return mass >= 100 and mass <= 300
			end)
			if #shiptypes == 0 then return end

			local ship

			while ships > 0 do
				ships = ships-1

				if Engine.rand:Number() <= risk then
					local shipname = shiptypes[Engine.rand:Integer(1,#shiptypes)]
					local shiptype = ShipType.GetShipType(shipname)
					local default_drive = shiptype.defaultHyperdrive

					local max_laser_size = shiptype.capacity - EquipType.GetEquipType(default_drive).mass
					local lasers = EquipType.GetEquipTypes('LASER', function (e,et)
						return et.mass <= max_laser_size and string.sub(e,0,11) == 'PULSECANNON'
					end)
					local laser = lasers[Engine.rand:Integer(1,#lasers)]

					ship = Space.SpawnShipNear(shipname, Game.player, 50, 200)
					ship:AddEquip(default_drive)
					ship:AddEquip(laser)
					ship:AIKill(Game.player)
				end
			end

			if ship then
				UI.ImportantMessage(ship.label, "You're going to regret dealing with "..mission.client)
			end
		end
	end
end

local onShipDocked = function (player, station)
	if not player:IsPlayer() then return end

	for ref,mission in pairs(missions) do

		if mission.location == station.path then

			if Game.time > mission.due then
				UI.ImportantMessage(mission.client, delivery_flavours[mission.flavour].failuremsg)
			else
				UI.ImportantMessage(mission.client, delivery_flavours[mission.flavour].successmsg)
				player:AddMoney(mission.reward)
			end

			player:RemoveMission(ref)
			missions[ref] = nil

		elseif Game.time > mission.due then
			mission.status = 'failed'
			player:UpdateMission(ref, mission)
		end

	end
end

local serialize = function ()
	return { missions = missions, ads = ads }
end

local unserialize = function (data)
	for k,mission in pairs(data.missions) do
		local mref = Game.player:AddMission(mission)
		missions[mref] = mission
	end
	for k,ad in pairs(data.ads) do
		local ref = ad.station:AddAdvert(ad.desc, onChat, onDelete)
		ads[ref] = ad
	end
end

EventQueue.onCreateBB:Connect(onCreateBB)
EventQueue.onUpdateBB:Connect(onUpdateBB)
EventQueue.onEnterSystem:Connect(onEnterSystem)
EventQueue.onShipDocked:Connect(onShipDocked)

Serializer:Register("DeliverPackage", serialize, unserialize)
