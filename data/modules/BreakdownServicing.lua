local test_flavours = {
    {
        title = "{name} Engine Servicing Company",
        intro = [[Avoid the inconvenience of a broken-down hyperspace engine.  Get yours serviced today, by the officially endorsed {name} Engine Servicing Company.

Engine: {drive}
Service: {price}
Guarantee: 1 year]],
        yesplease = "Service hyperspace engine",
        response = "Your engine has been serviced.",
    },{
        title = "{proprietor}: Hyperdrive maintenance specialist",
        intro = "I'm {proprietor}.  I can service your {drive}, guaranteeing at least a year of trouble-free performance.  The cost for this service will be {price}",
        yesplease = "Service my drive",
        response = "I have serviced your hyperdrive.",
    },{
        title = "{proprietor} & Co HyperMechanics",
        intro = [[Hi there.  We at {proprietor} & Co stake our reputation on our work.

We can tune your ship's {drive}, ensuring 12 months of trouble-free operation, for the sum of {price}.  I'll be supervising the work myself, so you can be sure that a good job will be done.]],
        yesplease = "Please tune my drive at the quoted price",
        response = "Service complete.  Thanks for your custom.",
    },{
        title = "SuperFix Maintenance ({name} branch)",
        intro = [[Welcome SuperFix Maintenance.

Time for your annual maintenance? Let us SuperFix your hyperdrive!
We can tune your {drive} for just {price}.  There's nobody cheaper on this station!]],
        yesplease = "SuperFix me!",
        response = "Your SuperFix service is complete, with SuperFix guarantee!",
    },{
        title = "Time and Space Engines, Inc.",
        intro = [[Welcome to Time and Space.
        
We specialise in interstellar engines. All maintenance work guaranteed for a year.
Servicing your {drive} will cost {price}.  Would you like to proceed?]],
        yesplease = "Yes, please proceed",
        response = "We have completed the work on your hyperdrive.",
    }
}

local hangup = "Hang up"

local ads = {}

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
    else -- 
        price = price * 10
    end

    -- Now make it bigger (-:
    price = price * 100

    message = string.interp(ad.intro, {
        drive = EquipType.GetEquipType(hyperdrive).name,
        price = Format.Money(price),
    })

	if option == -1 then
		form:Close()
		return
	end

	if option == 0 then
		form:SetFace({ female = ad.isfemale, seed = ad.faceseed, name = ad.name })
		form:SetMessage(message)
		form:AddOption(ad.yesplease, 1)
		form:AddOption(hangup, -1)
    end

    if option == 1 then
        form:Clear()
		form:SetFace({ female = ad.isfemale, seed = ad.faceseed, name = ad.name })
		form:SetMessage(ad.response)
		form:AddOption(hangup, -1)
    end
end

local onDelete = function (ref)
	ads[ref] = nil
end

local onCreateBB = function (station)
    -- 10, guaranteed random by D16 dice roll.
    -- This is to make the name different from the station welcome character.
    local rand = Rand.New(station.seed + 10)
	local n = rand:Integer(1,#test_flavours)
    local isfemale = rand:Integer(1) == 1
    local name = NameGen.FullName(isfemale,rand)

	local ad = {
        name = name,
        isfemale = isfemale,
		title = string.interp(test_flavours[n].title, {
            name = station.label,
            proprietor = name,
        }),
		intro = string.interp(test_flavours[n].intro, {
            name = station.label,
            proprietor = name,
        }),
        yesplease = test_flavours[n].yesplease,
        response = test_flavours[n].response,
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

	if not loaded_data then return end

	for k,ad in pairs(loaded_data.ads) do
		local ref = ad.station:AddAdvert(ad.title, onChat, onDelete)
		ads[ref] = ad
	end

	loaded_data = nil
end

local serialize = function ()
	return { ads = ads }
end

local unserialize = function (data)
	loaded_data = data
end

EventQueue.onCreateBB:Connect(onCreateBB)
EventQueue.onGameStart:Connect(onGameStart)

Serializer:Register("TestMission", serialize, unserialize)
