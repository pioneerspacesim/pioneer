-- don't produce missions for further than this many light years away
local max_delivery_dist = 30
-- typical time for travel to a system max_delivery_dist away
local typical_travel_time = 0.9 * max_delivery_dist * 24 * 60 * 60 * 2
-- typical reward for delivery to a system max_delivery_dist away
local typical_reward = 25 * max_delivery_dist
--local MissionState = 0


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
		adtext = "Searching Pilot for reconnaissance in the {system} system",
		introtext = "Hello, my name is {name}. We have had some blips near {systembody} in the {system} ({sectorx}, {sectory}, {sectorz}) system. I'm empowered to pay you {cash} for a sensor sweep of the area.",
		whysomuchtext = "It's our standard fee for such services.",
		successmsg = "Thank you for transmitting the information. The agreed fee has been transfered to your account",
		failuremsg = "Because of your breach of contract, I had to dispatch another vessel. Your unreliability will be noted!",
		urgency = 0,
		risk = 0,
		localdelivery = 0,

	}, {
		adtext = "Scout needed in the {system} system.",
		introtext = "Hello. I'm {name}. I'm willing to pay {cash} for a sensor sweep of the area near {systembody} in the {system} ({sectorx}, {sectory}, {sectorz}) system.",
		whysomuchtext = "If you want I can offer you less...?",
		successmsg = "I've received your data, you should be getting the money any instant now.",
		failuremsg = "I might as well have sent a snail there. I'm not paying for outdated information!",
		urgency = 0.1,
		risk = 0,
		localdelivery = 0,
	}, {
		adtext = "URGENT. Data about the {system} system needed!",
		introtext = "My name is {name}, I'm a journalist currently writing a story about the {system} ({sectorx}, {sectory}, {sectorz}) system. There's some hints I need to verify in the vicinity of {systembody}. I'm willing to pay {cash} to anyone who can get me data about the area before my deadline runs up.",
		whysomuchtext = "I'm a renowned journalist, not one of those wannabes writing for handout newspapers. I know that accurate information comes at a price.",
		successmsg = "Thanks a lot, that's just the source material I needed for my article. Your money is on the way!",
		failuremsg = "I could not finish the article on time because I did not have the data to back up some points. No money for me, no money for you either.",
		urgency = 0.6,
		risk = 0,
		localdelivery = 0,
	}, {
		adtext = "RECON. in the {system} system. {cash} to an experienced pilot.",
		introtext = "Hello. I'm {name}, information is my business. I'm willing to pay {cash} for a sensor sweep of {systembody} in the {system} ({sectorx}, {sectory}, {sectorz}) system.",
		whysomuchtext = "I know there's something going on there, but I don't know what. In my profession, that's not good.",
		successmsg = "Your timely and discrete service is much appreciated. You have been paid in full.",
		failuremsg = "Useless! I will never depend on you again! Needless to say, you will not be paid for this.",
		urgency = 0.4,
		risk = 0.75,
		localdelivery = 0,
	}, {
		adtext = "Help us keep the {system} system orderly!",
		introtext = "Greetings. This is Lieutenant {name} from AdAstra security services. We pay {cash} for data about {systembody} in the {system} ({sectorx}, {sectory}, {sectorz}) system.",
		whysomuchtext = "We need to check out some rumors. Usually they are unsubstantial, but sometimes they turn out to be true.",
		successmsg = "Well done, your money is being transfered.",
		failuremsg = "Your ship registration has been noted, we will reject all further applications for work from you.",
		urgency = 0.1,
		risk = 0.1,
		localdelivery = 0,
	},	{
		adtext = "{system} administration offices need your help to keep their files up to date!",
		introtext = "Pleased to meet you, I am secretary {name}, {system} administration, and I'm willing to pay {cash} for current data about {systembody}. No rush, we simply need to keep our files up to date.",
		whysomuchtext = "This is a government job. It's not OUR money.",
		successmsg = "Thank you for helping us keeping our information current. Your pay is transfered as we speak.",
		failuremsg = "Am I supposed to update our outdated information with other outdated information? This is unacceptable I'm afraid.",
		urgency = 0.1,
		risk = 0,
		localdelivery = 1,
	},
--	{
	--[[	adtext = "SHORT-RANGE COURIER. Delivery of a small package to {starport}.",
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
	},]]--
	{
		adtext = "{system} police needs your help to keep order!",
		introtext = "I am captain {name}, {system} police. We urgently need information about {systembody}. I'll pay you {cash} credits if I get the information in a reasonable time.",
		whysomuchtext = "We can't allow such dangers to our citizens in our neighbourhood. We need to know what's going on there, and we need to know it soon!",
		successmsg = "Your prompt report is appreciated. Your money has been transfered.",
		failuremsg = "I needed that information some time ago! I refuse to pay you.",
		urgency = 0.6,
		risk = 0.4,
		localdelivery = 1,
	},
--[[	{
		adtext = "PACKAGE DROP. Urgent dispatch of perishables to {starport}.",
		introtext = "Greetings, we're behind with our produce shipment and need it delivered to {starport} urgently. We'll pay you {cash} for your troubles.",
		whysomuchtext = "Our livelyhood depends on it.",
		successmsg = "Grand! We'll start unpacking immediately. I'll have your account updated right away.",
		failuremsg = "It's all spoilt, this is of no use to anyone! We cannot and will not pay you.",
		urgency = 0.8,
		risk = 0,
		localdelivery = 1,
	},]]--
}

local delivery_dangers = {
	simple = "This is just a routine check. If there was a substantial risk, I think we would have heard of attacks in the area.",
	easy = "I suspect that there is some unregistered activity going on. Nothing big probably, but you'd better be prepared.",
	medium = "a ship has vanished in the area. I suspect pirate activity.",
	hard = "several ships have been lost in the area, including my last scout. I really need to know what's going on.",
	deadly = "I have reports from passing ships that confirm pirate attacks. What I need to know is how strong they are. You are certain to meet hostiles.",
}

local pirate_taunts = {
	"Looky here, it's payday!",
	"All your ship are belong to us!",
	"You won't get back with that sensor data!",
	--[["I'm sure this will bring a pretty penny on the market",
	"Today isn't your lucky day! Prepare to die.",
	"Tell my old friend {client} that I'll see them in hell!",
	"That package isn't going to reach it's destination today.",
	"You're not getting to {location} today!",
	"You'll pay for that cargo, with your life.",]]--
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
			systembody = sbody.name,
			system   = sys.name,
			sectorx  = ad.location.sectorX,
			sectory  = ad.location.sectorY,
			sectorz  = ad.location.sectorZ,
		})

		form:SetMessage(introtext)

	elseif option == 1 then
		form:SetMessage(delivery_flavours[ad.flavour].whysomuchtext)

	elseif option == 2 then
		form:SetMessage("I need the information by "..Format.Date(ad.due))

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
			type	 = "Recon",
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

		form:SetMessage("Excellent. I will await your report.")
		form:AddOption("Hang up.", -1)

		return
	end

	form:AddOption("Why so much money?", 1)
	form:AddOption("When do you need the data?", 2)
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
		local nearbysystems = Game.system:GetNearbySystems(max_delivery_dist, function (s) return #s:GetStationPaths() > 0 end)
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
										UI.ImportantMessage("Distance reached, starting long range sensor sweep. Maintain orbit for at least 60 minutes", "computer")
										mission.state = 1
									end
									if Dist > PhysBody.radius * 1.4 and mission.state == 1 then
										UI.ImportantMessage("sensor sweep interrupted, too far from target!", "computer")
										mission.state = 0
										TimeUp = 0
									end
									if mission.state == 1 then
										TimeUp = TimeUp + 10
										if not ShipSpawned then
											ShouldSpawn = Engine.rand:Number(MinChance, 1)

											-- -----------------------------------------------------------
											if 	ShouldSpawn > 0.9 then
												ShipSpawned = true
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

														ship = Space.SpawnShipNear(shipname,Game.player, 10, 15)
														ship:AddEquip(default_drive)
														ship:AddEquip(laser)
														ship:AIKill(Game.player)
													end
												end

												if ship then
													local pirate_greeting = string.interp(pirate_taunts[Engine.rand:Integer(1,#pirate_taunts)], {
													client = mission.client, location = mission.location:GetSystemBody().name,})
													UI.ImportantMessage(pirate_greeting, ship.label)
												end
											end

											-- -----------------------------------------------------------
											if not ShipSpawned then
												MinChance = MinChance + 0.1
											end
										end

										if TimeUp > 3600 then
											mission.state = 2
											UI.ImportantMessage("Sensor sweep complete, data stored.", "computer")
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
			UI.ImportantMessage(delivery_flavours[mission.flavour].successmsg, mission.client)
			player:AddMoney(mission.reward)
			player:RemoveMission(ref)
			missions[ref] = nil
		elseif mission.state == 3 then
			UI.ImportantMessage(delivery_flavours[mission.flavour].failuremsg, mission.client)
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
