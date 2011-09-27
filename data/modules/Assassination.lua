-- don't produce missions for further than this many light years away
local max_ass_dist = 30

-- ass flavours indeed ;-)
local ass_flavours = {
	{
		adtext = "WANTED: Removal of {target} from the {system} system.",
		introtext = "Hi, I'm {name}. I'll pay you {cash} to get rid of {target}.",
		successmsg = "News of {target}'s long vacation gratefully received. Well done, I have initiated your full payment.",
		failuremsg = "I am most displeased to find that {target} is still alive. Needless to say you will receive no payment.",
		failuremsg2 = "{target}'s removal was not done by you. No payment this time.",
	},
	{
		adtext = "WANTED: Someone to kill {target} from the {system} system.",
		introtext = "I need {target} taken out of the picture. I'll pay you {cash} to do this.",
		successmsg = "I am most sad to hear of {target}'s demise. You have been paid in full.",
		failuremsg = "I hear that {target} is in good health. This pains me.",
		failuremsg2 = "{target}'s demise was not caused by you, so do not ask for payment.",
	},
	{
		adtext = "REMOVAL: {target} is no longer wanted in the {system} system.",
		introtext = "I am {name}, and I will pay you {cash} to terminate {target}.",
		successmsg = "You have been paid in full for the completion of that important contract.",
		failuremsg = "It is most regrettable that {target} is still live and well. You will receive no payment as you did not complete your contract.",
		failuremsg2 = "Contract was completed by someone else. Be faster next time!",
	},
	{
		adtext = "TERMINATION: Someone to eliminate {target}.",
		introtext = "The {target} must be reduced to space dust. I'll award you {cash} to do this.",
		successmsg = "{target} is dead. Here is your award.",
		failuremsg = "You will pay for not eliminating {target}!",
		failuremsg2 = "Are you asking money for job done by someone else? Get lost.",
	},
	{
		adtext = "RETIREMENT: Someone to retire {target}.",
		introtext = "For {cash} we wish to encourage {target} to stop work permanently.",
		successmsg = "News of {target}'s retirement delightfully obtained. Here is your money.",
		failuremsg = "{target} is still breathing and I'm not giving money to you.",
		failuremsg2 = "Retirement of {target} was done by someone else.",
	},
	{
		adtext = "BIOGRAPHICAL: Some admirers wish {target} dead.",
		introtext = "We wish {target} to have a fitting career end in the {system} system for {cash}.",
		successmsg = "Message of {target}'s ending career happily acquired. Here is your {cash}.",
		failuremsg = "We found out that {target} is nonetheless operative. This sadness us.",
		failuremsg2 = "{target} was neutralized by someone else.",
	}
}

local ads = {}
local missions = {}

local onDelete = function (ref)
	ads[ref] = nil
end

local onChat = function (form, ref, option)
	local ad = ads[ref]

	form:Clear()

	if option == -1 then
		form:Close()
		return
	elseif option == 0 then
		form:SetFace({ female = ad.isfemale, seed = ad.faceseed })
		local sys = ad.location:GetStarSystem()

		local introtext = string.interp(ass_flavours[ad.flavour].introtext, {
			name	= ad.client,
			cash	= Format.Money(ad.reward),
			target	= ad.target,
			system	= sys.name,
		})
		form:SetMessage(introtext)

	elseif option == 1 then
		local sys = ad.location:GetStarSystem()
		local sbody = ad.location:GetSystemBody()

		form:SetMessage(string.format("%s will be leaving %s in the %s system (%s, %s, %s) at %s. The ship is %s and has registration id %s.", ad.target, sbody.name, sys.name, ad.location.sectorX, ad.location.sectorY, ad.location.sectorZ, Format.Date(ad.due), ad.shipname, ad.shipregid) )

	elseif option == 2 then
		local sbody = ad.location:GetSystemBody()

		form:SetMessage(string.format('It must be done after %s leaves %s. Do not miss this opportunity.', ad.target, sbody.name) )

	elseif option == 3 then
		local backstation = Game.player:GetDockedWith().path

		form:RemoveAdvertOnClose()

		ads[ref] = nil

		local mission = {
			type		= "Assassination",
			backstation	= backstation,
			boss		= ad.client,
			client		= ad.shipname .. "\n(" .. ad.shipregid .. ")",
			danger		= ad.danger,
			due		= ad.due,
			flavour		= ad.flavour,
			location	= ad.location,
			reward		= ad.reward,
			shipname	= ad.shipname,
			shipregid	= ad.shipregid,
			status		= 'ACTIVE',
			target		= ad.target,
		}

		local mref = Game.player:AddMission(mission)
		missions[mref] = mission

		form:SetMessage("Excellent.")
		form:AddOption("Hang up.", -1)

		return
	elseif option == 4 then
		form:SetMessage("Return here on the completion of the contract and you will be paid.")
	end
	form:AddOption(string.format("Where can I find %s?", ad.target), 1);
	form:AddOption("Could you repeat the original request?", 0);
	form:AddOption("How soon must it be done?", 2);
	form:AddOption("How will I be paid?", 4);
	form:AddOption("Ok, agreed.", 3);
	form:AddOption("Hang up.", -1);
end

local RandomShipRegId = function ()
	local letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	local a = Engine.rand:Integer(1, #letters)
	local b = Engine.rand:Integer(1, #letters)
	return string.format("%s%s-%04d", letters:sub(a,a), letters:sub(b,b), Engine.rand:Integer(0, 9999))
end

local makeAdvert = function (station)
	local nearbysystems = Game.system:GetNearbySystems(max_ass_dist, function (s) return #s:GetStationPaths() > 0 end)
	if #nearbysystems == 0 then return end
	local isfemale = Engine.rand:Integer(1) == 1
	local client = NameGen.FullName(isfemale)
	local targetIsfemale = Engine.rand:Integer(1) == 1
	local title = { -- just for fun
		"Admiral",
		"Ambassador",
		"Brigadier",
		"Cadet",
		"Captain",
		"Cardinal",
		"Colonel",
		"Commandant",
		"Commodore",
		"Corporal",
		"Ensign",
		"General",
		"Judge",
		"Lawyer",
		"Lieutenant",
		"Marshal",
		"Merchant",
		"Officer",
		"Private",
		"Professor",
		"Prosecutor",
		"Provost",
		"Seaman",
		"Senator",
		"Sergeant",
	}
	local target = title[Engine.rand:Integer(1, #title)] .. " " .. NameGen.FullName(targetIsfemale)
	local flavour = Engine.rand:Integer(1, #ass_flavours)
	local nearbysystem = nearbysystems[Engine.rand:Integer(1,#nearbysystems)]
	local nearbystations = nearbysystem:GetStationPaths()
	local location = nearbystations[Engine.rand:Integer(1,#nearbystations)]
	local time = Engine.rand:Number(0.3, 3)
	local due = Game.time + Engine.rand:Number(7*60*60*24, time * 31*60*60*24)
	local danger = Engine.rand:Integer(1,4)
	local reward = Engine.rand:Number(2100, 7000) * danger
	local shiptypes = ShipType.GetShipTypes('SHIP', function (t) return t.hullMass >= (danger * 17) end)
	local shipname = shiptypes[Engine.rand:Integer(1,#shiptypes)]

	local ad = {
		client = client,
		danger = danger,
		due = due,
		faceseed = Engine.rand:Integer(),
		flavour = flavour,
		isfemale = isfemale,
		location = location,
		reward = reward,
		shipname = shipname,
		shipregid = RandomShipRegId(),
		station = station,
		target = target,
	}

	ad.desc = string.interp(ass_flavours[ad.flavour].adtext, {
		target	= ad.target,
		system	= nearbysystem.name,
	})
	local ref = station:AddAdvert(ad.desc, onChat, onDelete)
	ads[ref] = ad
end

local onCreateBB = function (station)
	local num = Engine.rand:Integer(0, math.ceil(Game.system.population) / 2 + 1)
	for i = 1,num do
		makeAdvert(station)
	end
end

local onShipHit = function (ship, attacker)
	if attacker and not attacker:IsPlayer() then return end -- XX

	-- When the player attacks the target, make it fight back
	for k,mission in pairs(missions) do
		if mission.status == 'ACTIVE' and
		   mission.ship == ship then
			ship:AIKill(attacker)
			return
		end
	end
end

local onShipDestroyed = function (ship, body)
	for ref, mission in pairs(missions) do
		if mission.status == 'ACTIVE' and
		   mission.ship == ship and
		   mission.due < Game.time then
			if not body:isa("Ship") or
			   not body:IsPlayer() then
				mission.status = 'FAILED'
				mission.notplayer = 'TRUE'
			else -- well done, comrade
				mission.status = 'COMPLETED'
				mission.client = mission.boss
				mission.location = mission.backstation
				mission.notplayer = 'FALSE'
			end
			mission.ship = nil
			Game.player:UpdateMission(ref, mission)
			return
		end
	end
end

local _setupHooksForMission = function (mission)
	if mission.ship:exists() and
	   mission.due > Game.time then
		-- Target hasn't launched yet. set up a timer to do this
		Timer:CallAt(mission.due, function () if mission.ship:exists() then mission.ship:Undock()
			mission.timer = nil end end)
		mission.timer = 'SET'
	end
end

local onEnterSystem = function (ship)
	if not ship:IsPlayer() then return end

	local syspath = Game.system.path

	for ref,mission in pairs(missions) do
		if mission.status == 'ACTIVE' then
			if not mission.ship then
				if mission.due > Game.time then
					if mission.location:IsSameSystem(syspath) then -- spawn our target ship
						local station = Space.GetBody(mission.location.bodyIndex)
						local shiptype = ShipType.GetShipType(mission.shipname)
						local default_drive = shiptype.defaultHyperdrive
						local lasers = EquipType.GetEquipTypes('LASER', function (e,et) return et.slot == "LASER" end)
						local count = tonumber(string.sub(default_drive, -1)) ^ 2
						local laser = lasers[mission.danger]

						mission.ship = Space.SpawnShipDocked(mission.shipname, station)
						if mission.ship == nil then
							return -- TODO
						end
						mission.ship:SetLabel(mission.shipregid)
						mission.ship:AddEquip(default_drive)
						mission.ship:AddEquip(laser)
						mission.ship:AddEquip('SHIELD_GENERATOR', mission.danger)
						mission.ship:AddEquip('HYDROGEN', count)
						if mission.danger > 2 then
							mission.ship:AddEquip('SHIELD_ENERGY_BOOSTER')
						end
						if mission.danger > 3 then
							mission.ship:AddEquip('LASER_COOLING_BOOSTER')
						end
						_setupHooksForMission(mission)
						mission.shipstate = 'docked'
					end
				else	-- too late
					mission.status = 'FAILED'
					ship:UpdateMission(ref, mission)
				end
			else
				if mission.ship:exists() then
					local planets = Space.GetBodies(function (body) return body:isa("Planet") end)
					if #planets == 0 then return end
					local planet = planets[Engine.rand:Integer(1,#planets)]
					mission.ship:AIEnterHighOrbit(planet)
				else
					mission.ship = nil
					if mission.due < Game.time then
						mission.status = 'FAILED'
						ship:UpdateMission(ref, mission)
					end
				end
			end
		end
	end
end

local onShipDocked = function (ship, station)
	for ref,mission in pairs(missions) do
		if ship:IsPlayer() then
			if mission.status == 'COMPLETED' and
			   mission.backstation == station.path then
				local text = string.interp(ass_flavours[mission.flavour].successmsg, {
					target	= mission.target,
					cash	= Format.Money(ad.reward),
				})
				UI.ImportantMessage(text, mission.boss)
				ship:AddMoney(mission.reward)
				ship:RemoveMission(ref)
				missions[ref] = nil
			elseif mission.status == 'FAILED' then
				if mission.notplayer == 'TRUE' then
					local text = string.interp(ass_flavours[mission.flavour].failuremsg2, {
						target	= mission.target,
					})
				else
					local text = string.interp(ass_flavours[mission.flavour].failuremsg, {
						target	= mission.target,
					})
				end
				UI.ImportantMessage(text, mission.boss)
				ship:RemoveMission(ref)
				missions[ref] = nil
			end
		else
			if mission.ship == ship then
				mission.status = 'FAILED'
				Game.player:UpdateMission(ref, mission)
			end
		end
		return
	end
end
	
local onShipUndocked = function (ship, station)
	if ship:IsPlayer() then return end -- not interested in player, yet

	for ref,mission in pairs(missions) do
		if mission.status == 'ACTIVE' and
		   mission.ship == ship then
			local planets = Space.GetBodies(function (body) return body:isa("Planet") end)
			if #planets == 0 then
				local stats = ship:GetStats()
				local systems = Game.system:GetNearbySystems(stats.hyperspaceRange, function (s) return #s:GetStationPaths() > 0 end)
				if #systems == 0 then return end
				local system = systems[Engine.rand:Integer(1,#systems)]

				ship:HyperspaceTo(system.path)
			else
				local planet = planets[Engine.rand:Integer(1,#planets)]

				mission.ship:AIEnterHighOrbit(planet)
				mission.shipstate = 'flying'
			end
			return
		end
	end
end

local onAICompleted = function (ship)
	for ref,mission in pairs(missions) do
		if mission.status == 'ACTIVE' and
		   mission.ship == ship and
		   mission.shipstate == 'flying' then
			Timer:CallAt(Game.time + 60 * 60 * 8, function () if mission.ship:exists() then
				local stations = Space.GetBodies(function (body) return body:isa("SpaceStation") end)
				if #stations == 0 then return end
				local station = stations[Engine.rand:Integer(1,#stations)]

				mission.ship:AIDockWith(station)
				end end)
			return
		end
	end
end

local onUpdateBB = function (station)
	for ref,ad in pairs(ads) do
		if (ad.due < Game.time + 5*60*60*24) then
			ad.station:RemoveAdvert(ref)
		end
	end
	if Engine.rand:Integer(12*60*60) < 60*60 then -- roughly once every twelve hours
		makeAdvert(station)
	end
end
	
local loaded_data

local onGameStart = function ()
	ads = {}
	missions = {}

	if not loaded_data then return end

	for k,ad in pairs(loaded_data.ads) do
		local ref = ad.station:AddAdvert(ad.desc, onChat, onDelete)
		ads[ref] = ad
	end
	for k,mission in pairs(loaded_data.missions) do
		local mref = Game.player:AddMission(mission)
		missions[mref] = mission
	end

	loaded_data = nil
end

local serialize = function ()
	return { ads = ads, missions = missions }
end

local unserialize = function (data)
	loaded_data = data
	for k,mission in pairs(loaded_data.missions) do
		if mission.ship and
		   mission.ship:exists() and
		   mission.timer == 'SET' then
			Timer:CallAt(mission.due, function () if mission.ship:exists() then mission.ship:Undock()
				mission.timer = nil end end)
		end
	end
end

EventQueue.onCreateBB:Connect(onCreateBB)
EventQueue.onGameStart:Connect(onGameStart)
EventQueue.onEnterSystem:Connect(onEnterSystem)
EventQueue.onShipDestroyed:Connect(onShipDestroyed)
EventQueue.onShipUndocked:Connect(onShipUndocked)
EventQueue.onAICompleted:Connect(onAICompleted)
EventQueue.onShipDocked:Connect(onShipDocked)
EventQueue.onShipHit:Connect(onShipHit)
EventQueue.onUpdateBB:Connect(onUpdateBB)

Serializer:Register("Assassination", serialize, unserialize)
