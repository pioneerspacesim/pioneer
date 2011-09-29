local service_flavours = {
	-- title: Name of company, can contain a {name} for the station's name,
	--        or a {proprietor} for the company's owner
	-- intro: The initial blurb on the ad.  In additoin to {name} and
	--        {proprietor}, {drive} is the player's current hyperdrive,
	--        {price} is the price of a 12 month service and {lasttime} inserts
	--        the time and company name of the last service.
	-- yesplease: The prompt for the player to click on to get a service.
	-- response: What the company says when the player clicks yesplease.
	{
		title = "{name} Engine Servicing Company",
		intro = [[Avoid the inconvenience of a broken-down hyperspace engine.  Get yours serviced today, by the officially endorsed {name} Engine Servicing Company.

Engine: {drive}
Service: {price}
Guarantee: 1 year
{lasttime}]],
		yesplease = "Service hyperspace engine",
		response = "Your engine has been serviced.",
	},{
		title = "{proprietor}: Hyperdrive maintenance specialist",
		intro = [[I'm {proprietor}.  I can service your {drive}, guaranteeing at least a year of trouble-free performance.  The cost for this service will be {price}
{lasttime}]],
		yesplease = "Service my drive",
		response = "I have serviced your hyperdrive.",
	},{
		title = "{proprietor} & Co HyperMechanics",
		intro = [[Hi there.  We at {proprietor} & Co stake our reputation on our work.

{lasttime}
We can tune your ship's {drive}, ensuring 12 months of trouble-free operation, for the sum of {price}.  I'll be supervising the work myself, so you can be sure that a good job will be done.]],
		yesplease = "Please tune my drive at the quoted price",
		response = "Service complete.  Thanks for your custom.",
	},{
		title = "SuperFix Maintenance ({name} branch)",
		intro = [[Welcome SuperFix Maintenance.

{lasttime}
Time for your annual maintenance? Let us SuperFix your hyperdrive!
We can tune your {drive} for just {price}.  There's nobody cheaper on this station!]],
		yesplease = "SuperFix me!",
		response = "Your SuperFix service is complete, with SuperFix guarantee!",
	},{
		title = "Time and Space Engines, Inc.",
		intro = [[Welcome to Time and Space.
		
We specialise in interstellar engines. All maintenance work guaranteed for a year.
{lasttime}
Servicing your {drive} will cost {price}.  Would you like to proceed?]],
		yesplease = "Yes, please proceed",
		response = "We have completed the work on your hyperdrive.",
	}
}

-- Other translatable strings used --
-------------------------------------
-- Hang up
local hangup = "Hang up"
-- I don't have enough money
local notenoughmoney = "I don't have enough money"
-- Arbitrary string; it was in case I *needed* a name for the last service.
-- It's little more than a flag at the moment.
local default_service_name = "Manufacturer's warranty"
-- How to describe the last service, if (and only if) there was one.
local last_service_message = "Your drive was last serviced on {date} by {company}"
-- How to describe the last service after a game start or ship purchase.
local first_service_message = "Your drive has not been serviced since it was installed on {date}"
-- How to describe the last service when there's no drive
local no_drive_to_service = "You do not have a drive to service!"
-- Hyperdrive broke down
local broken_hyperdrive = "The ship's hyperdrive has been destroyed by a malfunction"

-- Default numeric values --
----------------------------
local service_period = 31557600 -- One standard Julian year
-- 10, guaranteed random by D16 dice roll.
-- This is to make the BBS name different from the station welcome character.
local seedbump = 10
-- How many jumps might you get after your service_period is finished?
-- Failure is increasingly likely with each jump, this being the limit
-- where probability = 1
local max_jumps_unserviced = 255

local ads = {}
local service_history = {
	lastdate = 0, -- Default will be overwritten on game start
	company = default_service_name,
	jumpcount = 0, -- Number of jumps made after the service_period
}

local lastServiceMessage = function (hyperdrive)
	-- Fill in the blanks tokens on the {lasttime} string from service_history
	local message
	if hyperdrive == 'NONE' then
		message = no_drive_to_service
	elseif service_history.company == default_service_name then
		message = first_service_message
	else
		message = last_service_message
	end
	return string.interp(message, {date = Format.Date(service_history.lastdate), company = service_history.company})
end

local onChat = function (form, ref, option)
	local ad = ads[ref]

	local hyperdrive = Game.player:GetEquip('ENGINE',0)

	-- Tariff!  ad.baseprice is from 2 to 10
	local price = ad.baseprice
	price = price * (({
		NONE = 0,
		DRIVE_CLASS1 = 1.0,
		DRIVE_CLASS2 = 1.2,
		DRIVE_CLASS3 = 1.4,
		DRIVE_CLASS4 = 1.8,
		DRIVE_CLASS5 = 2.6,
		DRIVE_CLASS6 = 3.8,
		DRIVE_CLASS7 = 5.4,
		DRIVE_CLASS8 = 7.0,
		DRIVE_CLASS9 = 9.6,
		DRIVE_MIL1 = 1.2,
		DRIVE_MIL2 = 1.6,
		DRIVE_MIL3 = 2.8,
		DRIVE_MIL4 = 4.0,
	})[hyperdrive] or 10)

	-- Now make it bigger (-:
	price = price * 10

	-- Replace those tokens into ad's intro text that can change during play
	message = string.interp(ad.intro, {
		drive = EquipType.GetEquipType(hyperdrive).name,
		price = Format.Money(price),
	})

	if option == -1 then
		-- Hang up
		form:Close()
		return
	end

	if option == 0 then
		-- Initial proposal
		form:SetFace({ female = ad.isfemale, seed = ad.faceseed, name = ad.name })
		-- Replace token with details of last service (which might have
		-- been seconds ago)
		form:SetMessage(string.interp(message, {
			lasttime = lastServiceMessage(hyperdrive),
		}))
		if hyperdrive == 'NONE' then
			-- er, do nothing, I suppose.
		elseif Game.player:GetMoney() < price then
			form:AddOption(notenoughmoney, -1)
		else
			form:AddOption(ad.yesplease, 1)
		end
		form:AddOption(hangup, -1)
	end

	if option == 1 then
		-- Yes please, service my engine
		form:Clear()
		form:SetFace({ female = ad.isfemale, seed = ad.faceseed, name = ad.name })
		if Game.player:GetMoney() >= price then -- We did check earlier, but...
			-- Say thanks
			form:SetMessage(ad.response)
			form:AddOption(hangup, -1)
			Game.player:AddMoney(-price)
			service_history.lastdate = Game.time
			service_history.company = ad.title
			service_history.jumpcount = 0
		end
	end
end

local onDelete = function (ref)
	ads[ref] = nil
end

local onShipFlavourChanged = function (ship)
	if ship:IsPlayer() then
		service_history.company = default_service_name
		service_history.lastdate = Game.time
	end
end

local onShipEquipmentChanged = function (ship, equipment)
	if ship:IsPlayer() and (EquipType.GetEquipType(equipment).slot == 'ENGINE') then
		service_history.company = default_service_name
		service_history.lastdate = Game.time
	end
end

local onCreateBB = function (station)
	local rand = Rand.New(station.seed + seedbump)
	local n = rand:Integer(1,#service_flavours)
	local isfemale = rand:Integer(1) == 1
	local name = NameGen.FullName(isfemale,rand)

	local ad = {
		name = name,
		isfemale = isfemale,
		-- Only replace tokens which are not subject to further change
		title = string.interp(service_flavours[n].title, {
			name = station.label,
			proprietor = name,
		}),
		intro = string.interp(service_flavours[n].intro, {
			name = station.label,
			proprietor = name,
		}),
		yesplease = service_flavours[n].yesplease,
		response = service_flavours[n].response,
		station = station,
		faceseed = rand:Integer(),
		baseprice = rand:Number(2,10),
	}

	local ref = station:AddAdvert(ad.title, onChat, onDelete)
	ads[ref] = ad
end

local loaded_data

local onGameStart = function ()
	ads = {}

	if not loaded_data then return end

	for k,ad in pairs(loaded_data.ads) do
		local ref = ad.station:AddAdvert(ad.title, onChat, onDelete)
		ads[ref] = ad
	end

	service_history = loaded_data.service_history

	loaded_data = nil
end

local onEnterSystem = function (ship)
	if ship:IsPlayer() and (service_history.lastdate + service_period < Game.time) then
		service_history.jumpcount = service_history.jumpcount + 1
		if (service_history.jumpcount > max_jumps_unserviced) or (Engine.rand:Integer(max_jumps_unserviced - service_history.jumpcount) == 0) then
			-- Destroy the engine
			local engine = ship:GetEquip('ENGINE',0)
			ship:RemoveEquip(engine)
			ship:AddEquip('RUBBISH',EquipType.GetEquipType(engine).mass)
			UI.Message(broken_hyperdrive)
		end
	end
end

local serialize = function ()
	return { ads = ads, service_history = service_history }
end

local unserialize = function (data)
	loaded_data = data
end

EventQueue.onCreateBB:Connect(onCreateBB)
EventQueue.onGameStart:Connect(onGameStart)
EventQueue.onShipFlavourChanged:Connect(onShipFlavourChanged)
EventQueue.onShipEquipmentChanged:Connect(onShipEquipmentChanged)
EventQueue.onEnterSystem:Connect(onEnterSystem)

Serializer:Register("BreakdownServicing", serialize, unserialize)
