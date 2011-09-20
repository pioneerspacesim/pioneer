local test_flavours = {
    {
        title = "Engine servicing",
        intro = "Avoid the inconvenience of a broken-down hyperspace engine.  Get yours serviced today.",
    },{
        title = "Hyperdrive specialists",
        intro = "We can service your hyperdrive, guaranteeing a year of trouble-free performance.",
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
		form:SetFace({ female = ad.isfemale, seed = ad.faceseed, name = ad.client })
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
	local n = Engine.rand:Integer(1, #test_flavours)

	local ad = {
		title = test_flavours[n].title,
		intro = test_flavours[n].intro,
		station = station,
		faceseed = Engine.rand:Integer()
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
