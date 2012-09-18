-- Get the translator function
local t = Translate:GetTranslator()

-- don't produce missions for further than this many light years away
local max_ass_dist = 30

local ads = {}
local missions = {}

local onDelete = function (ref)
	ads[ref] = nil
end

local onChat = function (form, ref, option)
	local ass_flavours = Translate:GetFlavours('Assassination')
	local ad = ads[ref]

	form:Clear()

	if option == -1 then
		form:Close()
		return
	elseif option == 0 then
		form:SetFace(ad.client)
		local sys = ad.location:GetStarSystem()

		local introtext = string.interp(ass_flavours[ad.flavour].introtext, {
			name	= ad.client.name,
			cash	= Format.Money(ad.reward),
			target	= ad.target,
			system	= sys.name,
		})
		form:SetMessage(introtext)

	elseif option == 1 then
		local sys = ad.location:GetStarSystem()
		local sbody = ad.location:GetSystemBody()

		form:SetMessage(string.interp(t("{target} will be leaving {spaceport} in the {system} system ({sectorX}, {sectorY}, {sectorZ}) at {date}. The ship is {shipname} and has registration id {shipregid}."), {
		  target    = ad.target,
		  spaceport = sbody.name,
		  system    = sys.name,
		  sectorX   = ad.location.sectorX,
		  sectorY   = ad.location.sectorY,
		  sectorZ   = ad.location.sectorZ,
		  date      = Format.Date(ad.due),
		  shipname  = ad.shipname,
		  shipregid = ad.shipregid,
		  })
		)

	elseif option == 2 then
		local sbody = ad.location:GetSystemBody()

		form:SetMessage(string.interp(t("It must be done after {target} leaves {spaceport}. Do not miss this opportunity."), {
		  target    = ad.target,
		  spaceport = sbody.name,
      })
    )

	elseif option == 3 then
		local backstation = Game.player:GetDockedWith().path

		form:RemoveAdvertOnClose()

		ads[ref] = nil

		local mission = {
			type		= t("Assassination"),
			backstation	= backstation,
			boss		= ad.client.name,
			client		= ad.shipname .. "\n(" .. ad.shipregid .. ")",
			danger		= ad.danger,
			due		= ad.due,
			flavour		= ad.flavour,
			location	= ad.location,
			reward		= ad.reward,
			shipid		= ad.shipid,
			shipname	= ad.shipname,
			shipregid	= ad.shipregid,
			status		= 'ACTIVE',
			target		= ad.target,
		}

		local mref = Game.player:AddMission(mission)
		missions[mref] = mission

		form:SetMessage(t("Excellent."))
		form:AddOption(t('HANG_UP'), -1)

		return
	elseif option == 4 then
		form:SetMessage(t("Return here on the completion of the contract and you will be paid."))
	end
	form:AddOption(string.interp(t("Where can I find {target}?"), {target = ad.target}), 1);
	form:AddOption(t("Could you repeat the original request?"), 0);
	form:AddOption(t("How soon must it be done?"), 2);
	form:AddOption(t("How will I be paid?"), 4);
	form:AddOption(t("Ok, agreed."), 3);
	form:AddOption(t('HANG_UP'), -1);
end

local RandomShipRegId = function ()
	local letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	local a = Engine.rand:Integer(1, #letters)
	local b = Engine.rand:Integer(1, #letters)
	return string.format("%s%s-%04d", letters:sub(a,a), letters:sub(b,b), Engine.rand:Integer(0, 9999))
end

local nearbysystems
local makeAdvert = function (station)
	local ass_flavours = Translate:GetFlavours('Assassination')
	if nearbysystems == nil then
		nearbysystems = Game.system:GetNearbySystems(max_ass_dist, function (s) return #s:GetStationPaths() > 0 end)
	end
	if #nearbysystems == 0 then return end
	local client = Character.New()
	local targetIsfemale = Engine.rand:Integer(1) == 1
	local target = t('TITLE')[Engine.rand:Integer(1, #t('TITLE'))] .. " " .. NameGen.FullName(targetIsfemale)
	local flavour = Engine.rand:Integer(1, #ass_flavours)
	local nearbysystem = nearbysystems[Engine.rand:Integer(1,#nearbysystems)]
	local nearbystations = nearbysystem:GetStationPaths()
	local location = nearbystations[Engine.rand:Integer(1,#nearbystations)]
	local time = Engine.rand:Number(0.3, 3)
	local due = Game.time + Engine.rand:Number(7*60*60*24, time * 31*60*60*24)
	local danger = Engine.rand:Integer(1,4)
	local reward = Engine.rand:Number(2100, 7000) * danger
	local shiptypes = ShipType.GetShipTypes('SHIP', function (t)
		return (t.hullMass >= (danger * 17)) and (t:GetEquipSlotCapacity('ATMOSHIELD') > 0) end)
	local shipid = shiptypes[Engine.rand:Integer(1,#shiptypes)]
	local shipname = ShipType.GetShipType(shipid).name

	local ad = {
		client = client,
		danger = danger,
		due = due,
		faceseed = Engine.rand:Integer(),
		flavour = flavour,
		isfemale = isfemale,
		location = location,
		reward = reward,
		shipid = shipid,
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

local planets
local onEnterSystem = function (ship)
	if not ship:IsPlayer() then return end

	local syspath = Game.system.path

	for ref,mission in pairs(missions) do
		if mission.status == 'ACTIVE' then
			if not mission.ship then
				if mission.due > Game.time then
					if mission.location:IsSameSystem(syspath) then -- spawn our target ship
						local station = Space.GetBody(mission.location.bodyIndex)
						local shiptype = ShipType.GetShipType(mission.shipid)
						local default_drive = shiptype.defaultHyperdrive
						local lasers = EquipType.GetEquipTypes('LASER', function (e,et) return et.slot == "LASER" end)
						local count = tonumber(string.sub(default_drive, -1)) ^ 2
						local laser = lasers[mission.danger]

						mission.ship = Space.SpawnShipDocked(mission.shipid, station)
						if mission.ship == nil then
							return -- TODO
						end
						mission.ship:SetLabel(mission.shipregid)
						mission.ship:AddEquip('ATMOSPHERIC_SHIELDING')
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
				if not mission.ship:exists() then
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

local onLeaveSystem = function (ship)
	if ship:IsPlayer() then
		nearbysystems = nil
		planets = nil
	end
end

local onShipDocked = function (ship, station)
	for ref,mission in pairs(missions) do
		if ship:IsPlayer() then
			if mission.status == 'COMPLETED' and
			   mission.backstation == station.path then
				local ass_flavours = Translate:GetFlavours('Assassination')
				local text = string.interp(ass_flavours[mission.flavour].successmsg, {
					target	= mission.target,
					cash	= Format.Money(mission.reward),
				})
				Comms.ImportantMessage(text, mission.boss)
				ship:AddMoney(mission.reward)
				ship:RemoveMission(ref)
				missions[ref] = nil
			elseif mission.status == 'FAILED' then
				local ass_flavours = Translate:GetFlavours('Assassination')
				local text
				if mission.notplayer == 'TRUE' then
					text = string.interp(ass_flavours[mission.flavour].failuremsg2, {
						target	= mission.target,
					})
				else
					text = string.interp(ass_flavours[mission.flavour].failuremsg, {
						target	= mission.target,
					})
				end
				Comms.ImportantMessage(text, mission.boss)
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
			planets = Space.GetBodies(function (body) return body:isa("Planet") end)
			if #planets == 0 then
				ship:AIFlyTo(station)
				mission.shipstate = 'outbound'
			else
				local planet = Engine.rand:Integer(1,#planets)

				mission.ship:AIEnterMediumOrbit(planets[planet])
				mission.shipstate = 'flying'

				table.remove(planets, planet)
			end
			return
		end
	end
end

local onAICompleted = function (ship, ai_error)
	for ref,mission in pairs(missions) do
		if mission.status == 'ACTIVE' and
		   mission.ship == ship then
			if mission.shipstate == 'outbound' then
				local stats = ship:GetStats()
				local systems = Game.system:GetNearbySystems(stats.hyperspaceRange, function (s) return #s:GetStationPaths() > 0 end)
				if #systems == 0 then return end
				local system = systems[Engine.rand:Integer(1,#systems)]

				mission.shipstate = 'inbound'
				ship:HyperspaceTo(system.path)
			-- the only other states are flying and inbound, and there is no AI to complete for inbound
			elseif ai_error == 'NONE' then
				Timer:CallAt(Game.time + 60 * 60 * 8, function ()
					if mission.ship:exists() then
						local stations = Space.GetBodies(function (body) return body:isa("SpaceStation") end)
						if #stations == 0 then return end
						local station = stations[Engine.rand:Integer(1,#stations)]

						mission.ship:AIDockWith(station)
					end
				end)
			else
				if #planets > 0 then
					local planet = Engine.rand:Integer(1,#planets)

					mission.ship:AIEnterMediumOrbit(planets[planet])

					table.remove(planets, planet)
				else
					mission.ship:AIFlyTo(Space.GetBody(mission.location.bodyIndex))
					mission.shipstate = 'outbound'
				end
			end
		end
	end
end

local onUpdateBB = function (station)
	for ref,ad in pairs(ads) do
		if (ad.due < Game.time + 5*60*60*24) then
			ad.station:RemoveAdvert(ref)
		end
	end
	if Engine.rand:Integer(4*24*60*60) < 60*60 then -- roughly once every four days
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

local onGameEnd = function ()
	nearbysystems = nil
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

Event.Register("onCreateBB", onCreateBB)
Event.Register("onGameStart", onGameStart)
Event.Register("onEnterSystem", onEnterSystem)
Event.Register("onLeaveSystem", onLeaveSystem)
Event.Register("onShipDestroyed", onShipDestroyed)
Event.Register("onShipUndocked", onShipUndocked)
Event.Register("onAICompleted", onAICompleted)
Event.Register("onShipDocked", onShipDocked)
Event.Register("onShipHit", onShipHit)
Event.Register("onUpdateBB", onUpdateBB)
Event.Register("onGameEnd", onGameEnd)


Serializer:Register("Assassination", serialize, unserialize)
