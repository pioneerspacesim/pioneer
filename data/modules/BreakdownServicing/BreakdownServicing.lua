-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- Get the translator function
local t = Translate:GetTranslator()



-- Default numeric values --
----------------------------
local oneyear = 31557600 -- One standard Julian year
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
	company = nil, -- Name of company that did the last service
	service_period = oneyear, -- default
	jumpcount = 0, -- Number of jumps made after the service_period
	engine = '', -- Engine at the time of failure
	Rubbish = 0, --Engine weight at the time of failure
	hyperdrivedestroyed = false, -- Is the hyperdrive actually destroyed?
}


local lastServiceMessage = function (hyperdrive)
	-- Fill in the blanks tokens on the {lasttime} string from service_history
	local message
	if hyperdrive == 'NONE' then
		message = t("You do not have a drive to service!")
	elseif not service_history.company then
		message = t("Your drive has not been serviced since it was installed on {date}")
	else
		message = t("Your drive was last serviced on {date} by {company}")
	end
	return string.interp(message, {date = Format.Date(service_history.lastdate), company = service_history.company})
end

local onChat = function (form, ref, option)
	local ad = ads[ref]

	local hyperdrive = Game.player:GetEquip('ENGINE',1)

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
			form:AddOption(t("I don't have enough money"), -1)
		else
			form:AddOption(ad.yesplease, 1)
		end
		form:AddOption(t('HANG_UP'), -1)
		print(('DEBUG: %.2f years / %.2f price = %.2f'):format(ad.strength, ad.baseprice, ad.strength/ad.baseprice))
	end

	if option == 1 then
		-- Yes please, service my engine
		form:Clear()
		form:SetFace({ female = ad.isfemale, seed = ad.faceseed, name = ad.name })
		if Game.player:GetMoney() >= price then -- We did check earlier, but...
			-- Say thanks
			form:SetMessage(ad.response)
			form:AddOption(t('HANG_UP'), -1)
			Game.player:AddMoney(-price)
			service_history.lastdate = Game.time
			service_history.service_period = ad.strength * oneyear
			service_history.company = ad.title
			service_history.jumpcount = 0
			service_history.engine = ''
			service_history.Rubbish = 0
			service_history.hyperdrivedestroyed = false
			
		end
	end
end

local onDelete = function (ref)
	ads[ref] = nil
end

local onShipFlavourChanged = function (ship)
	if ship:IsPlayer() then
		service_history.company = nil
		service_history.lastdate = Game.time
	end
end

local onShipEquipmentChanged = function (ship, equipment)
	if ship:IsPlayer() and (EquipType.GetEquipType(equipment).slot == 'ENGINE') then
		service_history.company = nil
		service_history.lastdate = Game.time
		service_history.service_period = oneyear
		service_history.jumpcount = 0
		service_history.engine = ''
		service_history.Rubbish = 0
		service_history.hyperdrivedestroyed = false
	end
end

local onCreateBB = function (station)
	local service_flavours = Translate:GetFlavours('BreakdownServicing')
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
		strength = service_flavours[n].strength,
		baseprice = service_flavours[n].baseprice *rand:Number(0.8,1.2), -- A little per-station flavouring
	}

	local ref = station:AddAdvert(ad.title, onChat, onDelete)
	ads[ref] = ad
end

local loaded_data

local onGameStart = function ()
	ads = {}

	if not loaded_data then
		service_history = {
			lastdate = 0, -- Default will be overwritten on game start
			company = nil, -- Name of company that did the last service
			service_period = oneyear, -- default
			jumpcount = 0, -- Number of jumps made after the service_period
			engine = '',
			Rubbish = 0,
			hyperdrivedestroyed = false,
		}
		
	else
		for k,ad in pairs(loaded_data.ads) do
			local ref = ad.station:AddAdvert(ad.title, onChat, onDelete)
			ads[ref] = ad
		end

		service_history = loaded_data.service_history

		loaded_data = nil
	end
end



	

local onEnterSystem = function (ship)
	if ship:IsPlayer() then print(('DEBUG: Jumps since warranty: %d, chance of failure (if > 0): 1/%d\nWarranty expires: %s'):format(service_history.jumpcount,max_jumps_unserviced-service_history.jumpcount,Format.Date(service_history.lastdate + service_history.service_period))) end
	if ship:IsPlayer() then
		service_history.jumpcount = service_history.jumpcount + 255
		if (service_history.jumpcount > max_jumps_unserviced) or (Engine.rand:Integer(max_jumps_unserviced - service_history.jumpcount) == 0) then
			-- Destroy the engine and get amount of rubbish equal to the engine weight.
			service_history.engine = ship:GetEquip('ENGINE',1)
			ship:RemoveEquip(service_history.engine)
			ship:AddEquip('RUBBISH',EquipType.GetEquipType(service_history.engine).mass)
			service_history.Rubbish = (EquipType.GetEquipType(service_history.engine).mass)
			Comms.Message(t("The ship's hyperdrive has been destroyed by a malfunction. Land at the nearest planet to initiate emergency repairs."))
			service_history.hyperdrivedestroyed = true
		end
	end
end


local onShipLanded = function (ship, body)
	--Check if is player and if the hyperdrive is destroyed.
	if ship:IsPlayer() and (service_history.hyperdrivedestroyed == true) then
			--Define the materials needed
			preciousmetals = Game.player:GetEquipCount('CARGO', 'PRECIOUS_METALS')
			rubbish = Game.player:GetEquipCount('CARGO','RUBBISH')
			metalalloys = Game.player:GetEquipCount('CARGO','METAL_ALLOYS')
		--Make sure player has the correct amount of materials. The value of these should be at least equal to a class one hyperdrive.
		--If amount is incorrect, tell player
		if (preciousmetals >= 3) and (rubbish >= service_history.Rubbish) and (metalalloys >= 4) then
			Comms.Message('Unload rubbish to attempt to salvage the old hyperdrive')
		elseif (preciousmetals < 2) or (rubbish < service_history.Rubbish) or (metalalloys < 4) then
			Comms.Message('You do not have enough materials to salvage your hyperdrive.')	
		end	
	end
end


local onCargoUnload = function (body, cargotype)
		--Make sure engine is broken and that rubbish is jettisoned. Can be changed to something else easily.
	if (cargotype == 'RUBBISH') and (service_history.hyperdrivedestroyed == true) then
		-- Make sure the player is landed; not at a spacestation or port. And make sure engine slot is empty... again.
		if (Game.player.flightState == "LANDED") and (Game.player:GetEquipFree('ENGINE') == 1) then
				--Make sure the amount of materials is correct
			if (preciousmetals >= 3) and (rubbish == service_history.Rubbish - 1) and (metalalloys >= 4) then
				-- Add engine and remove materials
				Game.player:RemoveEquip('RUBBISH', service_history.Rubbish)
				Game.player:RemoveEquip('PRECIOUS_METALS', 4)
				Game.player:RemoveEquip('METAL_ALLOYS',3)
				Comms.Message('Hyperdrive repair begun. ETC is 6 hours.')
				--Keep the player from taking off
				local fuel = Game.player.fuel
				Game.player:SetFuelPercent(0)
				--Keep player landed for 6 hours and then display message and set fuel.
				Timer:CallAt(Game.time + 21600, function () Game.player:SetFuelPercent(fuel) end)
				Timer:CallAt(Game.time + 21600, function () Comms.Message('Hyperdrive repairs complete. The repaired drive should be good for 30 more jumps.') end)
				Timer:CallAt(Game.time + 21600, function () Game.player:AddEquip(service_history.engine, 1) end)
				--Reset service history and give the player 30 more jumps
				service_history.hyperdrivedestroyed = false
				service_history.jumpcount = 220
			end
		end	
	end
end

local serialize = function ()
	return { ads = ads, service_history = service_history }
end

local unserialize = function (data)
	loaded_data = data
end

Event.Register("onCreateBB", onCreateBB)
Event.Register("onGameStart", onGameStart)
Event.Register("onShipFlavourChanged", onShipFlavourChanged)
Event.Register("onShipEquipmentChanged", onShipEquipmentChanged)
Event.Register("onEnterSystem", onEnterSystem)
Event.Register("onShipLanded", onShipLanded)
Event.Register ("onCargoUnload", onCargoUnload)

Serializer:Register("BreakdownServicing", serialize, unserialize)
