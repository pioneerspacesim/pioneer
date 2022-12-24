-- Copyright © 2008-2022 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Lang         = require "Lang"
local Engine       = require "Engine"
local Game         = require "Game"
local StarSystem   = require "StarSystem"
local Space        = require "Space"
local Comms        = require "Comms"
local Event        = require "Event"
local Mission      = require "Mission"
local MissionUtils = require "modules.MissionUtils"
local NameGen      = require "NameGen"
local Format       = require "Format"
local Serializer   = require "Serializer"
local Character    = require "Character"
local Timer        = require "Timer"
local Eq           = require "Equipment"
local utils        = require 'utils'

local ScanManager = require '.ScanManager'

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

-- Duration expected to de-orbit and carry out a typical surface scan
local surface_duration = 2 * MissionUtils.Days

-- minimum $350 reward in local missions
local local_reward = 350
-- minimum $800 reward in interstellar missions
local hyperspace_reward = 800
-- percent of mission reward returned if you're late beyond the deadline
local mission_failed_reward = 0.2

local EARTH_G = 9.8106

-- NOTE: these constants will need to be tweaked when/if scanner equipment capabilities are tweaked

local orbital_params = {
	-- scan resolution, post-modified by difficulty
	resolution_max = 125,
	resolution_min = 35,

	-- scan coverage values generated, relative to the nominal radius
	coverage_max = 0.8,
	coverage_min = 0.1,

	-- approximate "normal" body radius used to scale coverage to the body being scanned
	nominal_radius = 2500,

	-- reward per kilometer-width of body coverage
	reward_per_km = 0.05,
	-- reward scaling by resolution
	reward_resolution_max = 0.5,
	reward_resolution_min = 8.0,
}

local surface_params = {
	-- scan resolution, post-modified by difficulty
	resolution_max = 3.5,
	resolution_min = 0.8,
	-- scan coverage
	coverage_max = 500,
	coverage_min = 50,

	-- reward per km of coverage
	reward_per_km = 0.6,
	-- reward scalar by required resolution
	reward_resolution_max = 0.5,
	reward_resolution_min = 6.0,

	-- base reward for difficulty of deorbiting to the surface
	reward_difficulty = 300,
	-- base reward for interesting nature of the body (ice, liquids, atmosphere composition, volcanicity etc)
	reward_interesting = 500,
}

-- reward scaling by distance from this system
local scan_reward_distance_min = 1.0
local scan_reward_distance_max = 1.6

local MissionCalc = MissionUtils.Calculator.New()

MissionCalc:SetParams({
	baseDuration = 0,
	baseReward = 0,

	inSystemTime = 2 * MissionUtils.Days,
	travelTimeAU = 3 * MissionUtils.Days,
	inSystemReward = local_reward,

	hyperspaceTime = 60 * MissionUtils.Days,
	hyperspaceDist = max_scout_dist,
	hyperspaceReward = hyperspace_reward
})

local flavours = {
	-- localscout: if in same system or not
	-- days: simply the hard deadline for this type of contract
	-- difficulty: used to set altitude in scanner
	-- reward: used as multiplier in reward calculation
	{                          -- flavour 1
		localscout = false,    -- is in same system?
		days       = 60,       -- days until deadline, from accepting it
		difficulty = 1,        -- altitude, [0,1]
		reward     = 1,        -- reward multiplier, 1=none. (unrelated to "urgency")
	}, {
		localscout = false,    -- 2 Galactic Geographic Society
		days       = 60,
		difficulty = 2,        -- low altitude flying
		reward     = 1,
	}, {
		localscout = false,    -- 3 rich pirate hiring
		days       = 30,
		difficulty = 1.5,
		reward     = 1.5,
	}, {
		localscout = false,    -- 4 Stressed PhD student, short time
		days       = 30,
		difficulty = 1.5,
		reward     = 1,
	}, {
		localscout = false,    -- 5 Neutral / standard
		days       = 60,
		difficulty = 1,
		reward     = 1,
	}, {
		localscout = true,     -- 6 local system admin
		days       = 60,
		difficulty = 1,
		reward     = 1.5,      -- government pays well
	}, {
		localscout = true,     -- 7 family in need of new land
		days       = 25,       -- urgent!
		difficulty = 1,
		reward     = 2,        -- because urgent
	}, {
		localscout = true,     -- 8 geographical society
		days       = 80,
		difficulty = 2,
		reward     = 2,
	}, {
		localscout = false,    -- 9 Family in race to a claim
		days       = 30,	   -- should be short
		difficulty = 1,
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

		local scanMgr = Game.player:GetComponent("ScanManager")

		if ad.orbital then
			mission.scanId = scanMgr:AddNewOrbitalScan(ad.location, ad.resolution, ad.coverage)
		else
			mission.scanId = scanMgr:AddNewSurfaceScan(ad.location, ad.resolution, ad.coverage)
		end

		table.insert(missions, Mission.New(mission))
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

-- Generate coverage, resolution, and reward values for an orbital scan of the passed body.
---@param sBody SystemBody body to generate a scan parameter for
---@param difficulty number scalar to multiply the mission requirements by
---@param reward number scalar to multiply the mission reward by
local function calcOrbitalScanMission(sBody, difficulty, reward)
	local realDifficulty = Engine.rand:Number(math.min(difficulty, 1.0), math.max(difficulty, 1.0))

	local p = orbital_params

	local coverage = Engine.rand:Number(p.coverage_min, p.coverage_max)
	local resolution = Engine.rand:Number(p.resolution_min, p.resolution_max) / realDifficulty

	local radiusKm = sBody.radius / 1000
	local radiusScalar = p.nominal_radius / radiusKm

	if radiusScalar > 1.0 then
		-- body is small, increase coverage of the scan
		coverage = math.min(coverage, 1.0) * (radiusScalar ^ 0.9)
	else
		-- body is large, reduce coverage of the scan proportionally
		coverage = math.min(coverage, 1.0) * radiusScalar
		-- similarly increase the resolution of the scan to ensure we can scan at higher altitudes
		resolution = resolution * (1.0 + math.log(radiusKm / p.nominal_radius, 10) * 0.5)
	end

	coverage = math.min(coverage * realDifficulty, 1.0)

	local rewardAmount = reward
		* p.reward_per_km * (math.pi * radiusKm * coverage)
		* math.lerp(p.reward_resolution_min, p.reward_resolution_max,
			math.invlerp(p.resolution_min, p.resolution_max, resolution))

	return {
		coverage = coverage,
		minResolution = resolution,
		reward = rewardAmount
	}
end

-- Generate coverage, resolution, and reward values for a surface scan of the passed body.
---@param sBody SystemBody body to generate a scan parameter for
---@param difficulty number scalar to multiply the mission requirements by
---@param reward number scalar to multiply the mission reward by
local function calcSurfaceScanMission(sBody, difficulty, reward)
	local realDifficulty = Engine.rand:Number(math.min(difficulty, 1.0), math.max(difficulty, 1.0))

	local p = surface_params

	local coverage = Engine.rand:Number(p.coverage_min, p.coverage_max)
	local resolution = Engine.rand:Number(p.resolution_min, p.resolution_max) / realDifficulty

	coverage = math.max(coverage * realDifficulty, 10.0)
	resolution = math.max(resolution, 0.5)

	-- Calculate parameters which make approaching the body to scan it more difficult
	local bodyDifficulty = (1 + sBody.gravity / EARTH_G)
		* (1 + math.max(math.log(sBody.volatileGas), 0.0) * 0.5)
		* (1 + sBody.eccentricity * 0.5)

	local bodyReward = 1
		+ sBody.volcanicity * 1.5
		+ sBody.metallicity * 0.4
		+ sBody.atmosOxidizing * 2.3
		+ sBody.volatileIces * 0.8
		+ sBody.volatileLiquid * 1.8

	-- calculate some normalized mean between 1.0 and ~3
	bodyReward = 1 + bodyReward / 5

	-- print("{name} | g: {gravity} v: {volatileGas} e: {eccentricity}" % sBody)
	-- print("difficulty: {}, reward: {}, coverage: {}, resolution: {}" % { bodyDifficulty, bodyReward, p.reward_per_km * coverage, resolution })

	local rewardAmount = reward
		* p.reward_per_km * coverage
		* math.lerp(p.reward_resolution_min, p.reward_resolution_max,
			math.invlerp(p.resolution_min, p.resolution_max, resolution))
		+ p.reward_difficulty
		* bodyDifficulty
		+ p.reward_interesting
		* bodyReward

	-- print(rewardAmount)

	return {
		coverage = coverage,
		minResolution = resolution,
		reward = rewardAmount,
	}
end

-- store once for the system player is in
local nearbysystems

-- Filter a body for surface scan suitability
---@param station SpaceStation
---@param sBody SystemBody
local filterBodySurface = function(station, sBody)
	-- Allow only planets
	if sBody.superType ~= "ROCKY_PLANET" or sBody.type == "PLANET_ASTEROID" then
		return false --- xxx why SUPER type? only allow satellites?
	end

	-- filter out bodies unless at least 100km in diameter
	if sBody.radius < 50000 then
		return false
	end

	-- no missions in the backyard please
	if station:GetSystemBody().parent == sBody then
		return false
	end

	return true
end

-- Filter a body for orbital scan suitability
---@param sBody SystemBody
local filterBodyOrbital = function(station, sBody)
	-- Allow scanning planets and gas giants
	if (sBody.superType ~= "ROCKY_PLANET" and sBody.superType ~= "GAS_GIANT") or sBody.type == "PLANET_ASTEROID" then
		return false --- xxx why SUPER type? only allow satellites?
	end

	-- filter out bodies unless at least 300km in diameter
	if sBody.radius < 150000 then
		return false
	end

	-- no missions in the backyard please
	local body = sBody.body
	if body and station.frameBody == body then
		return false
	end

	return true
end

---@param station SpaceStation
local makeAdvert = function (station)
	local reward, due, nearbysystem
	local location ---@type SystemPath mission location
	local missionBody ---@type SystemBody mission body

	local flavourN = Engine.rand:Integer(1,#flavours)
	local flavour = flavours[flavourN]
	local days = Engine.rand:Number(flavour.days * 0.5, flavour.days)
	local urgency = 1.0 - math.clamp((days - 30) / 50, 0.0, 1.0)
	local difficulty = flavour.difficulty
	local filter = flavour.orbital and filterBodyOrbital or filterBodySurface

	if flavour.localscout then  -- local system
		nearbysystem = assert(Game.system)        -- i.e. this system
	else                                   -- remote system
		if nearbysystems == nil then       -- only uninhabited systems with at least two bodies besides the sun
			nearbysystems =	Game.system:GetNearbySystems(max_scout_dist,
				function (s) return s.numberOfBodies > 2 and s.population == 0 end)
		end

		if #nearbysystems == 0 then return end
		nearbysystem = nearbysystems[Engine.rand:Integer(1,#nearbysystems)]
	end

	local nearbybodies = nearbysystem:GetBodyPaths()
	local numBodies = #nearbybodies

	for i = 1, numBodies do -- check, at most, all nearbybodies
		location = nearbybodies[Engine.rand:Integer(1, numBodies)]
		local sBody = location:GetSystemBody() -- go from syspath to sysbody

		if filter(station, sBody) then
			missionBody = sBody
			break
		end
	end

	if not missionBody then
		return
	end

	local info
	if flavour.orbital then
		info = calcOrbitalScanMission(missionBody, difficulty, flavour.reward)
	else
		info = calcSurfaceScanMission(missionBody, difficulty, flavour.reward)
	end

	due = Game.time + days * MissionUtils.Days + MissionCalc:CalcTravelTime(station, location, urgency, 0.1) * 3.5
	reward = info.reward + MissionCalc:CalcReward(station, location, urgency, difficulty - 1.0, 0.2) * 1.6

	reward = utils.round(reward, 5)
	local client = Character.New()

	local ad = {
		station    = station,
		flavour    = flavourN,
		client     = client,
		location   = location,
		dist       = Game.system:DistanceTo(location),
		due        = due,
		difficulty = difficulty,
		days       = days,
		reward     = reward,
		faceseed   = Engine.rand:Integer(),

		orbital    = flavour.orbital,
		coverage   = info.coverage,
		resolution = info.minResolution,
	}

	ad.desc = string.interp(flavour.adtext, {
		system     = nearbysystem.name,
		cash       = Format.Money(ad.reward),
		dist       = string.format("%.2f", ad.dist),
		systembody = ad.location:GetSystemBody().name
	})

	local ref = station:AddAdvert({
		title = "{difficulty} | {coverage} | {resolution}" % ad,
		description = ad.desc,
		icon        = "scout",
		due         = ad.due,
		reward      = ad.reward,
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

local onScanComplete = function (player, scanId)
	for ref, mission in pairs(missions) do
		if mission.scanId == scanId then
			mission.status = "COMPLETED"
		end
	end
end


---@param player Player
local onShipDocked = function (player, station)
	if not player:IsPlayer() then return end
	local scanMgr = player:GetComponent("ScanManager")

	for ref, mission in pairs(missions) do

		if mission.status == "ACTIVE" and Game.time > mission.due then
			mission.status = "FAILED"
		end

		if station.path == mission.location and mission.status == "FAILED" or mission.status == "COMPLETED" then
			local flavour = flavours[mission.flavour]
			local failed = mission.status == "FAILED"
			local scan = scanMgr:AcceptScanComplete(mission.scanId)

			if not scan then -- still incomplete, just delete it
				scanMgr:CancelScan(mission.scanId)
			end

			if failed and scan and Game.time < mission.due + (MissionUtils.Days * flavour.days * 0.3) then
				-- You get some of the reward if you're back late with the data
				Comms.ImportantMessage((flavour.failmsg), mission.client.name)
				Character.persistent.player.reputation = Character.persistent.player.reputation - 1
				player:AddMoney(mission.reward * mission_failed_reward)

			elseif failed then
				-- You get no money if you're back without data or entirely too late
				Comms.ImportantMessage((flavour.failmsg), mission.client.name)
				Character.persistent.player.reputation = Character.persistent.player.reputation - 1

			else
				-- You get all of the money and reputation if you're back in time with the data!
				Comms.ImportantMessage((flavour.successmsg), mission.client.name)
				Character.persistent.player.reputation = Character.persistent.player.reputation + 1
				player:AddMoney(mission.reward)
			end

			mission:Remove()
			missions[ref] = nil
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
		-- mapped(currentBody)
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
-- Event.Register("onFrameChanged", onFrameChanged)
Event.Register("onShipDocked", onShipDocked)
Event.Register("onGameStart", onGameStart)

Event.Register("onScanComplete", onScanComplete)

-- Ensure the player ship has a ScanManager available
Event.Register('onGameStart', function()
	Game.player:SetComponent("ScanManager", ScanManager.New(Game.player))
end)

Mission.RegisterType('Scout', l.MAPPING, buildMissionDescription)

Serializer:Register("Scout", serialize, unserialize)
