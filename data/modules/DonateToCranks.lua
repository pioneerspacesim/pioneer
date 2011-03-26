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

local onActivate = function (dialog, ref, option)
	local ad = ads[ref]

	if not option then
		dialog:clear();

		dialog:set_title(ad.flavour.title)
		dialog:set_message(ad.flavour.message)

		dialog:add_option("$1", 1);
		dialog:add_option("$10", 10);
		dialog:add_option("$100", 100);
		dialog:add_option("$1000", 1000);
		dialog:add_option("$10000", 10000);
		dialog:add_option("$100000", 100000);
		dialog:add_option("Hang up.", -1);

		return
	end

	if option == -1 then
		dialog:close()
		return
	end

	local player = Pi.GetPlayer()
	if player:get_money() < option then
		Pi.Message("", "You do not have enough money.")
	else
		if optionClicked >= 10000 then
			Pi.Message("", "Wow! That was very generous.")
		else
			Pi.Message("", "Thank you. All donations are welcome.")
		end
		player:add_money(-optionClicked)
		dialog:refresh()
	end
end

local onDelete = function (ref)
	ads[ref] = nil
end

local onCreateBB = function (station)
	local n = Pi.rand:Int(1, #crank_flavours)
	local ad = {
		id      = #ads+1,
		station = station,
		flavour = crank_flavours[n],
	}

	local ref = station:add_advert(ad.flavour.title, onActivate, onDelete)
	ads[ref] = ad;
end

Module:new {
	__name='DonateToCranks', 
	
	Init = function(self)
		EventQueue.onCreateBB:connect(onCreateBB)
	end,
}

