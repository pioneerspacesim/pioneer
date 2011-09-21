local test_flavours = {
    {
        title = "{name} Engine Servicing Company",
        intro = "Avoid the inconvenience of a broken-down hyperspace engine.  Get yours serviced today.",
    },{
        title = "{proprietor}: Hyperdrive maintenance specialist",
        intro = "I can service your hyperdrive, guaranteeing at least a year of trouble-free performance.",
    },{
        title = "{proprietor} & Co HyperMechanics",
        intro = "You've come to the right place if you'd like us to take a look at your hyperdrive.",
    },{
        title = "SuperFix Maintenance ({name} branch)",
        intro = "Time for your annual tune-up? We can SuperFix up your engine!",
    },{
        title = "Time and Space Engines, Inc.",
        intro = [[Welcome to Time and Space.
        
We specialise in interstellar engines. All maintenance work guaranteed for a year.
Servicing your {drive} will cost {price}.  Would you like to proceed?]]
    }
}

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
		form:AddOption("Fix my ship!", 1)
		form:AddOption("Hang up.", -1)
    end

    if option == 1 then
        UI.Message("You got it.")
    end
end

local onDelete = function (ref)
	ads[ref] = nil
end

local onCreateBB = function (station)
    local rand = Rand.New(station.seed)
	local n = rand:Integer(1,#test_flavours)
    local isfemale = rand:Integer(1) == 1
    local name = NameGen.FullName(isfemale,rand)
    -- And again - otherwise we get the same name as the dude in the main station menu!
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
