local crank_flavours = {
	{
		title = "DONATE! The Church of The Celestial Flying Spaghetti Monster needs YOUR money to spread the word of god.",
		message = "Please select an amount to donate to the Church of the Celestial Flying Spaghetti Monster.\n",
	},{
		title = "DONATE. The Guardians of the Free Spirit humbly request your charity to support our monasteries.",
		message = "Peace be with you, brother. Please select an amount to donate to the Guardians of the Free Spirit.\n",
	},{
		title = "FEELING GENEROUS? War Orphan's Support needs your help to keep up its essential work.",
		message = "Please select an amount to donate to War Orphan's Support, and end the suffering of children all over the galaxy.\n"
	}
}

local ads = {}

local onChat = function (dialog, station, ref, option)
	local ad = ads[station][ref]

	if option == 0 then
		dialog:Clear();

		dialog:SetTitle(ad.title)
		dialog:SetMessage(ad.message)

		dialog:AddOption("$1", 1);
		dialog:AddOption("$10", 10);
		dialog:AddOption("$100", 100);
		dialog:AddOption("$1000", 1000);
		dialog:AddOption("$10000", 10000);
		dialog:AddOption("$100000", 100000);
		dialog:AddOption("Hang up.", -1);

		return
	end

	if option == -1 then
		dialog:Close()
		return
	end

	local player = Pi.GetPlayer()
	if player:GetMoney() < option then
		Pi.Message("", "You do not have enough money.")
	else
		if option >= 10000 then
			Pi.Message("", "Wow! That was very generous.")
		else
			Pi.Message("", "Thank you. All donations are welcome.")
		end
		player:AddMoney(-option)
		dialog:Refresh()
	end
end

local onDelete = function (station, ref)
	ads[station][ref] = nil
end

local onCreateBB = function (station)
	ads[station] = {}

	local n = Pi.rand:Int(1, #crank_flavours)
	local ad = crank_flavours[n]

	local ref = station:AddAdvert(ad.title, onChat, onDelete)
	ads[station][ref] = ad;
end

local onDestroyBB = function (station)
	for station, ads in pairs(ads) do
		for ref,ad in (ads) do
			station:RemoveAdvert(ref)
		end
	end
	ads[station] = nil
end

EventQueue.onCreateBB:Connect(onCreateBB)
EventQueue.onDestroyBB:Connect(onDestroyBB)
