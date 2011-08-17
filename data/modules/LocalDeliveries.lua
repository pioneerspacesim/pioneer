local localdelivery_flavours = {
	{
		adtext = "SHORT-RANGE COURIER. Delivery of a small package to {starport}.",
		introtext = "Hi. I'm {name} and I will pay {cash} for a ship to deliver this package to {starport}.",
		whysomuchtext = "I don't think it's a lot.",
		dangertext = "I doubt it, they are just some left-behind curios.",
		successmsg = "Thank you for the package, you have been paid in full.",
		failuremsg = "I could have delivered it faster myself. I'm not paying you.",
		urgency = 0.2,
	},
	{
		adtext = "INTER-PLANETARY CARGO. Freight of local cargo to {starport}.",
		introtext = "Hello. We need these crates delivered to {starport} as soon as possible. Standard rate for this is shipment is {cash}.",
		whysomuchtext = "Standard rates, we work with the market.",
		dangertext = "Not any more so than usual.",
		successmsg = "Excellent, we've credited the funds into your account.",
		failuremsg = "Our customers are not going to be happy with this. Do not expect to be paid.",
		urgency = 0.4,
	},
	{
		adtext = "NEARBY DELIVERY. Require quick delivery of an item to {starport}.",
		introtext = "My name is {name} and I need this item delivered to a friend at {starport} pronto, I'll pay you {cash} credits if you get it there in a reasonable time.",
		whysomuchtext = "It's really urgent.",
		dangertext = "It's not of value to anyone else, so I would say no.",
		successmsg = "Your prompt delivery is appreciated, I have credited your account accordingly.",
		failuremsg = "You were offered a premium for quick delivery! I refuse to pay for this.",
		urgency = 0.6,
	},
	{
		adtext = "PACKAGE DROP. Urgent dispatch of perishables to {starport}.",
		introtext = "Greetings, we're behind with our produce shipment and need it delivered to {starport} urgently. We'll pay you {cash} for your troubles.",
		whysomuchtext = "Our livelyhood depends on it.",
		dangertext = "You shouldn't see any trouble.",
		successmsg = "Grand! We'll start unpacking immediately. I'll have your account updated right away.",
		failuremsg = "It's all spoilt, this is of no use to anyone! We cannot and will not pay you.",
		urgency = 0.8,
	},
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
		form:SetFace({female = ad.isfemale, seed = ad.faceseed, name = ad.client})

		local sbody = ad.location:GetSystemBody()

		local introtext = string.interp(localdelivery_flavours[ad.flavour].introtext, {
			name     = ad.client,
			cash     = Format.Money(ad.reward),
			starport = sbody.name,
		})

		form:SetMessage(introtext)

	elseif option == 1 then
		form:SetMessage(localdelivery_flavours[ad.flavour].whysomuchtext)
	
	elseif option == 2 then
		form:SetMessage("It has to be delivered by "..Format.Date(ad.due))

	elseif option == 4 then
		form:SetMessage(localdelivery_flavours[ad.flavour].dangertext)

	elseif option == 3 then
		form:RemoveAdvertOnClose()

		ads[ref] = nil

		local mission = {
			type     = "Local Delivery",
			client   = ad.client,
			location = ad.location,
			reward   = ad.reward,
			due      = ad.due,
			flavour  = ad.flavour
		}

		local mref = Game.player:AddMission(mission)
		missions[mref] = mission

		form:SetMessage("It's a deal.")
		form:AddOption("Hang up.", -1)

		return
	end

	form:AddOption("Could you repeat the original request?", 0)
	form:AddOption("Why so much money?", 1)
	form:AddOption("Will I be in any danger?", 4)
	form:AddOption("How soon must it be delivered?", 2)
	form:AddOption("Ok, agreed.", 3)
	form:AddOption("Hang up.", -1)
end

local onDelete = function (ref)
	ads[ref] = nil
end

local makeAdvert = function (station)
	local nearbystations = Game.system:GetStationPaths()
	local location = nearbystations[Engine.rand:Integer(1,#nearbystations)]
	local locdist = Space.GetBody(location.bodyIndex)
	local dist = Game.player:DistanceTo(locdist)
	if dist < 1000 then return end -- Bail if it selects the station we're docked at

	local isfemale = Engine.rand:Integer(1) == 1
	local client = NameGen.FullName(isfemale)

	local flavour = Engine.rand:Integer(1,#localdelivery_flavours)
	local urgency = localdelivery_flavours[flavour].urgency
	local basett = 3*24*60*60 -- base travel time of 3 days
	local due = Game.time + (basett * (Engine.rand:Number(1.2,2.8) - urgency))
	local reward = 15 + (math.sqrt(dist) / 10000) * (1+urgency) -- Base of $15 with distance and urgency added value

	local ad = {
		station  = station,
		flavour  = flavour,
		client   = client,
		location = location,
		isfemale = isfemale,
		due      = due,
		reward   = reward,
		faceseed = Engine.rand:Integer(),
	}

	local sbody = ad.location:GetSystemBody()

	ad.desc = string.interp(localdelivery_flavours[flavour].adtext, {
		cash   = Format.Money(ad.reward),
		starport = sbody.name,
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
		if (ad.due < Game.time + 2*60*60*24) then
			ads[ref] = nil
			station:RemoveAdvert(ref)
		end	
	end
	if Engine.rand:Integer(14*60*60) < 60*60 then
		makeAdvert(station)
	end
end

local onShipDocked = function (player, station)
	if not player:IsPlayer() then return end

	for ref,mission in pairs(missions) do

		if mission.location == station.path then

			if Game.time > mission.due then
				UI.ImportantMessage(localdelivery_flavours[mission.flavour].failuremsg, mission.client)
			else
				UI.ImportantMessage(localdelivery_flavours[mission.flavour].successmsg, mission.client)
				player:AddMoney(mission.reward)
			end

			player:RemoveMission(ref)
			missions[ref] = nil

		elseif Game.time > mission.due then
			mission.status = 'FAILED'
			player:UpdateMission(ref, mission)

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
EventQueue.onShipDocked:Connect(onShipDocked)
EventQueue.onGameStart:Connect(onGameStart)

Serializer:Register("LocalDeliveries", serialize, unserialize)
