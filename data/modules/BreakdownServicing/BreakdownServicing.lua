-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
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


local RepairHyperdrive = function ()
		--Define the materials needed
	local function round(num, idp)
		local mult = 10^(idp or 0)
		return math.floor(num * mult + 0.5) / mult
	end
	local preciousmetals = Game.player:GetEquipCount('CARGO', 'PRECIOUS_METALS')
	local rubbish = Game.player:GetEquipCount('CARGO','RUBBISH')
	local metalalloys = Game.player:GetEquipCount('CARGO','METAL_ALLOYS')
	--If the player is landed, and has the correct materiels, replaces the drive.
	if Game.player.flightState == 'LANDED' and service_history.hyperdrivedestroyed == true then	
		if preciousmetals >= 2 and rubbish >= service_history.Rubbish and metalalloys >= 2 then
			-- Add engine and remove materials
			Game.player:RemoveEquip('RUBBISH', service_history.Rubbish)
			Game.player:RemoveEquip('PRECIOUS_METALS', 2)
			Game.player:RemoveEquip('METAL_ALLOYS',2)
			--Generate a random time between 3 and 6 hours.
			local seconds = Engine.rand:Integer(10800, 21600)
			local hours = (seconds / 60) / 60
			Comms.ImportantMessage(t"Hyperdrive repairs begun. ETC is " .. round (hours, 1) .. " hrs")
			--Define fuel
			--Keep the player from taking off
			local fuel = Game.player.fuel
			Game.player:SetFuelPercent(0)
			--Keep player landed for x hours and then display message and set fuel.
			Timer:CallAt(Game.time + seconds, function () Game.player:SetFuelPercent(fuel) end)
			Timer:CallAt(Game.time + seconds, function () Comms.ImportantMessage(t('Hyperdrive repairs complete. The repaired drive should be good for 50-100 more jumps')) end)
			Timer:CallAt(Game.time + seconds, function () Game.player:AddEquip(service_history.engine, 1) end)
			--Reset service history and give the player a random number of more jumps
			service_history.hyperdrivedestroyed = false
			service_history.jumpcount = Engine.rand:Integer(100 , 200)
			service_history.Rubbish = 0
		end	
	end
end




GetDriveCondition = function ()

	local ui = Engine.ui
	
	local RepairButton = function ()
		local preciousmetals = Game.player:GetEquipCount('CARGO', 'PRECIOUS_METALS')
		local rubbish = Game.player:GetEquipCount('CARGO','RUBBISH')
		local metalalloys = Game.player:GetEquipCount('CARGO','METAL_ALLOYS')
		if preciousmetals >= 2 and rubbish >= service_history.Rubbish and metalalloys >= 2 then
			repairButton = UI.SmallLabeledButtonLeftText.New ("Destroyed. Repair?")
			return repairButton.widget
		else
			return ui:Label('Destroyed')
		end
	end
	
	if service_history.hyperdrivedestroyed == true and Game.player.flightState == 'LANDED' then
		return 
			RepairButton (),
				repairButton.button.onClick:Connect(RepairHyperdrive)
	elseif Game.player:GetEquip('ENGINE') == 'NONE' and service_history.hyperdrivedestroyed == false then
		return ui:Label('None')
	elseif service_history.hyperdrivedestroyed == true and Game.player.flightState ~= 'LANDED' then
		return ui:Label('Destroyed')
	elseif (service_history.lastdate + service_history.service_period > Game.time) then
		return ui:Label('Under Warranty')
	elseif service_history.jumpcount < 25 then
		return ui:Label('Warranty expired')
	elseif service_history.jumpcount  < 50 then
		return ui:Label('Great')
	elseif service_history.jumpcount  < 100 then
		return ui:Label('Good')
	elseif service_history.jumpcount  < 150 then
		return ui:Label('Stable')
	elseif service_history.jumpcount  < 175 then
		return ui:Label('Poor')
	elseif service_history.jumpcount  < 200 then
		return ui:Label('Unstable')
	elseif service_history.jumpcount  < 255 then
		return ui:Label('Failure Imminent')

	end	
end




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
	if ship:IsPlayer() and (EquipType.GetEquipType(equipment).slot == 'ENGINE') and Game.player:GetEquip('ENGINE') ~= 'NONE' and EquipType.GetEquipType(equipment) ~= service_history.engine then
		service_history.company = nil
		service_history.lastdate = Game.time
		service_history.jumpcount = 0
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
	if ship:IsPlayer() and (service_history.lastdate + service_history.service_period < Game.time) then 
		service_history.jumpcount = service_history.jumpcount + 1
		if (service_history.jumpcount > max_jumps_unserviced) or (Engine.rand:Integer(max_jumps_unserviced - service_history.jumpcount) == 0) then
			-- Destroy the engine and get amount of rubbish equal to the engine weight.
			service_history.engine = Game.player:GetEquip('ENGINE',1)
			Game.player:RemoveEquip(service_history.engine)
			Game.player:AddEquip('RUBBISH',EquipType.GetEquipType(service_history.engine).mass)
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
		--Make sure player has the correct amount of materials. The value of these should be at least equal to a class one hyperdrive.
		--If amount is incorrect, tell player
		local preciousmetals = Game.player:GetEquipCount('CARGO', 'PRECIOUS_METALS')
		local rubbish = Game.player:GetEquipCount('CARGO','RUBBISH')
		local metalalloys = Game.player:GetEquipCount('CARGO','METAL_ALLOYS')
		if (preciousmetals >= 2) and (rubbish >= service_history.Rubbish) and (metalalloys >= 2) then
			Comms.ImportantMessage('You have enough materials to salvage your hyperdrive.')
		elseif (preciousmetals < 2) or (rubbish < service_history.Rubbish) or (metalalloys < 2) then
			Comms.ImportantMessage('You do not have enough materials to salvage your hyperdrive.')	
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
Event.Register("onShipLanded",onShipLanded)


Serializer:Register("BreakdownServicing", serialize, unserialize)
