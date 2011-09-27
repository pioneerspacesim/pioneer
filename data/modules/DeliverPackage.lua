-- don't produce missions for further than this many light years away
local max_delivery_dist = 30
-- typical time for travel to a system max_delivery_dist away
local typical_travel_time = 0.9 * max_delivery_dist * 24 * 60 * 60 
-- typical reward for delivery to a system max_delivery_dist away
local typical_reward = 25 * max_delivery_dist

local delivery_flavours = {
	-- adtext - text shown in the bulletin board list
	-- introtext - shown when the advert is selected (and "Could you repeat request?")
	-- whysomuchtext - response to "Why so much?"
	-- successmsg - message sent on successful delivery
	-- failuremsg - message sent on failed delivery
	-- urgency - how urgent the delivery is. 0 is surface mail. 1 is overnight
	-- risk - how risky the mission is. 0 is letters from mother. 1 is certain death
	-- local - 1 if the delivery is to the local (this) system, 0 otherwise
	{
		adtext = "GOING TO the {system} system? Money paid for delivery of a small package.",
		introtext = "Hi, I'm {name}. I'll pay you {cash} if you will deliver a small package to {starport} in the {system} ({sectorx}, {sectory}, {sectorz}) system.",
		whysomuchtext = "When a friend visited me she left behind some clothes and antique paper books. I'd like to have them returned to her.",
		successmsg = "Thank you for the delivery. You have been paid in full.",
		failuremsg = "Unacceptable! You took forever over that delivery. I'm not willing to pay you.",
		urgency = 0,
		risk = 0,
		localdelivery = 0,

	}, {
		adtext = "WANTED. Delivery of a package to the {system} system.",
		introtext = "Hello. I'm {name}. I'm willing to pay {cash} for a ship to carry a package to {starport} in the {system} ({sectorx}, {sectory}, {sectorz}) system.",
		whysomuchtext = "It is nothing special.",
		successmsg = "The package has been received and you have been paid in full.",
		failuremsg = "I'm frustrated by the late delivery of my package, and I refuse to pay you.",
		urgency = 0.1,
		risk = 0,
		localdelivery = 0,
	}, {
		adtext = "URGENT. Fast ship needed to deliver a package to the {system} system.",
		introtext = "Hello. I'm {name}. I'm willing to pay {cash} for a ship to carry a package to {starport} in the {system} ({sectorx}, {sectory}, {sectorz}) system.",
		whysomuchtext = "It is a research proposal and must be delivered by the deadline or we may not get funding.",
		successmsg = "You have been paid in full for the delivery. Thank you.",
		failuremsg = "I was quite clear about the deadline and am very disappointed by the late delivery. You will not be paid.",
		urgency = 0.6,
		risk = 0,
		localdelivery = 0,
	}, {
		adtext = "DELIVERY. Documents to the {system} system. {cash} to an experienced pilot.",
		introtext = "Hello. I'm {name}. I'm willing to pay {cash} for a ship to carry a package to {starport} in the {system} ({sectorx}, {sectory}, {sectorz}) system.",
		whysomuchtext = "Some extremely sensitive documents have fallen into my hands, and I have reason to believe that the leak has been traced to me.",
		successmsg = "Your timely and discrete service is much appreciated. You have been paid in full.",
		failuremsg = "Useless! I will never depend on you again! Needless to say, you will not be paid for this.",
		urgency = 0.4,
		risk = 0.75,
		localdelivery = 0,
	}, {
		adtext = "POSTAL SERVICE. We require a ship for the delivery run to {system} system.",
		introtext = "Greetings. This is an automated message from Bedford and {name} Courier Services. We pay {cash} for the run to {starport} in the {system} ({sectorx}, {sectory}, {sectorz}) system.",
		whysomuchtext = "We would be happy to pay you less money.",
		successmsg = "Your timely and discrete service is much appreciated. You have been paid in full.",
		failuremsg = "Your ship registration has been noted, we will reject all further applications for work from you.",
		urgency = 0.1,
		risk = 0.1,
		localdelivery = 0,
	},
		{
		adtext = "MOVING HOME. Move of hardware to {starport} from storage.",
		introtext = "Nice to meet you. I am {name} and I'm willing to pay {cash} for someone with a ship to help me move my belongings to {starport}. No rush, they are just some leftovers from moving house.",
		whysomuchtext = "Is it a lot? I should rethink my offer!",
		successmsg = "Oh wonderful. I'll start unloading immediately. Thanks again.",
		failuremsg = "What are these? Oh, you took so long that I forgot I'd even sent this!",
		urgency = 0.1,
		risk = 0,
		localdelivery = 1,
	},
	{
		adtext = "SHORT-RANGE COURIER. Delivery of a small package to {starport}.",
		introtext = "Hi. I'm {name} and I will pay {cash} for a ship to deliver this package to {starport}.",
		whysomuchtext = "I don't think it's a lot.",
		successmsg = "Thank you for the package, you have been paid in full.",
		failuremsg = "I could have delivered it faster myself. I'm not paying you.",
		urgency = 0.2,
		risk = 0,
		localdelivery = 1,
	},
	{
		adtext = "INTER-PLANETARY CARGO. Freight of local cargo to {starport}.",
		introtext = "Hello. We need these crates delivered to {starport} as soon as possible. Standard payment for this shipment is {cash}.",
		whysomuchtext = "Standard rates, we work with the market.",
		successmsg = "Excellent, we've credited the funds into your account.",
		failuremsg = "Our customers are not going to be happy with this. Do not expect to be paid.",
		urgency = 0.4,
		risk = 0,
		localdelivery = 1,
	},
	{
		adtext = "NEARBY DELIVERY. Require quick delivery of an item to {starport}.",
		introtext = "My name is {name} and I need this item delivered to a friend at {starport} pronto, I'll pay you {cash} credits if you get it there in a reasonable time.",
		whysomuchtext = "It's really urgent.",
		successmsg = "Your prompt delivery is appreciated, I have credited your account accordingly.",
		failuremsg = "You were offered a premium for quick delivery! I refuse to pay for this.",
		urgency = 0.6,
		risk = 0,
		localdelivery = 1,
	},
	{
		adtext = "PACKAGE DROP. Urgent dispatch of perishables to {starport}.",
		introtext = "Greetings, we're behind with our produce shipment and need it delivered to {starport} urgently. We'll pay you {cash} for your troubles.",
		whysomuchtext = "Our livelyhood depends on it.",
		successmsg = "Grand! We'll start unpacking immediately. I'll have your account updated right away.",
		failuremsg = "It's all spoilt, this is of no use to anyone! We cannot and will not pay you.",
		urgency = 0.8,
		risk = 0,
		localdelivery = 1,
	},
}

local delivery_dangers = {
	simple = "I highly doubt it.",
	easy = "Not any more than usual.",
	medium = "This is a valuable package, you should keep your eyes open.",
	hard = "It could be dangerous, you should make sure you're adequately prepared.",
	deadly = "This is very risky, you will almost certainly run into resistance.",
}

local pirate_taunts = {
	"You're going to regret dealing with {client}",
	"Looks like my paycheck has arrived!",
	"You're working for {client}? That was a bad idea.",
	"Your cargo and your life, pilot!",
	"I'm sure this will bring a pretty penny on the market",
	"Today isn't your lucky day! Prepare to die.",
	"Tell my old friend {client} that I'll see them in hell!",
	"That package isn't going to reach it's destination today.",
	"You're not getting to {location} today!",
	"You'll pay for that cargo, with your life.",
}

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
		form:SetMessage("It must be delivered by "..Format.Date(ad.due))

	elseif option == 4 then
		if ad.risk <= 0.1 then
			form:SetMessage(delivery_dangers.simple)
		elseif ad.risk > 0.1 and ad.risk <= 0.3 then
			form:SetMessage(delivery_dangers.easy)
		elseif ad.risk > 0.3 and ad.risk <= 0.6 then
			form:SetMessage(delivery_dangers.medium)
		elseif ad.risk > 0.6 and ad.risk <= 0.8 then
			form:SetMessage(delivery_dangers.hard)
		elseif ad.risk > 0.8 and ad.risk <= 1 then
			form:SetMessage(delivery_dangers.deadly)
		end
	
	elseif option == 3 then
		form:RemoveAdvertOnClose()

		ads[ref] = nil

		local mission = {
			type	 = "Delivery",
			client	 = ad.client,
			location = ad.location,
			risk	 = ad.risk,
			reward	 = ad.reward,
			due	 = ad.due,
			flavour	 = ad.flavour
		}

		local mref = Game.player:AddMission(mission)
		missions[mref] = mission

		form:SetMessage("Excellent. I will let the recipient know you are on your way.")
		form:AddOption("Hang up.", -1)

		return
	end

	form:AddOption("Why so much money?", 1)
	form:AddOption("How soon must it be delivered?", 2)
	form:AddOption("Will I be in any danger?", 4)
	form:AddOption("Could you repeat the original request?", 0)
	form:AddOption("Ok, agreed.", 3)
	form:AddOption("Hang up.", -1)
end

local onDelete = function (ref)
	ads[ref] = nil
end

local makeAdvert = function (station)
	local reward, due, location, nearbysystem
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
		local nearbysystems = Game.system:GetNearbySystems(max_delivery_dist, function (s) return #s:GetStationPaths() > 0 end)
		if #nearbysystems == 0 then return end
		nearbysystem = nearbysystems[Engine.rand:Integer(1,#nearbysystems)]
		local dist = nearbysystem:DistanceTo(Game.system)
		local nearbystations = nearbysystem:GetStationPaths()
		location = nearbystations[Engine.rand:Integer(1,#nearbystations)]
		reward = ((dist / max_delivery_dist) * typical_reward * (1+risk) * (1.5-urgency) * Engine.rand:Number(0.8,1.2))
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
	if (not player:IsPlayer()) then return end

	local syspath = Game.system.path

	for ref,mission in pairs(missions) do
		if not mission.status and mission.location:IsSameSystem(syspath) then
			local risk = delivery_flavours[mission.flavour].risk
			local ships = 0

			local riskmargin = Engine.rand:Number(-0.3,0.3) -- Add some random luck
			if risk >= (0.5 + riskmargin) then ships = 1
			elseif risk >= (0.7 + riskmargin) then ships = 2
			elseif risk >= (1.0 + riskmargin) then ships = 3
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
				local pirate_greeting = string.interp(pirate_taunts[Engine.rand:Integer(1,#pirate_taunts)], {
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

local onShipDocked = function (player, station)
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
EventQueue.onShipDocked:Connect(onShipDocked)
EventQueue.onGameStart:Connect(onGameStart)

Serializer:Register("DeliverPackage", serialize, unserialize)
