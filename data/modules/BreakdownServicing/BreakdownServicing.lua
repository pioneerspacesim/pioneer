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
			-- Destroy the engine
			local engine = ship:GetEquip('ENGINE',1)
			ship:RemoveEquip(engine)
			ship:AddEquip('RUBBISH',EquipType.GetEquipType(engine).mass)
			UI.Message(t("The ship's hyperdrive has been destroyed by a malfunction"))
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
