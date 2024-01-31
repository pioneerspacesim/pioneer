-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Lang         = require "Lang"
local Engine       = require "Engine"
local Game         = require "Game"
local StarSystem   = require "StarSystem"
local Comms        = require "Comms"
local Event        = require "Event"
local Music        = require "Music"
local Mission      = require "Mission"
local MissionUtils = require "modules.MissionUtils"
local Format       = require "Format"
local Serializer   = require "Serializer"
local Character    = require "Character"
local utils        = require 'utils'

local ui = require 'pigui'

local ScanManager = require '.ScanManager'

local l = Lang.GetResource("module-scout")
local lc = Lang.GetResource("core")
local luc = Lang.GetResource("ui-core")

 -- don't produce missions for further than this many light years away
local max_scout_dist = 30

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
	reward_per_km = 0.08,
	-- reward scaling by resolution
	reward_resolution_max = 1.0,
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
	-- dropoff: data needs to be delivered to another station than where the mission was accepted from
	{                          -- flavour 1
		localscout = false,    -- is in same system?
		days       = 60,       -- days until deadline, from accepting it
		difficulty = 1,        -- altitude, [0,1]
		reward     = 1,        -- reward multiplier, 1=none. (unrelated to "urgency")
		dropoff    = false,
	}, {
		localscout = false,    -- 2 Galactic Geographic Society
		days       = 60,
		difficulty = 2,        -- low altitude flying
		reward     = 1,
		dropoff    = false,
	}, {
		localscout = false,    -- 3 rich pirate hiring
		days       = 30,
		difficulty = 1.5,
		reward     = 1.5,
		dropoff    = true,
	}, {
		localscout = false,    -- 4 Stressed PhD student, short time
		days       = 30,
		difficulty = 1.5,
		reward     = 1,
		dropoff    = false,
	}, {
		localscout = false,    -- 5 Neutral / standard
		days       = 60,
		difficulty = 1,
		reward     = 1,
		dropoff    = false,
	}, {
		localscout = true,     -- 6 local system admin
		days       = 60,
		difficulty = 1,
		reward     = 1.5,      -- government pays well
		dropoff    = false,
	}, {
		localscout = true,     -- 7 family in need of new land
		days       = 25,       -- urgent!
		difficulty = 1,
		reward     = 2,        -- because urgent
		dropoff    = false,
	}, {
		localscout = true,     -- 8 geographical society
		days       = 80,
		difficulty = 2,
		reward     = 2,
		dropoff    = false,
	}, {
		localscout = false,    -- 9 Family in race to a claim
		days       = 30,	   -- should be short
		difficulty = 1,
		reward     = 3,
		dropoff    = true,
	}
}

-- add strings to scout flavours
for i = 1,#flavours do
	local f = flavours[i]
	f.adtitle    = l["ADTITLE_"..i]
	f.adtext     = l["ADTEXT_"..i]
	f.introtext  = l["ADTEXT_"..i.."_INTRO"]
	f.whysomuch	 = l["ADTEXT_"..i.."_WHYSOMUCH"]
	f.successmsg = l["ADTEXT_"..i.."_SUCCESSMSG"]
	f.failmsg	 = l["ADTEXT_"..i.."_FAILMSG"]
end

local ads      = {}
local missions = {}
local missionKey = {}

local format_coverage = function(orbital, val)
	return orbital and string.format("%.2f%%", val * 100) or ui.Format.Distance(val * 1000)
end

local format_resolution = function(val)
	return ui.Format.Distance(val, "%.1f")
end

local format_dist = function(ad)
	local flavour = flavours[ad.flavour]
	return flavour.localscout and ui.Format.Distance(ad.dist) or string.format("%.2f ", ad.dist) .. lc.UNIT_LY
end

local onChat = function (form, ref, option)
	local ad      = ads[ref]
	local station = Game.player:GetDockedWith().path
	form:Clear()
	if option == -1 then
		form:Close()
		return
	end

	form:AddNavButton(ad.location)

	if option == 0 then
		form:SetFace(ad.client)

		local sbody = ad.location:GetSystemBody()     ---@type SystemBody mission body

		local introtext = string.interp(flavours[ad.flavour].introtext, {
			name       = ad.client.name,
			cash       = Format.Money(ad.reward, false),
			systembody = sbody.name,
			system     = ui.Format.SystemPath(ad.location:SystemOnly()),
			dist       = format_dist(ad),
		})
		form:SetMessage(introtext)

	elseif option == 1 then
		form:SetMessage(flavours[ad.flavour].whysomuch)

	elseif option == 2 then
		form:SetMessage(string.interp(l.PLEASE_HAVE_THE_DATA_BACK_BEFORE, {date = Format.Date(ad.due)}))

	elseif option == 4 then
		form:SetMessage(string.interp(l.SCAN_DETAILS, {
			coverage = format_coverage(ad.orbital, ad.coverage),
			resolution = string.format("%.1f", ad.resolution),
			body = ad.location:GetSystemBody().name,
			type = ad.orbital and l.AN_ORBITAL_SCAN or l.A_SURFACE_SCAN
		}))

	elseif option == 5 then
		form:SetMessage(ad.orbital and l.ADDITIONAL_INFORMATION_ORBITAL or l.ADDITIONAL_INFORMATION_SURFACE)

	elseif option == 3 then

		form:RemoveAdvertOnClose()
		ads[ref] = nil
		local mission = {
			type        = "Scout",
			station     = station,
			client      = ad.client,
			location    = ad.location,
			difficulty  = ad.difficulty,
			reward      = ad.reward,
			due         = ad.due,
			flavour     = ad.flavour,
			dropoff     = ad.dropoff,
			coverage    = ad.coverage,
			resolution  = ad.resolution,
			orbital     = ad.orbital,
			status      = 'ACTIVE',
		}

		local scanMgr = Game.player:GetComponent("ScanManager")

		if ad.orbital then
			mission.scanId = scanMgr:AddNewOrbitalScan(ad.location, ad.resolution, ad.coverage)
		else
			mission.scanId = scanMgr:AddNewSurfaceScan(ad.location, ad.resolution, ad.coverage)
		end

		mission = Mission.New(mission)
		table.insert(missions, mission)
		missionKey[mission.scanId] = mission

		form:SetMessage(l.EXCELLENT_I_AWAIT_YOUR_REPORT)
		return
	end

	form:AddOption(l.WHY_SO_MUCH_MONEY, 1)
	form:AddOption(l.WHEN_DO_YOU_NEED_THE_DATA, 2)
	form:AddOption(l.WHAT_DATA_DO_YOU_NEED, 4)
	form:AddOption(l.HOW_DOES_IT_WORK, 5)
	form:AddOption(l.REPEAT_THE_ORIGINAL_REQUEST, 0)
	form:AddOption(l.OK_AGREED, 3)
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

	local resolutionScalar = math.invlerp(p.resolution_min, p.resolution_max, resolution)

	if radiusScalar > 1.0 then
		-- body is small, increase coverage of the scan
		coverage = math.min(coverage, 1.0) * (radiusScalar ^ 0.9)
	else
		-- body is large, reduce coverage of the scan proportionally
		coverage = math.min(coverage, 1.0) * radiusScalar
		-- similarly increase the resolution of the scan to ensure we can scan at higher altitudes
		resolution = resolution * (1.0 + math.log(radiusKm / p.nominal_radius, 10))
	end

	coverage = math.min(coverage * realDifficulty, 1.0)

	local rewardAmount = reward
		* p.reward_per_km * (math.pi * radiusKm * coverage)
		* math.lerp(p.reward_resolution_min, p.reward_resolution_max, resolutionScalar)

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
		* (1 + math.max(math.log(sBody.atmosDensity), 0.0) * 0.5)
		* (1 + sBody.eccentricity * 0.5)

	local bodyReward = 1
		+ sBody.volcanicity * 1.5
		+ sBody.metallicity * 0.4
		+ sBody.atmosOxidizing * 2.3
		+ sBody.volatileIces * 0.8
		+ sBody.volatileLiquid * 1.8

	-- calculate some normalized mean between 1.0 and ~3
	bodyReward = 1 + bodyReward / 5

	local rewardAmount = reward
		* p.reward_per_km * coverage
		* math.lerp(p.reward_resolution_min, p.reward_resolution_max,
			math.invlerp(p.resolution_min, p.resolution_max, resolution))
		+ p.reward_difficulty
		* bodyDifficulty
		+ p.reward_interesting
		* bodyReward

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
		return false
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
		return false
	end

	-- filter out bodies unless at least 300km in diameter
	if sBody.radius < 150000 then
		return false
	end

	-- no missions in the backyard please
	if station:GetSystemBody().parent == sBody then
		return false
	end

	return true
end

local function placeAdvert(station, ad)
	local system = ad.location:GetStarSystem()

	local title = flavours[ad.flavour].adtitle
	local desc = flavours[ad.flavour].adtext % {
		system     = system.name,
		cash       = Format.Money(ad.reward),
		dist       = format_dist(ad),
		systembody = ad.location:GetSystemBody().name
	}

	local icon = ad.orbital and "scout_orbital" or "scout_surface"

	local ref = station:AddAdvert({
		title       = title,
		description = desc,
		icon        = icon,
		location    = ad.location,
		due         = ad.due,
		reward      = ad.reward,
		onChat      = onChat,
		onDelete    = onDelete
	})

	ads[ref] = ad
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

	local orbital = Engine.rand:Number() > 0.6 -- slightly more surface scans than orbital scans
	local filter = orbital and filterBodyOrbital or filterBodySurface

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
	if orbital then
		info = calcOrbitalScanMission(missionBody, difficulty, flavour.reward)
	else
		info = calcSurfaceScanMission(missionBody, difficulty, flavour.reward)
	end

	due = Game.time + days * MissionUtils.Days + MissionCalc:CalcTravelTime(station, location, urgency, 0.1) * 3.5
	reward = info.reward + MissionCalc:CalcReward(station, location, urgency, difficulty - 1.0, 0.2) * 1.6
	local dist = MissionCalc:CalcDistance(station, location)

	reward = utils.round(reward, 5)
	local client = Character.New()

	local ad = {
		station    = station,
		flavour    = flavourN,
		client     = client,
		location   = location,
		dist       = dist,
		due        = due,
		difficulty = difficulty,
		days       = days,
		reward     = reward,
		dropoff    = flavour.dropoff,
		faceseed   = Engine.rand:Integer(),

		orbital    = orbital,
		coverage   = info.coverage,
		resolution = info.minResolution,
	}

	placeAdvert(station, ad)
end

local onCreateBB = function (station)
	local num = Engine.rand:Integer(math.ceil(Game.system.population)) / 3

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

local onScanRangeEnter = function(player, scanId)
	local mission = missionKey[scanId]
	if not mission then
		print("onScanRangeEnter: scan not associated with a Scout mission!", scanId)
		return
	end

	Comms.ImportantMessage(mission.orbital and l.DISTANCE_REACHED_ORBITAL or l.DISTANCE_REACHED_SURFACE, l.COMPUTER)
	-- TODO: this will be muted if the player has music muted - needs a dedicated SFX channel
	-- FIXME: the MusicPlayer immediately overrides this track
	-- Music.FadeIn("music/core/radar-mapping/scanner-in-operation", 0.6, true)
end

local onScanRangeExit = function(player, scanId)
	local mission = missionKey[scanId]
	if not mission then
		print("onScanRangeExit: scan not associated with a Scout mission!", scanId)
		return
	end

	Comms.ImportantMessage(mission.orbital and l.MAPPING_INTERRUPTED_ORBITAL or l.MAPPING_INTERRUPTED_SURFACE, l.COMPUTER)
	-- Music.FadeOut(0.8)
	-- TODO: should play a short notification sound here
end

local onScanPaused = function(player, scanId)
	local mission = missionKey[scanId]
	if not mission then
		print("onScanRangeExit: scan not associated with a Scout mission!", scanId)
		return
	end

	Comms.ImportantMessage(l.MAPPING_PAUSED, l.COMPUTER)
	-- Music.FadeOut(0.8)
	-- TODO: should play a short notification sound here
end

local onScanComplete = function (player, scanId)
	local mission = missionKey[scanId]
	if not mission then
		print("onScanComplete: scan not associated with a Scout mission!", scanId)
		return
	end

	Comms.ImportantMessage(l.MAPPING_COMPLETED, l.COMPUTER)
	-- Music.FadeOut(0.8)
	-- TODO: should play a short notification sound here

	mission.status = "COMPLETED"

	-- decide delivery location:

	-- TODO: local missions with dropoff should select a new station in the same system
	local newlocation = mission.station
	if mission.dropoff and not flavours[mission.flavour].localscout then
		-- XXX-TODO GetNearbyStationPaths triggers bug in Gliese 190 mission. Empty system!
		local nearbystations =
		MissionUtils.GetNearbyStationPaths(Game.system, Engine.rand:Integer(10, 20), nil,
			function(s) return (s.type ~= 'STARPORT_SURFACE') or
				(s.parent.type ~= 'PLANET_ASTEROID')
			end)

		if nearbystations and #nearbystations > 0 then
			newlocation = nearbystations[Engine.rand:Integer(1, #nearbystations)]
			Comms.ImportantMessage(l.YOU_WILL_BE_PAID_ON_MY_BEHALF_AT_NEW_DESTINATION,
				mission.client.name)
		end
	end

	mission.location = newlocation

	if Game.system and mission.location:IsSameSystem(Game.system.path) then
		Game.player:SetNavTarget(mission.location)
	else
		Game.player:SetHyperspaceTarget(mission.location:SystemOnly())
	end
end


---@param player Player
---@param station SpaceStation
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
			missionKey[mission.scanId] = nil
		end

	end
end


local loaded_data

local onGameStart = function ()
	-- If we loaded a saved game, the player may have a ScanManager component already
	if not Game.player:GetComponent("ScanManager") then
		Game.player:SetComponent("ScanManager", ScanManager.New(Game.player))
	end

	ads = {}
	missions = {}
	missionKey = {}

	if loaded_data then
		for k,ad in pairs(loaded_data.ads) do
			placeAdvert(ad.station, ad)
		end

		-- Recreate mission lookup key
		missions = loaded_data.missions
		for ref, mission in pairs(missions) do
			if mission.scanId then missionKey[mission.scanId] = mission end
		end

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
	end
end


local buildMissionDescription = function (mission)
	local desc = {}
	local dist = Game.system and string.format("%.2f", Game.system:DistanceTo(mission.location)) or "???"

	local finished = mission.status == "COMPLETED" or mission.status == "FAILED"

	-- Main body intro text
	if finished then
		desc.description = string.interp(l.DROP_OFF_DATA,
										 {date = Format.Date(mission.due),
										  location = mission.location:GetSystemBody().name})
	else
		desc.description =
			flavours[mission.flavour].introtext:interp(
				{
					name       = mission.client.name,
					systembody = mission.location:GetSystemBody().name,
					system     = ui.Format.SystemPath(mission.location:SystemOnly()),
					dist       = dist,
					cash       = Format.Money(mission.reward),
				})
		desc.location = mission.location
	end
	desc.client = mission.client

	local coordinates = "("..mission.location.sectorX..","
		..mission.location.sectorY..","
		..mission.location.sectorZ..")"

	-- station is shown for return station, after mission is completed
	local destination = not finished and
		{ l.TARGET_BODY,   mission.location:GetSystemBody().name } or
		{ l.DESTINATION,   mission.location:GetSystemBody().name }

	desc.details = {
		"Mapping",
		{lc.SYSTEM..":",  mission.location:GetStarSystem().name.." "..coordinates},
		destination,
		{l.DISTANCE,      dist .. lc.UNIT_LY},
		{l.DEADLINE,      Format.Date(mission.due)},
		{luc.TYPE..":",   mission.orbital and l.ORBITAL_SCAN or l.SURFACE_SCAN},
		{l.COVERAGE,      format_coverage(mission.orbital, mission.coverage) },
		{l.RESOLUTION,    format_resolution(mission.resolution) },
		{luc.STATUS,      luc[mission.status]},
	}

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

Event.Register("onGameStart", onGameStart)
Event.Register("onGameEnd", onGameEnd)
Event.Register("onCreateBB", onCreateBB)
Event.Register("onUpdateBB", onUpdateBB)
Event.Register("onEnterSystem", onEnterSystem)
Event.Register("onShipDocked", onShipDocked)

Event.Register("onScanRangeEnter", onScanRangeEnter)
Event.Register("onScanRangeExit", onScanRangeExit)
Event.Register("onScanPaused", onScanPaused)
Event.Register("onScanComplete", onScanComplete)

Mission.RegisterType('Scout', l.MAPPING, buildMissionDescription)

Serializer:Register("Scout", serialize, unserialize)
