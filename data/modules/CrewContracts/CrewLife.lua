-- Copyright © 2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt


-- This sub-module describes the crew life aboard the ship

local Event     = require 'Event'
local Comms     = require 'Comms'
local Game      = require 'Game'
local Timer     = require 'Timer'
local Character = require 'Character'
local Lang      = require 'Lang'
local Rand      = require 'Rand'
local FlightLog = require 'FlightLog'

local rand      = Rand.New()
local l         = Lang.GetResource("module-crewcontracts")

local week_in_secs = 604800 -- a week of seconds
local month_in_secs = 2629800 -- month in seconds


-- thought = {text = "short thought description",
--			  adjustment = signed integer that adjusts the playerRelationship,
--            time = Game.time when thought was applied}
-- memories = {}  -- an ordered list of thoughts
-- civaffinity = ("high" | "medium" | "low")

local max_memory = 4           -- maximum number of thoughts retained
local max_relationship = 65    -- maximum playerRelationship score
local min_relationship = 4     -- minimum playerRelationship score
local desert_threshold = 15    -- playerRelationship threshold at which crew members start deserting the ship
local law_upper_threshold = 45 -- lawfulness threshold at which lawfull crew can be unhappy with illegal trading
local law_lower_threshold = 25 -- lawfulness threshold at which lawless crew can be happy with illegal trading
local civ_high_threshold = 0.4 -- rolling mean visited system population threshold (high) for crew civ affinity happiness testing
local civ_low_threshold = 0.1  -- rolling mean visited system population threshold (low) for crew civ affinity happiness testing
local explored_threshold = 2   -- systems explored within the last 5 systems visited for triggering happiness impact
local decay_threshold = month_in_secs -- time in seconds after which thoughts start to drop from memory (plus up to a week)
local max_repeat_memory = 2    -- maximum time the same thought can exist in memory (avoids same-thought spamming)


local mean = function (x)
	-- Return the mean of the values in x. Assumes values are numeric.
	local sum = 0
	for _, value in pairs(x) do
		sum = sum + value
	end
	return sum / #x
end


local thoughts = {
	employment           = {text = l.THOUGHT_EMPLOYMENT,             adjustment = 10, time = 0},
	happy_home           = {text = l.THOUGHT_HAPPY_HOME,             adjustment = 1,  time = 0},
	illegal_trading_bad  = {text = l.THOUGHT_ILLEGAL_TRADING_BAD,    adjustment = -1, time = 0},
	illegal_trading_good = {text = l.THOUGHT_ILLEGAL_TRADING_GOOD,   adjustment = 1,  time = 0},
	offender_bad         = {text = l.THOUGHT_OFFENDER,               adjustment = -1, time = 0},
	offender_good        = {text = l.THOUGHT_OFFENDER,               adjustment = 1,  time = 0},
	criminal_bad         = {text = l.THOUGHT_CRIMINAL,               adjustment = -2, time = 0},
	ciminal_good         = {text = l.THOUGHT_CRIMINAL,               adjustment = 2,  time = 0},
	outlaw_bad           = {text = l.THOUGHT_OUTLAW,                 adjustment = -3, time = 0},
	outlaw_good          = {text = l.THOUGHT_OUTLAW,                 adjustment = 3,  time = 0},
	fugitive_bad         = {text = l.THOUGHT_FUGITIVE,               adjustment = -4, time = 0},
	fugitve_good         = {text = l.THOUGHT_FUGITIVE,               adjustment = 4,  time = 0},
	high_civ_good        = {text = l.THOUGHT_WELL_DEVELOPED_SYSTEMS, adjustment = 1,  time = 0},
	high_civ_bad         = {text = l.THOUGHT_TOO_MANY_BUSY_SYSTEMS,  adjustment = -1, time = 0},
	low_civ_good         = {text = l.THOUGHT_QUIET_SYSTEMS,          adjustment = 1,  time = 0},
	low_civ_bad          = {text = l.THOUGHT_TOO_FEW_BUSY_SYSTEMS,   adjustment = -1, time = 0},
	frontier_good        = {text = l.THOUGHT_EXPLORING_THE_FRONTIER, adjustment = 2,  time = 0},
	frontier_bad         = {text = l.THOUGHT_DULL_FRONTIER,          adjustment = -2, time = 0}
}


local boostCrewSkills = function (crewMember)
	-- Each week, there's a small chance that a crew member gets better
	-- at each skill, due to the experience of working on the ship.

	-- If they fail their intelligence roll, they learn nothing.
	if not crewMember:TestRoll('intelligence') then return end

	-- The attributes to be tested and possibly enhanced. These will be sorted
	-- by their current value, but appear first in arbitrary order.
	local attribute = {
		{'engineering',crewMember.engineering},
		{'piloting',crewMember.piloting},
		{'navigation',crewMember.navigation},
		{'sensors',crewMember.sensors},
	}
	table.sort(attribute,function (a,b) return a[2] > b[2] end)
	-- The sorted attributes mean that the highest scoring attribute gets the
	-- first opportunity for improvement. The next loop actually makes it harder
	-- for the highest scoring attributes to improve, so if they fail, the next
	-- one is given an opportunity. At most one attribute will be improved, and
	-- the distribution means that, for example, a pilot will improve in piloting
	-- first, but once that starts to get very good, the other skills will start
	-- to see an improvement.

	for i = 1,#attribute do
		-- A carefully weighted test here. Scores will creep up to the low 50s.
		if not crewMember:TestRoll(attribute[i][1],math.floor(attribute[i][2] * 0.2 - 10)) then
			-- They learned from their failure,
			crewMember[attribute[i][1]] = crewMember[attribute[i][1]]+1
			-- but only on this skill.
			break
		end
	end
end


local scheduleWages = function (crewMember)
	-- Must have a contract to be treated like crew
	if not crewMember.contract then return end

	local payWages
	payWages = function ()
		local contract = crewMember.contract
		-- Check if crew member has been dismissed
		if not contract then return end

		if Game.player:GetMoney() > contract.wage then
			Game.player:AddMoney(0 - contract.wage)
			-- Being paid can make awkward crew like you more
			if not crewMember:TestRoll('playerRelationship') then
				crewMember.playerRelationship = crewMember.playerRelationship + 1
			end
		else
			contract.outstanding = contract.outstanding + contract.wage
			crewMember.playerRelationship = crewMember.playerRelationship - 1
			Character.persistent.player.reputation = Character.persistent.player.reputation - 0.5
		end

		-- Attempt to pay off any arrears
		local arrears = math.min(Game.player:GetMoney(),contract.outstanding)
		Game.player:AddMoney(0 - arrears)
		contract.outstanding = contract.outstanding - arrears

		-- The crew gain experience each week, and might get better
		boostCrewSkills(crewMember)

		-- Schedule the next pay day, if there is one.
		if contract.payday and not crewMember.dead then
			contract.payday = contract.payday + week_in_secs
			Timer:CallAt(math.max(Game.time + 5,contract.payday),payWages)
		end
	end

	Timer:CallAt(math.max(Game.time + 1,crewMember.contract.payday),payWages)
end


-- Applies the supplied thought to the supplied crew member. This will commit the thought to their memory
-- (if the memory is not full) and make appropriate adjustments to the playerRelationship variable.
local applyThought = function (crewMember, thought)

	-- apply only to non-player crew members
	if crewMember.player then return end

	-- avoid same-memory spamming
	local same_memories = 0
	for _, memory in pairs(crewMember.memories) do
		if memory.text == thought.text then
			same_memories = same_memories + 1
		end
	end
	if same_memories >= max_repeat_memory then return end
	
	-- adjust relationship with player
	crewMember.playerRelationship = crewMember.playerRelationship + thought.adjustment
	if crewMember.playerRelationship > max_relationship then crewMember.playerRelationship = max_relationship end
	if crewMember.playerRelationship < min_relationship then crewMember.playerRelationship = min_relationship end

	-- add thought to thought stack, respecting thought memory max
	if not crewMember.memories then crewMember.memories = {} end
	thought.time = Game.time
	if #crewMember.memories == max_memory then table.remove(crewMember.memories, 1) end
	table.insert(crewMember.memories, thought)
end


-- Drop thoughts from memory if they are older than the decay threshold
-- Gets run every week
local decayThoughts = function ()
	for crewMember in Game.player:EachCrewMember() do
		if not crewMember.player then
			for i, thought in pairs(crewMember.memories) do
				if thought.time < Game.time - decay_threshold then
					table.remove(crewMember.memories, i)
				end
			end
		end
	end
end


-- Deserts a crew member from the ship. Happens after docking to a space port.
local desertCrew = function(crewMember)
	if Game.player:Dismiss(crewMember) then

		crewMember:Save() -- Save to persistent characters list

		Comms.Message(l.SICK_OF_WORKING_FOR_YOU, crewMember.name)

		if crewMember.contract.outstanding > 0 then
			Comms.Message(l.I_WILL_SEND_SOMEONE_AFTER_YOU, crewMember.name)
		end
	end
end

-- This gets run just after crew are restored from a saved game
local crewAvailable = function ()
	-- scheduleWages() for everybody
	for crewMember in Game.player:EachCrewMember() do
		if not crewMember.player then
			scheduleWages(crewMember)

			-- for old saves compatibility, add empty crew memory bank if none exists
			if not crewMember.memories then
				crewMember.memories = {}
			end
		end
	end
end


-- This gets run whenever the ship cargo changes
local onPlayerCargoChanged = function (comm, amount)
	if not Game.system:IsCommodityLegal(comm.name) then
		for crewMember in Game.player:EachCrewMember() do
			if not crewMember.player then

				-- crew happy or upset about illegal goods (depending on lawfulness)
				if crewMember.lawfulness > law_upper_threshold and crewMember:TestRoll('lawfulness') then
					applyThought(crewMember, thoughts['illegal_trading_bad'])
				elseif crewMember.lawfulness < law_lower_threshold and not crewMember:TestRoll('lawfulness') then
					applyThought(crewMember, thoughts['illegal_trading_good'])
				end
			end
		end
	end
end


-- This gets run whenever a crew member joins a ship
local onJoinCrew = function (ship, crewMember)
	if ship:IsPlayer() then
	   scheduleWages(crewMember)

	   -- start with blank memory stack
	   crewMember.memories = {}

	   -- happy because of employment
	   applyThought(crewMember, thoughts['employment'])

	   -- start tracking visits to home
	   crewMember.homeStation = ship:GetDockedWith().path
	   crewMember.lastHomeVisit = Game.time
	end
end


-- This gets run whenever a crew member leaves a ship
local onLeaveCrew = function (ship, crewMember)
	if ship:IsPlayer() and crewMember.contract then
		-- Prepare them for the job market
		crewMember.estimatedWage = crewMember.contract.wage + 5
		-- Terminate their contract
		crewMember.contract = nil

		-- clean up custom variables
		crewMember.homeStation = nil
		crewMember.lastHomeVisit = nil
	end
end


local onShipDocked = function (ship, station)
	if not ship:IsPlayer() then return end

	for crewMember in Game.player:EachCrewMember() do
		if not crewMember.player then

			-- check for deserting crew members at each station dock
			if crewMember.playerRelationship < desert_threshold then
				if not crewMember:TestRoll('playerRelationship') then
					desertCrew(crewMember)
				end
			end

			-- good thought if visiting home system of the crew member
			-- assumes that the last saved location for this character
			-- is actually it's "home"
			if station.path == crewMember.homeStation then
				-- only triggers if last visit is more than a month ago
				if Game.time - month_in_secs > crewMember.lastHomeVisit then
					applyThought(crewMember, thoughts["happy_home"])
				end
				crewMember.lastHomeVisit = Game.time
			end
		end
	end
end


local onEnterSystem = function (ship)
	if not ship:IsPlayer() then return end

	for crewMember in Game.player:EachCrewMember() do
		if not crewMember.player then

			-- collect some information about the last 5 systems visited
			local pops = {}
			local explored = {}
			for path, _, _, _ in FlightLog.GetSystemPaths(5) do
				table.insert(pops, path:GetStarSystem().population)
				if path:GetStarSystem().explored then
					table.insert(explored, 1)
				else
					table.insert(explored, 0)
				end
			end

			local randint = rand:Integer(1, 5)
			-- check for player legal status effect on crew happiness
			if randint == 1 then
				local status = Game.player:GetLegalStatus()
				if crewMember.lawfulness > law_upper_threshold and crewMember:TestRoll('lawfulness') then
					if status == 'OFFENDER' then applyThought(crewMember, thoughts['offender_bad'])
					elseif status == 'CRIMINAL' then applyThought(crewMember, thoughts['criminal_bad'])
					elseif status == 'OUTLAW' then applyThought(crewMember, thoughts['outlaw_bad'])
					elseif status == 'FUGITIVE' then applyThought(crewMember, thoughts['fugitive_bad'])
					end
				elseif crewMember.lawfulness < law_lower_threshold and not crewMember:TestRoll('lawfulness') then
					if status == 'OFFENDER' then applyThought(crewMember, thoughts['offender_good'])
					elseif status == 'CRIMINAL' then applyThought(crewMember, thoughts['criminal_good'])
					elseif status == 'OUTLAW' then applyThought(crewMember, thoughts['outlaw_good'])
					elseif status == 'FUGITIVE' then applyThought(crewMember, thoughts['fugitive_good'])
					end
				end

			-- check for system population effect on crew happiness
				elseif randint == 2 then
				local mean_pops = mean(pops)
				if crewMember.civaffinity == 'high' then
					if mean_pops > civ_high_threshold then applyThought(crewMember, thoughts['high_civ_good'])
					elseif mean_pops < civ_low_threshold then applyThought(crewMember, thoughts['low_civ_bad'])
					end
				elseif crewMember.civaffinity == 'low' then
					if mean_pops > civ_high_threshold then applyThought(crewMember, thoughts['high_civ_bad'])
					elseif mean_pops < civ_low_threshold then applyThought(crewMember, thoughts['low_civ_good'])
					end
				end

			-- check for system exploration status on crew happiness
			elseif randint == 3 then
				local num_explored = 0
				for _, value in pairs(explored) do
					num_explored = num_explored + value
				end
				if crewMember.cifaffinity == 'high' and num_explored > explored_threshold then
					applyThought(crewMember, thoughts['frontier_bad'])
				elseif crewMember.civaffinity == 'low' and num_explored > explored_threshold then
					applyThought(crewMember, thoughts['frontier_good'])
				end
			end
		end
	end
end


-- debug only
local thoughtGenerator = function()
	print("=====thoughtGenerator triggered")

	for crewMember in Game.player:EachCrewMember() do
		if not crewMember.player then

			crewMember.playerRelationship = 0
			
			-- local randint = rand:Integer(1, 18)
			-- print("Thoughttest")
			-- print(randint)
			-- local thought_keys = {}
			-- for key,_ in pairs(thoughts) do
			-- 	table.insert(thought_keys, key)
			-- end
			-- local thought = thoughts[thought_keys[randint]]
			-- print(thought.text)
			-- applyThought(crewMember, thought)

			-- applyThought(crewMember, thoughts['negative'])
			-- local randint = rand:Integer(0, 1)
			-- local thought = {}
			-- if randint == 1 then
			-- 	thought = thoughts['positive']
			-- else
			-- 	thought = thoughts['negative']
			-- end
			-- applyThought(crewMember, thought)
			
			-- Comms.ImportantMessage("another thought")
		end
	end
end
--



local onGameStart = function ()
	Game.player:GetComponent('CargoManager'):AddListener('crewlife', onPlayerCargoChanged)
	Timer:CallEvery(week_in_secs, decayThoughts)

	-- debug only
	-- Timer:CallEvery(10, thoughtGenerator)
	--
end


Event.Register("onGameStart", onGameStart)
Event.Register("onShipDocked", onShipDocked)
Event.Register("onLeaveCrew", onLeaveCrew)
Event.Register("onJoinCrew", onJoinCrew)
Event.Register("crewAvailable", crewAvailable)
Event.Register("onEnterSystem", onEnterSystem)


-- Serializer:Register('CrewLife',serialize,unserialize)
