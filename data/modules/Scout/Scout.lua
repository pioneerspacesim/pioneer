-- Copyright © 2008-2022 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Lang       = require "Lang"
local Engine     = require "Engine"
local Game       = require "Game"
local StarSystem = require "StarSystem"
local Space      = require "Space"
local Comms      = require "Comms"
local Event      = require "Event"
local Mission    = require "Mission"
local NameGen    = require "NameGen"
local Format     = require "Format"
local Serializer = require "Serializer"
local Character  = require "Character"
local Timer      = require "Timer"
local Eq         = require "Equipment"
local utils      = require 'utils'

local l = Lang.GetResource("module-scout")
local lc = Lang.GetResource("core")
local luc = Lang.GetResource("ui-core")

-- TODO:
-- * days / flat deadline, always e.g. 3 months (depending on contractor?)
-- * adjust deadline for each flavour
-- * ...thus remove "why so much" line
-- * have propper progress bar for surface scanner
-- * look over each flavour's deadline, rewards and difficulty
-- * round reward utils.round()
-- * use svg icon instead of png, remove png

 -- don't produce missions for further than this many light years away
local max_scout_dist = 30

-- scanning time 600 = 10 minutes
local scan_time = 600

-- CallEvery(xTimeUp,....
local xTimeUp = 10

-- minimum $350 reward in local missions
local local_reward = 350

-- Get the UI class
local ui = Engine.ui


local flavours = {
	-- localscout: if in same system or not
	-- days: simply the hard deadline for this type of contract
	-- difficulty: used to set altitude in scanner
	-- reward: used as multiplier in reward calculation
	{                          -- flavour 1
		localscout = false,    -- is in same system?
		days       = 60,       -- days until deadline, from accepting it
		difficulty = 0,        -- altitude, [0,1]
		reward     = 1,        -- reward multiplier, 1=none. (unrelated to "urgency")
	}, {
		localscout = false,    -- 2 Galactic Geographic Society
		days       = 60,
		difficulty = 2,        -- low altitude flying
		reward     = 1,
	}, {
		localscout = false,    -- 3 rich pirate hiring
		days       = 30,
		difficulty = 1,
		reward     = 1.5,
	}, {
		localscout = false,    -- 4 Stressed PhD student, short time
		days       = 30,
		difficulty = 1,
		reward     = 1,
	}, {
		localscout = false,    -- 5 Neutral / standard
		days       = 60,
		difficulty = 0,
		reward     = 1,
	}, {
		localscout = true,     -- 6 local system admin
		days       = 60,
		difficulty = 0,
		reward     = 1.5,      -- government pays well
	}, {
		localscout = true,     -- 7 family in need of new land
		days       = 25,       -- urgent!
		difficulty = 0,
		reward     = 2,        -- because urgent
	}, {
		localscout = true,     -- 8 geographical society
		days       = 80,
		difficulty = 2,
		reward     = 2,
	}, {
		localscout = false,    -- 9 Family in race to a claim
		days       = 30,	   -- should be short
		difficulty = 0,
		reward     = 3,
	}
}

-- add strings to scout flavours
for i = 1,#flavours do
	local f = flavours[i]
	f.adtext     = l["ADTEXT_"..i]
	f.introtext  = l["ADTEXT_"..i.."_INTRO"]
	f.introtext2 = l["INTROTEXT_COMPLETED_"..i]   -- xxx
	f.whysomuch	 = l["ADTEXT_"..i.."_WHYSOMUCH"]
	f.successmsg = l["ADTEXT_"..i.."_SUCCESSMSG"]
	f.failmsg	 = l["ADTEXT_"..i.."_FAILMSG"]
end

local ads      = {}
local missions = {}

local onChat = function (form, ref, option)
	local ad          = ads[ref]
	local backstation = Game.player:GetDockedWith().path
	form:Clear()
	if option == -1 then
		form:Close()
		return
	end

	if option == 0 then
		form:SetFace(ad.client)

		local sys   = ad.location:GetStarSystem()     -- mission system
		local sbody = ad.location:GetSystemBody()     -- mission body

		local introtext = string.interp(flavours[ad.flavour].introtext, {
			name       = ad.client.name,
			cash       = Format.Money(ad.reward, false),
			systembody = sbody.name,
			system     = sys.name,
			sectorx    = ad.location.sectorX,
			sectory    = ad.location.sectorY,
			sectorz    = ad.location.sectorZ,
			dist       = string.format("%.2f", ad.dist),
		})
		form:SetMessage(introtext)

	elseif option == 1 then
		form:SetMessage(flavours[ad.flavour].whysomuch)

	elseif option == 2 then
		form:SetMessage(string.interp(l.PLEASE_HAVE_THE_DATA_BACK_BEFORE, {date = Format.Date(ad.due)}))

	elseif option == 4 then
			form:SetMessage(l.ADDITIONAL_INFORMATION)

	elseif option == 3 then

		-- use something else!
		-- if Game.player:CountEquip(Eq.misc.radar_mapper) == 0 then
		--	form:SetMessage(l.YOU_NEED_RADAR_MAPPER)
		--	return
		-- end
		form:RemoveAdvertOnClose()
		ads[ref] = nil
		local mission = {
			type        = "Scout",
			backstation = backstation,
			client      = ad.client,
			location    = ad.location,
			difficulty  = ad.difficulty,
			reward      = ad.reward,
			due         = ad.due,
			flavour     = ad.flavour,
			status      = 'ACTIVE',
		}

		table.insert(missions,Mission.New(mission))
		Game.player:SetHyperspaceTarget(mission.location:GetStarSystem().path)
		form:SetMessage(l.EXCELLENT_I_AWAIT_YOUR_REPORT)
		return
	end

	form:AddOption(l.WHY_SO_MUCH_MONEY, 1)
	form:AddOption(l.WHEN_DO_YOU_NEED_THE_DATA, 2)
	form:AddOption(l.HOW_DOES_IT_WORK, 4)             --- foo xxx
	form:AddOption(l.REPEAT_THE_ORIGINAL_REQUEST, 0)
	form:AddOption(l.ACCEPT_AND_SET_SYSTEM_TARGET, 3)
end


local onDelete = function (ref)
	ads[ref] = nil
end

-- store once for the system player is in
local nearbysystems

local makeAdvert = function (station)
	local reward, due, nearbysystem
	local location						  -- mission body
	local client = Character.New()
	local flavour = Engine.rand:Integer(1,#flavours)
	local days = flavours[flavour].days
	local difficulty = flavours[flavour].difficulty

	if flavours[flavour].localscout then  -- local system

		nearbysystem = Game.system        -- i.e. this system
		local nearbybodies = nearbysystem:GetBodyPaths()
		local checkedBodies = 0
		while checkedBodies <= #nearbybodies do -- check, at most, all nearbybodies
			location = nearbybodies[Engine.rand:Integer(1,#nearbybodies)]
			local currentBody = location:GetSystemBody()   -- go from syspath to sysbody
			if currentBody.superType == "ROCKY_PLANET" and currentBody.type ~= "PLANET_ASTEROID" then
				break  --- xxx why SUPER type? only allow satellites?
			end
			checkedBodies = checkedBodies + 1
		end

		-- no missions in the backyard please
		local dist = station:DistanceTo(Space.GetBody(location.bodyIndex))
		if dist < 1000 then return end

		reward = local_reward + (math.sqrt(dist) / 15000)
		due = Game.time + (86400 * days)
		due = 1e8 --- just for testing
	else                                   -- remote system
		if nearbysystems == nil then       -- only uninhabited systems
			nearbysystems =	Game.system:GetNearbySystems(max_scout_dist,
				function (s) return #s:GetBodyPaths() > 0 and s.population == 0 end)
		end
		if #nearbysystems == 0 then return end
		nearbysystem = nearbysystems[Engine.rand:Integer(1,#nearbysystems)]
		local dist = nearbysystem:DistanceTo(Game.system)
		local nearbybodies = nearbysystem:GetBodyPaths()

		local checkedBodies = 0
		while checkedBodies <= #nearbybodies do -- check, at most, all nearbybodies
			location = nearbybodies[Engine.rand:Integer(1,#nearbybodies)]
			local currentBody = location:GetSystemBody()
			if currentBody.superType == "ROCKY_PLANET" and currentBody.type ~= "PLANET_ASTEROID" then
				break
			end
			checkedBodies = checkedBodies + 1
		end

		-- Compute reward for mission
		local multiplier = Engine.rand:Number(1.5,1.6)
		reward = 100*multiplier  -- todo xxx
		due = Game.time + (86400 * days)
		due = 1e8 --- just for testing
	end

	local ad = {
		station    = station,
		flavour    = flavour,
		client     = client,
		location   = location,
		dist       = Game.system:DistanceTo(location),
		due        = due,
		difficulty = difficulty,
		days    = days,
		reward     = reward,
		isfemale   = isfemale,
		faceseed   = Engine.rand:Integer(),
	}

	ad.desc = string.interp(flavours[flavour].adtext, {
		system     = nearbysystem.name,
		cash       = Format.Money(ad.reward),
		dist       = string.format("%.2f", ad.dist),
		systembody = ad.location:GetSystemBody().name
	})

	local ref = station:AddAdvert({
		description = ad.desc,
		icon        = "scout",
		onChat      = onChat,
		onDelete    = onDelete})

	ads[ref] = ad
end


local onCreateBB = function (station)
	local num = Engine.rand:Integer(math.ceil(Game.system.population)) / 2
	num = 10 -- xxx force creation of adverts, for testing
	for i = 1,num do
		makeAdvert(station)
	end
end


local onUpdateBB = function (station)
	for ref,ad in pairs(ads) do
		if not flavours[ad.flavour].localscout
			and ad.due < Game.time + 432000 then -- 5 days
			ad.station:RemoveAdvert(ref)
		elseif flavours[ad.flavour].localscout
			and ad.due < Game.time + 172800 then -- 2 days
			ad.station:RemoveAdvert(ref)
		end
	end
	if Engine.rand:Integer(43200) < 3600 then    -- 12 h < 1 h
		makeAdvert(station)
	end
end


local onEnterSystem = function (playership)
	if not playership:IsPlayer() then return end
	nearbysystems = nil

	for ref,mission in pairs(missions) do
		if mission.status == "ACTIVE" and Game.time > mission.due then
			mission.status = 'FAILED'
		end
	end
end


local mapped = function(body)
	local CurBody = Game.player.frameBody or body
	if not CurBody then return end
	local mission, radius_min, radius_max

	for ref,mission in pairs(missions) do
		if Game.time > mission.due then mission.status = "FAILED" end
		if Game.system == mission.location:GetStarSystem() then

			if mission.status == "COMPLETED" then return end -- should this not be continue?

			local PhysBody = CurBody.path:GetSystemBody()
			if PhysBody and CurBody.path == mission.location then
				print("CORRECT BODY")
				local TimeUp = 0
				if mission.difficulty == 2 then
					radius_min = 1.1
					radius_max = 1.2
				elseif mission.difficulty == 1 then
					radius_min = 1.3
					radius_max = 1.4
				else
					radius_min = 1.5
					radius_max = 1.6
				end

				Timer:CallEvery(xTimeUp, function ()
					if not CurBody or not CurBody:exists() or mission.status == "COMPLETED" then return 1 end
					local Dist = CurBody:DistanceTo(Game.player)
					if Dist < PhysBody.radius * radius_min
					and (mission.status == 'ACTIVE' or mission.status == "SUSPENDED") then
						print("DIST1:", PhysBody.radius * radius_min)
						local lapse = scan_time / 60
						Comms.ImportantMessage(l.Distance_reached .. lapse .. l.minutes, l.COMPUTER)
						print(l.Distance_reached .. lapse .. l.minutes, l.COMPUTER)
						-- Music.Play("music/core/radar-mapping/mapping-on")
						mission.status = "MAPPING"
					elseif Dist > PhysBody.radius * radius_max and mission.status == "MAPPING" then
						-- Music.Play("music/core/radar-mapping/mapping-off",false)
						print("DIST1:", PhysBody.radius * radius_max)
						Comms.ImportantMessage(l.MAPPING_INTERRUPTED, l.COMPUTER)
						print(l.MAPPING_INTERRUPTED)
						mission.status = "SUSPENDED"
						TimeUp = 0
						return 1
					end
					if mission.status == "MAPPING" then
						TimeUp = TimeUp + xTimeUp
						if TimeUp >= scan_time then
							mission.status = "COMPLETED"
							-- Music.Play("music/core/radar-mapping/mapping-off",false)
							Comms.ImportantMessage(l.MAPPING_COMPLETED, l.COMPUTER)

							-- decide delivery location:

							--- uuu if we're changing location, that's silly -> remove
							--- or is it a "hack" that a delivery location might not be there?
							--- I know I can always return to the same station, as where I picked up the mission, right.
							local newlocation = mission.backstation
							if not flavours[mission.flavour].localscout then
								-- XXX-TODO GetNearbyStationPaths triggers bug in Gliese 190 mission. Empty system!
								local nearbystations =
									StarSystem:GetNearbyStationPaths(Engine.rand:Integer(10,20), nil, function (s) return
										(s.type ~= 'STARPORT_SURFACE') or (s.parent.type ~= 'PLANET_ASTEROID') end)
								if nearbystations and #nearbystations > 0 then
									newlocation = nearbystations[Engine.rand:Integer(1,#nearbystations)]
									Comms.ImportantMessage(l.YOU_WILL_BE_PAID_ON_MY_BEHALF_AT_NEW_DESTINATION,
												mission.client.name)
								end
							end
							mission.location = newlocation
							Game.player:SetHyperspaceTarget(mission.location:GetStarSystem().path)
						end
					end
					if mission.status == "COMPLETED" then return 1 end
				end)
			end
		end
	end
end


-- Is this called if the mission is to current frame? xxx
local onFrameChanged = function (body)
	if not body:isa("Ship") or not body:IsPlayer() then return end
	if body.frameBody == nil then return end
	local target = Game.player:GetNavTarget()
	if target == nil then return end
	local closestPlanet = Game.player:FindNearestTo("PLANET")
	if closestPlanet ~= target then return end
	local dist
	dist = Format.Distance(Game.player:DistanceTo(target))
	mapped(body)
end


local onShipDocked = function (player, station)
	if not player:IsPlayer() then return end

	local mission
	for ref, mission in pairs(missions) do

		if station.path == mission.location then
			if Game.time > mission.due then
				Comms.ImportantMessage((flavours[mission.flavour].failmesg), mission.client.name)
				Character.persistent.player.reputation = Character.persistent.player.reputation - 1
			else
				Comms.ImportantMessage((flavours[mission.flavour].successmsg), mission.client.name)
				Character.persistent.player.reputation = Character.persistent.player.reputation + 1
				player:AddMoney(mission.reward)
			end
			mission:Remove()
			missions[ref] = nil
		elseif mission.status == "ACTIVE" and Game.time > mission.due then -- or not COPMLEATED? is ACTIVE a state?
			mission.status = "FAILED"
		end
	end
end


local loaded_data

local onGameStart = function ()
	ads = {}
	missions = {}

	if loaded_data then
		for k,ad in pairs(loaded_data.ads) do
			ads[ad.station:AddAdvert({
				description = ad.desc,
				icon        = "scout",
				onChat      = onChat,
				onDelete    = onDelete})] = ad
		end
		missions = loaded_data.missions
		loaded_data = nil
	end

	local currentBody = Game.player.frameBody
	local mission
	for ref,mission in pairs(missions) do
		if currentBody and currentBody.path ~= mission.location then return end
		if Game.time > mission.due then
			mission.status = "FAILED"
			mission:Remove()
			missions[ref] = nil
			return
		end
		mapped(currentBody)
	end
end


local buildMissionDescription = function (mission)
	local ui = require 'pigui'
	local desc = {}
	local dist = Game.system and string.format("%.2f", Game.system:DistanceTo(mission.location)) or "???"

	-- Map to altitude, or 'low' / 'medium' / 'high'
	local danger = mission.difficulty

	-- Main body intro text (should be different if completed mission?)
	desc.description =
		flavours[mission.flavour].introtext:interp(
			{
				name       = mission.client.name,
				systembody = mission.location:GetSystemBody().name,
				system     = mission.location:GetStarSystem().name,
				sectorx    = mission.location.sectorX,
				sectory    = mission.location.sectorY,
				sectorz    = mission.location.sectorZ,
				dist       = dist,
				cash       = Format.Money(mission.reward),
			})

	desc.location = mission.location
	desc.client = mission.client

	local coordinates = "("..mission.location.sectorX..","
		..mission.location.sectorY..","
		..mission.location.sectorZ..")"

	-- TODO: don't show danger, instead, show min/max altitude for scanning
	-- station is shown for return station, after mission is completed
	-- mission status should be translated

	--if mission.status == "ACTIVE" or mission.status == "MAPPING" then
	desc.details = {
		"Mapping",
		{lc.SYSTEM,       mission.location:GetStarSystem().name.." "..coordinates},
		{luc.STATION,     mission.location:GetSystemBody().name},
		-- {l.OBJECTIVE,     mission.location:GetSystemBody().name},
		{l.DISTANCE,      dist .. lc.UNIT_LY},
		{l.DEADLINE,      Format.Date(mission.due)},
		{l.DANGER,        danger},
		{luc.STATUS,      mission.status},
		--{lc.STATUS,       l[mission.status]},
	}

	-- elseif mission.status =="COMPLETED" then
	-- elseif mission.status =="SUSPENDED" then
	-- elseif mission.status =="FAILED" then
	--	return "ERROR"
	return desc
end


local serialize = function ()
	return { ads = ads, missions = missions }
end


local unserialize = function (data)
	loaded_data = data
end

local onGameEnd = function ()
	nearbysystems = nil
end

Event.Register("onGameEnd", onGameEnd)
Event.Register("onCreateBB", onCreateBB)
Event.Register("onUpdateBB", onUpdateBB)
--Event.Register("onLeaveSystem", onLeaveSystem)
Event.Register("onEnterSystem", onEnterSystem)
Event.Register("onFrameChanged", onFrameChanged)
Event.Register("onShipDocked", onShipDocked)
Event.Register("onGameStart", onGameStart)

Mission.RegisterType('Scout', l.MAPPING, buildMissionDescription)

Serializer:Register("Scout", serialize, unserialize)
