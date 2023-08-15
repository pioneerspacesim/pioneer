-- Copyright © 2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Lang = require 'Lang'
local Event = require 'Event'
local Serializer = require 'Serializer'
local Engine = require 'Engine'
local Game = require 'Game'
local Character = require 'Character'
local Format = require 'Format'
local utils = require 'utils'
local Rand = require 'Rand'

local rand = Rand.New()

-- This module allows the player to hire crew members through BB adverts
-- on stations

local l = Lang.GetResource("module-crewcontracts")
local lui = Lang.GetResource("ui-core")

local wage_period = 604800 -- a week of seconds

-- The contract for a crew member is a table containing their weekly wage,
-- the date that they should next be paid and the amount outstanding if
-- the player has been unable to pay them.
--
-- contract = {
--   wage = 0,
--   payday = 0,
--   outstanding = 0,
-- }

-- New Character attribute: affinity for civilization
-- This determines how much the character enjoys being in busy systems with high
-- population vs. the unexplored frontier
local civaffinity = {"low", "medium", "high"}

local nonPersistentCharactersForCrew = {}
local stationsWithAdverts = {}

local wageFromScore = function(score)
	-- Default score is four 15s (=60). Anybody with that or less
	-- gets minimum wage offered.
	score = math.max(0,score-60)
	return math.floor(score * score / 100) + 10
end

local checkOffer = function(offer)
	-- Force wage offers to be in correct range
	return math.max(1,offer)
end

local crewInThisStation -- Table of available folk available for hire here
local candidate -- Run-time "static" variable for onChat
local offer

local onChat = function (form,ref,option)

	local station = stationsWithAdverts[ref]

	if option == -1 then
		-- Hang up
		candidate = nil
		form:Close()
		return
	end

	if option == 0 then
		-- Not currently candidate; option values indicate crewInThisStation index
		candidate = nil
		-- Re-initialise crew list, and build it fresh
		crewInThisStation = {}

		-- Add any non-persistent characters (which are persistent only in the sense
		-- that this BB ad is storing them)
		for k,c in ipairs(nonPersistentCharactersForCrew[station]) do
			table.insert(crewInThisStation,c)
			c.experience = c.engineering
							+c.piloting
							+c.navigation
							+c.sensors
			-- Base wage on experience
			c.estimatedWage = c.estimatedWage or wageFromScore(c.experience)
			c.estimatedWage = utils.round(c.estimatedWage, 1)
			-- pick affinity for civilization
			c.civaffinity = civaffinity[rand:Integer(1, 3)]
		end

		-- Now look for any persistent characters that are available in this station
		-- and have some sort of crew experience (however minor)
		-- and were last seen less than a month ago
		for c in Character.Find(function (c)
									return true
										and c.lastSavedSystemPath == station.path
										and c.available
										and (
											c.engineering > 16
											or c.piloting > 16
											or c.navigation > 16
											or c.sensors > 16
										)
										and Game.time - c.lastSavedTime < 2419200 -- (28 days)
								end) do
			table.insert(crewInThisStation,c)
			c.experience = c.engineering
											+c.piloting
											+c.navigation
											+c.sensors
			-- Either base wage on experience, or as a slight increase on their previous wage
			-- (which should only happen if this candidate was dismissed with wages owing)
			c.estimatedWage = math.max(c.contract and (c.contract.wage + 5) or 0, c.estimatedWage or wageFromScore(c.experience))
			c.estimatedWage = utils.round(c.estimatedWage, 1)
			-- pick affinity for civilization if it doesn't exist
			if not c.civaffinity then
				c.civaffinity = civaffinity[rand:Integer(1, 3)]
			end
		end

		form:ClearFace()
		form:Clear()
		form:SetTitle(l.CREW_FOR_HIRE)
		local numCrewInThisStation = 0
		for k,c in ipairs(crewInThisStation) do
			numCrewInThisStation = numCrewInThisStation + 1
		end
		if numCrewInThisStation > 0 then
			form:SetMessage("\n"..l.POTENTIAL_CREW_MEMBERS:interp({station=station.label}))
			for k,c in ipairs(crewInThisStation) do
				form:AddOption(l.CREWMEMBER_WAGE_PER_WEEK:interp({potentialCrewMember = c.name,wage = Format.Money(c.estimatedWage)}),k)
			end
		else
			form:SetMessage("\n"..l.NO_CREW_AVAILABLE:interp({station=station.label}))
		end
	end

	local showCandidateDetails = function (response)
		form:SetFace(candidate)
		form:Clear()
		form:SetTitle(candidate.name)

		-- if playerRelationship is terrible then don't even interact with player
		if candidate.playerRelationship < 15 then
			form:SetMessage(l.I_WOULD_NEVER_CONSIDER_WORKING_FOR_YOU)
			form:AddOption(l.GO_BACK, 0)
		else
			local experience =
				candidate.experience > 160 and l.VETERAN_TIME_SERVED_CREW_MEMBER or
				candidate.experience > 140 and l.TIME_SERVED_CREW_MEMBER or
				candidate.experience > 100 and l.MINIMAL_TIME_SERVED_ABOARD_SHIP or
				candidate.experience >  60 and l.SOME_EXPERIENCE_IN_CONTROLLED_ENVIRONMENTS or
				candidate.experience >  10 and l.SIMULATOR_TRAINING_ONLY or
				l.NO_EXPERIENCE
			candidate:PrintStats()
			print("Attitude: ",candidate.playerRelationship)
			print("Aspiration: ",candidate.estimatedWage)
			if response == "" then response = "\r" end
			form:SetMessage(l.CREWDETAILSHEETBB:interp({
									name = candidate.name,
									experience = experience,
									wage = Format.Money(offer),
									response = response,
			}))
			form:AddOption(l.MAKE_OFFER_OF_POSITION_ON_SHIP_FOR_STATED_AMOUNT,1)
			form:AddOption(l.SUGGEST_NEW_WEEKLY_WAGE_OF_N:interp({newAmount=Format.Money(checkOffer(offer+10))}),2)
			form:AddOption(l.SUGGEST_NEW_WEEKLY_WAGE_OF_N:interp({newAmount=Format.Money(checkOffer(offer+5))}),3)
			form:AddOption(l.SUGGEST_NEW_WEEKLY_WAGE_OF_N:interp({newAmount=Format.Money(checkOffer(offer-5))}),4)
			form:AddOption(l.SUGGEST_NEW_WEEKLY_WAGE_OF_N:interp({newAmount=Format.Money(checkOffer(offer-10))}),5)
			form:AddOption(l.ASK_CANDIDATE_FOR_INTERVIEW_AND_TEST,6)
			form:AddOption(l.GO_BACK, 0)
		end
	end

	if option > 0 then

		if not candidate then
			-- Absence of candidate indicates that option is to select a candidate,
			-- and that is all.
			candidate = crewInThisStation[option]
			offer = candidate.estimatedWage
			showCandidateDetails('')
			return
		end

		if option == 1 then
			-- Offer of employment
			form:Clear()
			form:SetTitle(candidate.name)

			if candidate:TestRoll('playerRelationship',15) then
				-- Boosting roll by 15, because they want to work
				if Game.player:Enroll(candidate) then
					candidate.contract = {
						wage = offer,
						payday = Game.time + wage_period,
						outstanding = 0
					}
					form:SetMessage(l.THANKS_ILL_GET_SETTLED_ON_BOARD_IMMEDIATELY)
					form:AddOption(l.GO_BACK, 0)
					for k,v in ipairs(nonPersistentCharactersForCrew[station]) do
						-- Take them off the available list in the ad
						if v == candidate then table.remove(nonPersistentCharactersForCrew[station],k) end
					end
				else
					form:SetMessage(l.THERE_DOESNT_SEEM_TO_BE_SPACE_FOR_ME_ON_BOARD)
					form:AddOption(l.GO_BACK, 0)
				end
			else
				form:SetMessage(l.IM_SORRY_YOUR_OFFER_ISNT_ATTRACTIVE_TO_ME)
				form:AddOption(l.GO_BACK, 0)
			end
			offer = nil
			candidate = nil
		end

		if option == 2 then
			-- Player suggested to increase the offer with $10
			candidate.playerRelationship = candidate.playerRelationship + 2
			offer = checkOffer(offer + 10)
			candidate.estimatedWage = offer -- They'll now re-evaluate themself

			if candidate.playerRelationship < 15 then
				showCandidateDetails(l.I_WOULD_NEVER_CONSIDER_WORKING_FOR_YOU)
			else
				showCandidateDetails(l.THATS_EXTREMELY_GENEROUS_OF_YOU)
			end
		end

		if option == 3 then
			-- Player suggested an extra $5
			candidate.playerRelationship = candidate.playerRelationship + 1
			offer = checkOffer(offer + 5)
			candidate.estimatedWage = offer -- They'll now re-evaluate themself
			if candidate.playerRelationship < 15 then
				showCandidateDetails(l.I_WOULD_NEVER_CONSIDER_WORKING_FOR_YOU)
			else
				showCandidateDetails(l.THAT_CERTAINLY_MAKES_THIS_OFFER_LOOK_BETTER)
			end
		end

		if option == 4 then
			-- Player suggested $5 less
			candidate.playerRelationship = candidate.playerRelationship - 1
			if candidate:TestRoll('playerRelationship') then
				offer = checkOffer(offer - 5)
				showCandidateDetails(l.OK_I_SUPPOSE_THATS_ALL_RIGHT)
			else
				showCandidateDetails(l.IM_SORRY_IM_NOT_PREPARED_TO_GO_ANY_LOWER)
			end
		end

		if option == 5 then
			-- Player suggested lowering the offer with $10
			candidate.playerRelationship = candidate.playerRelationship - 2
			if candidate.playerRelationship < 15 then
				showCandidateDetails(l.I_WOULD_NEVER_CONSIDER_WORKING_FOR_YOU)
			else
				showCandidateDetails(l.THATS_EXTREMELY_GENEROUS_OF_YOU)
			end
			if candidate:TestRoll('playerRelationship') then
				offer = checkOffer(offer - 10)
				showCandidateDetails(l.OK_I_SUPPOSE_THATS_ALL_RIGHT)
			else
				showCandidateDetails(l.IM_SORRY_IM_NOT_PREPARED_TO_GO_ANY_LOWER)
			end
		end

		if option == 6 then
			-- Player asks candidate to perform a test
			form:Clear()
			form:SetTitle(candidate.name)
			local general,engineering,piloting,navigation,sensors,lawfulness = 0,0,0,0,0,0
			for i = 1,10 do
				if candidate:TestRoll('intelligence') then general = general + 10 end
				if candidate:TestRoll('engineering') then engineering = engineering + 10 end
				if candidate:TestRoll('piloting') then piloting = piloting + 10 end
				if candidate:TestRoll('navigation') then navigation = navigation + 10 end
				if candidate:TestRoll('sensors') then sensors = sensors + 10 end
				if candidate:TestRoll('lawfulness') then lawfulness = lawfulness + 10 end
			end
			-- Candidates hate being tested.
			candidate.playerRelationship = candidate.playerRelationship - 1
			-- Show results

			local lawfulness_impression = ""
			if lawfulness > 90 then lawfulness_impression = l.FOLLOWS_LAW_TO_THE_LETTER
			elseif lawfulness > 70 then lawfulness_impression = l.LAW_ABIDING_CITIZEN
			elseif lawfulness > 40 then lawfulness_impression = l.NOT_PRO_OR_CONTRA_LAW
			elseif lawfulness > 10 then lawfulness_impression = l.WILL_DO_WHAT_THEY_WANT
			else lawfulness_impression = l.LAWS_ARE_FOR_OTHERS end

			local civaffinity_impression = ""
			if candidate.civaffinity == "high" then civaffinity_impression = l.LOVES_CULTURE
			elseif candidate.civaffinity == "medium" then civaffinity_impression = l.DOESNT_NEED_FREQUENT_CULTURE
			elseif candidate.civaffinity == "low" then civaffinity_impression = l.WANTS_TO_GET_AWAY_FROM_CIVILIZATION
			end

			form:SetMessage(l.CREWTESTRESULTSBB:interp{
								lawfulness_impression = lawfulness_impression,
								civaffinity_impression = civaffinity_impression,
								general = general,
								engineering = engineering,
								piloting = piloting,
								navigation = navigation,
								sensors = sensors,
								overall = math.ceil((general+general+engineering+piloting+navigation+sensors)/6),
			})
			form:AddOption(l.GO_BACK, 7)
		end

		if option == 7 then
			-- Player is done looking at the test results, just return
			showCandidateDetails('')
		end

	end
end

local isEnabled = function (ref)
	local station = stationsWithAdverts[ref]
	local numCrewmenAvailable = 0
	for k,v in pairs(nonPersistentCharactersForCrew[station]) do
		numCrewmenAvailable = numCrewmenAvailable + 1
	end
	return numCrewmenAvailable > 0
end

local onCreateBB = function (station)
	-- Create non-persistent Characters as available crew
	nonPersistentCharactersForCrew[station] = {}

	-- Only one crew hiring thingy per station
	stationsWithAdverts[station:AddAdvert({
		title       = l.CREW_FOR_HIRE_TITLE,
		description = l.CREW_FOR_HIRE_DESC,
		icon        = "crew_contracts",
		onChat      = onChat,
		isEnabled   = isEnabled})] = station

	-- Number is based on population, nicked from Assassinations.lua and tweaked
	for i = 1, Engine.rand:Integer(0, math.ceil(Game.system.population) * 2 + 1) do
		local hopefulCrew = Character.New()
		-- Roll new stats, with a 1/3 chance that they're utterly inexperienced
		hopefulCrew:RollNew(Engine.rand:Integer(0, 2) > 0)
		-- Make them a title if they're good at anything
		local maxScore = math.max(hopefulCrew.engineering,
									hopefulCrew.piloting,
									hopefulCrew.navigation,
									hopefulCrew.sensors)
		if maxScore > 45 then
			if hopefulCrew.engineering == maxScore then hopefulCrew.title = lui.SHIPS_ENGINEER end
			if hopefulCrew.piloting == maxScore then hopefulCrew.title = lui.PILOT end
			if hopefulCrew.navigation == maxScore then hopefulCrew.title = lui.NAVIGATOR end
			if hopefulCrew.sensors == maxScore then hopefulCrew.title = lui.SENSORS_AND_DEFENCE end
		end
		table.insert(nonPersistentCharactersForCrew[station],hopefulCrew)
	end
end

Event.Register("onCreateBB", onCreateBB)

-- Wipe temporary crew out when hyperspacing
Event.Register("onEnterSystem", function(ship)
	if ship:IsPlayer() then
		nonPersistentCharactersForCrew = {}
		stationsWithAdverts = {}
	end
end)

-- Load temporary crew from saved data
local loaded_data
Event.Register("onGameStart", function()
	-- XXX Need to re-initialise these until Lua is re-initialised with a new game
	nonPersistentCharactersForCrew = {}
	stationsWithAdverts = {}
	if loaded_data and loaded_data.stationsWithAdverts then
		nonPersistentCharactersForCrew = loaded_data.nonPersistentCharactersForCrew
		for k,station in pairs(loaded_data.stationsWithAdverts) do
		stationsWithAdverts[station:AddAdvert({
			title       = l.CREW_FOR_HIRE_TITLE,
			description = l.CREW_FOR_HIRE_DESC,
			icon        = "crew_contracts",
			onChat      = onChat,
			isEnabled   = isEnabled})] = station
		end
		loaded_data = nil
	end
end)

local serialize = function ()
	return {
		nonPersistentCharactersForCrew = nonPersistentCharactersForCrew,
		stationsWithAdverts = stationsWithAdverts,
	}
end

local unserialize = function (data)
	loaded_data = data
end

Serializer:Register('CrewContracts',serialize,unserialize)
