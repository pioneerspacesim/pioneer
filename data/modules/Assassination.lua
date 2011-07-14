-- don't produce missions for further than this many light years away
local max_ass_dist = 20

-- ass flavours indeed ;-)
local ass_flavours = {
	{
		adtext = "WANTED: Removal of {target} from the {system} system.",
		introtext = "Hi, I'm {name}. I'll pay you {cash} to get rid of {target}.",
		successmsg = "News of {target}'s long vacation gratefully received. Well done, I have initiated your full payment.",
		failuremsg = "I am most displeased to find that {target} is still alive. Needless to say you will receive no payment.",
		failuremsg2 = "{target}'s removal was not done by you. No payment this time.",
		danger = 0,
		time = 2.3,
		money = 0.8,
	},
	{
		adtext = "WANTED: Someone to kill {target} from the {system} system.",
		introtext = "I need {target} taken out of the picture. I'll pay you {cash} to do this.",
		successmsg = "I am most sad to hear of {target}'s demise. You have been paid in full.",
		failuremsg = "I hear that {target} is in good health. This pains me.",
		failuremsg2 = "{target}'s demise was not caused by you, so do not ask for payement.",
		danger = 0,
		time = 2.1,
		money = 1.0,
	},
	{
		adtext = "REMOVAL: {target} is no longer wanted in the {system} system.",
		introtext = "I am {name}, and I will pay you {cash} to terminate {target}",
		successmsg = "You have been paid in full for the completion of that important contract.",
		failuremsg = "It is most regrettable that {target} is still live and well. You will receive no payment as you did not complete your contract.",
		failuremsg2 = "Contract was completed by someone else. Be faster next time!",
		danger = 1,
		time = 1.9,
		money = 1.2,
	},
	{
		adtext = "TERMINATION: Someone to eliminate {target}.",
		introtext = "The {target} must be reduced to space dust. I'll award you {cash} to do this.",
		successmsg = "{target} is dead. Here is your award.",
		failuremsg = "You will pay for not eliminating {target}!",
		failuremsg2 = "Are you asking money for job done by someone else? Get lost.",
		danger = 2,
		time = 2,
		money = 1.4,
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

		local introtext = string.interp(ass_flavours[ad.flavour].introtext, {
			name	= ad.client,
			cash	= Format.Money(ad.reward),
			target	= ad.target,
		})
		form:SetMessage(introtext)

	elseif option == 1 then
		local sys = ad.location:GetStarSystem()
		local sbody = ad.location:GetSystemBody()

		form:SetMessage(string.format("%s will be leaving %s in the %s system (%s, %s) at %s. The ship is %s and has registration id %s.", ad.target, sbody.name, sys.name, ad.location.sectorX, ad.location.sectorY, Format.Date(ad.due), ad.shipname, ad.shipregid) )

	elseif option == 2 then
		local sbody = ad.location:GetSystemBody()

		form:SetMessage(string.format('It must be done after %s leaves %s. Do not miss this opportunity.', ad.target, sbody.name) )

	elseif option == 3 then
		local backstation = Game.player:GetDockedWith().path

		form:RemoveAdvertOnClose()

		ads[ref] = nil

		local mission = {
			type		= "Assassination",
			client		= ad.shipname .. "\n(" .. ad.shipregid .. ")",
			boss		= ad.client,
			location	= ad.location,
			reward		= ad.reward,
			due		= ad.due,
			flavour		= ad.flavour,
			target		= ad.target,
			backstation	= backstation,
			shipregid	= ad.shipregid,
			shipname	= ad.shipname,
			status		= 'ACTIVE',
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
	local title = { "Admiral", "Senator", "General", "Colonel", "Comandante", "Cardinal", "Professor", "Ambassador", "Judge", "Captain" }
	local target = title[Engine.rand:Integer(1, #title)] .. " " .. NameGen.FullName(targetIsfemale)
	local flavour = Engine.rand:Integer(1, #ass_flavours)
	local nearbysystem = nearbysystems[Engine.rand:Integer(1,#nearbysystems)]
	local nearbystations = nearbysystem:GetStationPaths()
	local location = nearbystations[Engine.rand:Integer(1,#nearbystations)]
	local due = Game.time + Engine.rand:Number(7*60*60*24, ass_flavours[flavour].time * 31*60*60*24)
	local reward = Engine.rand:Number(2000, 15000) * ass_flavours[flavour].money
	local shiptypes = ShipType.GetShipTypes('SHIP')
	local shipname = shiptypes[Engine.rand:Integer(1,#shiptypes)]

	local ad = {
		station = station,
		flavour = flavour,
		client = client,
		location = location,
		due = due,
		reward = reward,
		isfemale = isfemale,
		faceseed = Engine.rand:Integer(),
		target = target,
		shipregid = RandomShipRegId(),
		shipname = shipname,
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
	if not attacker:IsPlayer() then return end

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
				Game.player:UpdateMission(ref, mission)
				return
			else
				-- well done, comrade
				mission.status = 'COMPLETED'
				mission.client = mission.boss
				mission.location = mission.backstation
				mission.notplayer = 'FALSE'
				body:UpdateMission(ref, mission)
				return
			end
		end
	end
end

local _setupHooksForMission = function (mission)
	if mission.ship ~= nil and
	   mission.due > Game.time then
		-- Target hasn't launched yet. set up a timer to do this
		Timer:CallAt(mission.due, function () mission.ship:Undock()
			mission.timer = nil end)
		mission.timer = 'SET'
	end
end

local onEnterSystem = function (ship)
	if not ship:IsPlayer() then return end

	local syspath = Game.system.path

	for ref,mission in pairs(missions) do
		if mission.status == 'ACTIVE' and
			mission.ship == nil then
			if mission.location:IsSameSystem(syspath) then
				if mission.due > Game.time then -- spawn our target ship
					local danger = ass_flavours[mission.flavour].danger + 1
					local station = Space.GetBody(mission.location.bodyIndex)
					local shiptype = ShipType.GetShipType(mission.shipname)
					local default_drive = shiptype.defaultHyperdrive
					local lasers = EquipType.GetEquipTypes('LASER', function (e,et)
						return et.slot == "LASER"
					end)
					local laser = lasers[danger]
					ship = Space.SpawnShipDocked(mission.shipname, station)
					mission.ship = ship
					ship:SetLabel(mission.shipregid)
					ship:AddEquip(default_drive)
					ship:AddEquip('SHIELD_GENERATOR', danger)
					ship:AddEquip(laser)
					ship:AddEquip('HYDROGEN', danger * 3)
					_setupHooksForMission(mission)
				else	-- too late
					mission.status = 'FAILED'
					ship:UpdateMission(ref, mission)
				end
			elseif mission.ship then
				local planets = Space.GetBodies(function (body) return body:isa("Planet") end)
				local planet = planets[Engine.rand:Integer(1,#planets)]
				mission.ship:AIFlyTo(planet)
			elseif mission.due < Game.time then
				mission.status = 'FAILED'
				ship:UpdateMission(ref, mission)
			end
		end
	end
end

local onShipDocked = function (ship, station)
	if not ship:IsPlayer() then return end

	for ref,mission in pairs(missions) do
		if mission.status == 'COMPLETED' and
		   mission.backstation == station.path then
			local text = string.interp(ass_flavours[mission.flavour].successmsg, {
				target	= mission.target,
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
	end
end
	
local onShipUndocked = function (ship, station)
	if ship:IsPlayer() then return end -- not interested in player, yet

	for ref,mission in pairs(missions) do
		if mission.status == 'ACTIVE' and
		   mission.ship == ship then
			local planets = Space.GetBodies(function (body) return body:isa("Planet") end)
			local planet = planets[Engine.rand:Integer(1,#planets)]
			mission.ship:AIFlyTo(planet)
		end
	end
end

local onUpdateBB = function (station)
	for ref,ad in pairs(ads) do
		if (ad.due < Game.time + 5*60*60*24) then
			ads[ref] = nil
			station:RemoveAdvert(ref)
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
		   mission.timer == 'SET' then
			Timer:CallAt(mission.due, function () mission.ship:Undock()
				mission.timer = nil end)
		end
	end
end

EventQueue.onCreateBB:Connect(onCreateBB)
EventQueue.onGameStart:Connect(onGameStart)
EventQueue.onEnterSystem:Connect(onEnterSystem)
EventQueue.onShipDestroyed:Connect(onShipDestroyed)
EventQueue.onShipUndocked:Connect(onShipUndocked)
EventQueue.onShipDocked:Connect(onShipDocked)
EventQueue.onShipHit:Connect(onShipHit)
EventQueue.onUpdateBB:Connect(onUpdateBB)

Serializer:Register("Assassination", serialize, unserialize)
