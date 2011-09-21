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

-- Default numeric values --
----------------------------
local service_period = 31557600 -- One standard Julian year
-- 10, guaranteed random by D16 dice roll.
-- This is to make the BBS name different from the station welcome character.
local seedbump = 10

local ads = {}
local service_history = {
    lastdate = 0, -- Default will be overwritten on game start
    company = default_service_name,
}

local lastServiceMessage = function ()
    -- Fill in the blanks tokens on the {lasttime} string from service_history
    local message
    if service_history.company == default_service_name then
        message = first_service_message
    else
        message = last_service_message
    end
    return string.interp(message, {date = Format.Date(service_history.lastdate), company = service_history.company})
end

local onChat = function (form, ref, option)
    local ad = ads[ref]

    local hyperdrive = Game.player:GetEquip('ENGINE',0)

    -- Tariff!  ad.baseprice is from 1 to 100
    local price = ad.baseprice
    if hyperdrive == 'NONE' then
        price = 0
    elseif hyperdrive == 'DRIVE_CLASS2' then
        price = price * 1.2
    elseif hyperdrive == 'DRIVE_CLASS3' then
        price = price * 1.4
    elseif hyperdrive == 'DRIVE_CLASS4' then
        price = price * 1.8
    elseif hyperdrive == 'DRIVE_CLASS5' then
        price = price * 2.6
    elseif hyperdrive == 'DRIVE_CLASS6' then
        price = price * 3.8
    elseif hyperdrive == 'DRIVE_CLASS7' then
        price = price * 5.4
    elseif hyperdrive == 'DRIVE_CLASS8' then
        price = price * 7.0
    elseif hyperdrive == 'DRIVE_MIL1' then
        price = price * 1.2
    elseif hyperdrive == 'DRIVE_MIL2' then
        price = price * 1.6
    elseif hyperdrive == 'DRIVE_MIL3' then
        price = price * 2.8
    elseif hyperdrive == 'DRIVE_MIL4' then
        price = price * 4.0
    else -- Something weird!
        price = price * 10
    end

    -- Now make it bigger (-:
    price = price * 100

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
            lasttime = lastServiceMessage(),
        }))
        if Game.player:GetMoney() < price then
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
        baseprice = rand:Number(10),
	}

	local ref = station:AddAdvert(ad.title, onChat, onDelete)
	ads[ref] = ad
end

local loaded_data

local onGameStart = function ()
	ads = {}
    service_history.lastdate = Game.time

	if not loaded_data then return end

	for k,ad in pairs(loaded_data.ads) do
		local ref = ad.station:AddAdvert(ad.title, onChat, onDelete)
		ads[ref] = ad
	end

    service_history = loaded_data.service_history

	loaded_data = nil
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

Serializer:Register("BreakdownServicing", serialize, unserialize)
