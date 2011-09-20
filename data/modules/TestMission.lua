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
        intro = "We specialise in interstellar engines. All maintenance work guaranteed for a year.",
    }
}

local ads = {}

local onChat = function (form, ref, option)
    local ad = ads[ref]

	if option == -1 then
		form:Close()
		return
	end

	if option == 0 then
		form:SetFace({ female = ad.isfemale, seed = ad.faceseed, name = ad.name })
		form:SetMessage(ad.intro)
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
		intro = test_flavours[n].intro,
		station = station,
		faceseed = rand:Integer(),
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
