-- Copyright © 2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt


-- This sub-module describes the crew life aboard the ship

local Event       = require 'Event'
local Comms       = require 'Comms'
local Game        = require 'Game'
local Timer       = require 'Timer'
local Character   = require 'Character'
local Lang        = require 'Lang'
local Rand        = require 'Rand'
local PlayerState = require 'PlayerState'
local FlightLog   = require 'modules.FlightLog.FlightLog'
local utils       = require 'utils'

local rand      = Rand.New()
local l         = Lang.GetResource("module-crewcontracts")

local week_in_secs = 604800 -- a week of seconds
local month_in_secs = 2629800 -- month in seconds


local max_memory = 4                  -- maximum number of thoughts retained
local max_relationship = 65           -- maximum playerRelationship score
local min_relationship = 4            -- minimum playerRelationship score
local desert_threshold = 15           -- playerRelationship threshold at which crew members start deserting the ship
local desert_pop_threshold = 0.4      -- only desert ship if at busy station
local law_upper_threshold = 45        -- lawfulness threshold at which lawfull crew can be unhappy with illegal trading
local law_lower_threshold = 25        -- lawfulness threshold at which lawless crew can be happy with illegal trading
local civ_high_threshold = 0.4        -- rolling mean visited system population threshold (high) for crew civ affinity happiness testing
local civ_low_threshold = 0.1         -- rolling mean visited system population threshold (low) for crew civ affinity happiness testing
local explored_threshold = 2          -- systems explored within the last 5 systems visited for triggering happiness impact
local decay_threshold = month_in_secs -- time in seconds after which thoughts start to drop from memory (plus up to a week)
local max_repeat_memory = 2           -- maximum time the same thought can exist in memory (avoids same-thought spamming)


-- global containers and variables
local crewlife = {
	thoughts = {}     -- available crew member thoughts
}


-- thought = {text       = "short thought description",
--	      adjustment = signed integer that adjusts the playerRelationship,
--            time       = Game.time when thought was applied,
--            chance     = probability that the thought actually gets applied}

crewlife.thoughts = {
	employment           = {text = l.THOUGHT_EMPLOYMENT,             adjustment = 10, time = 0, chance = 1   },
	happy_home           = {text = l.THOUGHT_HAPPY_HOME,             adjustment = 1,  time = 0, chance = 1   },
	not_paid             = {text = l.THOUGHT_NOT_PAID,               adjustment = -4, time = 0, chance = 1   },
	paid                 = {text = l.THOUGHT_PAID,                   adjustment = 1,  time = 0, chance = 1   },
	illegal_trading_bad  = {text = l.THOUGHT_ILLEGAL_TRADING_BAD,    adjustment = -1, time = 0, chance = 0.5 },
	illegal_trading_good = {text = l.THOUGHT_ILLEGAL_TRADING_GOOD,   adjustment = 1,  time = 0, chance = 0.25},
	offender_bad         = {text = l.THOUGHT_OFFENDER,               adjustment = -1, time = 0, chance = 0.5 },
	offender_good        = {text = l.THOUGHT_OFFENDER,               adjustment = 1,  time = 0, chance = 0.25},
	criminal_bad         = {text = l.THOUGHT_CRIMINAL,               adjustment = -2, time = 0, chance = 0.5 },
	criminal_good        = {text = l.THOUGHT_CRIMINAL,               adjustment = 1,  time = 0, chance = 0.25},
	outlaw_bad           = {text = l.THOUGHT_OUTLAW,                 adjustment = -3, time = 0, chance = 0.5 },
	outlaw_good          = {text = l.THOUGHT_OUTLAW,                 adjustment = 2,  time = 0, chance = 0.25},
	fugitive_bad         = {text = l.THOUGHT_FUGITIVE,               adjustment = -4, time = 0, chance = 0.5 },
	fugitive_good        = {text = l.THOUGHT_FUGITIVE,               adjustment = 3,  time = 0, chance = 0.25},
	high_civ_good        = {text = l.THOUGHT_WELL_DEVELOPED_SYSTEMS, adjustment = 1,  time = 0, chance = 0.5 },
	high_civ_bad         = {text = l.THOUGHT_TOO_MANY_BUSY_SYSTEMS,  adjustment = -1, time = 0, chance = 0.5 },
	low_civ_good         = {text = l.THOUGHT_QUIET_SYSTEMS,          adjustment = 1,  time = 0, chance = 0.5 },
	low_civ_bad          = {text = l.THOUGHT_TOO_FEW_BUSY_SYSTEMS,   adjustment = -1, time = 0, chance = 0.5 },
	frontier_good        = {text = l.THOUGHT_EXPLORING_THE_FRONTIER, adjustment = 2,  time = 0, chance = 0.5 },
	frontier_bad         = {text = l.THOUGHT_DULL_FRONTIER,          adjustment = -2, time = 0, chance = 0.5 }
}



--
-- Function: math
--
-- Perform simple math on a table with numeric values.
--
-- Parameters:
--
--  x - a table with numeric values
--  fun - mathematical function to run ("sum", "mean")
--
-- Returns:
--
--  Returns the numeric result of a mathematical function, run on the numerical values of the provided table.
local tablemath = function(x, fun)
	local sum = 0
	for _, value in pairs(x) do
		sum = sum + value
	end

	if (fun == "sum") then
		return sum
	elseif (fun == "mean") then
		return sum / #x
	end
end


--
-- Function: boostCrewSkills
--
-- Each week, there's a small chance that a crew member gets better
-- at each skill, due to the experience of working on the ship.
--
-- Parameters:
--
--   crewMember - a crewMember object
local boostCrewSkills = function (crewMember)

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


--
-- Function: payWages
--
-- Pay wages. Triggered by contractRunner.
--
-- Parameters:
--
--   crewMember - a crewMember object
local payWages = function(crewMember)
    local contract = crewMember.contract

    if Game.player:GetMoney() > contract.wage then
        Game.player:AddMoney(0 - contract.wage)

        -- Being paid can make unhappy crew like you more
        if not crewMember:TestRoll('playerRelationship') then
			crewlife.applyThought(crewMember, crewlife.thoughts['paid'])
        end
    else
		contract.outstanding = contract.outstanding + contract.wage
		crewlife.applyThought(crewMember, crewlife.thoughts['not_paid'])

		-- TODO: is this still necessary?
		Character.persistent.player.reputation = Character.persistent.player.reputation - 0.5
    end

    -- Attempt to pay off any arrears
    local arrears = math.min(Game.player:GetMoney(), contract.outstanding)
    Game.player:AddMoney(0 - arrears)
    contract.outstanding = contract.outstanding - arrears
end


--
-- Function: scheduleContract
--
-- Schedule the contract timer for wages, etc.
--
-- Parameters:
--
--   crewMember - a crewMember object
local scheduleContract = function(crewMember)
	if not crewMember.contract then return end

	local contractRunner
	contractRunner = function ()
		-- check for valid contract again each time this is run
		-- in case crewMember was dismissed
		if not crewMember.contract then return end
		payWages(crewMember)
		boostCrewSkills(crewMember)

		-- schedule next run
		if contract.payday and not crewMember.dead then
			contract.payday = contract.payday + week_in_secs
			Timer:CallAt(math.max(Game.time + 5, contract.payday), payWages)
		end

	end
	
	Timer:CallAt(math.max(Game.time + 1, crewMember.contract.payday), contractRunner)
end


--
-- Function: applyThought
--
-- Apply the supplied thought to the supplied crew member. This will commit the thought to their memory
-- (if the memory is not full) and make appropriate adjustments to the playerRelationship variable.
--
-- Parameters:
--
--   crewMember - a crewMember object
--   thought - a thought (as defined above)
function crewlife.applyThought (crewMember, thought)

	-- apply only to non-player crew members
	if crewMember.player then return end

	-- check for probability that this thought is applied
	if thought.chance >= rand:Number() then

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
		local memory = table.copy(thought)
		memory.time = Game.time
		if #crewMember.memories == max_memory then table.remove(crewMember.memories, 1) end
		table.insert(crewMember.memories, memory)
	end
end


--
-- Function: decayThoughts
--
-- Drop thoughts from memory if they are older than the decay threshold.
-- Gets run every week.
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


--
-- Function: desertCrew
--
-- Checks for crew members that may want to desert the ship and lets them do so if successful.
-- Checking happens after docking to a space port.
--
-- Parameters:
--
--   crewMember - a crewMember object
--   station - a station object
local desertCrew = function(crewMember, station)
	if crewMember.playerRelationship < desert_threshold then

		-- only desert ship if at relatively populated station
		-- role play: only leave if there is reasonable expectation of new job
		-- game play: don't leave the player stranded without chance for new crew
		if station.path:GetSystemBody().parent.population > desert_pop_threshold then
			if not crewMember:TestRoll('playerRelationship') then
				if Game.player:Dismiss(crewMember) then
					crewMember:Save() -- Save to persistent characters list
					Comms.Message(l.SICK_OF_WORKING_FOR_YOU, crewMember.name)
					if crewMember.contract.outstanding > 0 then
						Comms.Message(l.I_WILL_SEND_SOMEONE_AFTER_YOU, crewMember.name)
					end
				end
			end
		end
	end
end


--
-- Function: crewAvailable
--
-- Triggered by event that restores crew from a saved game. Re-starts contract timers, etc.
local crewAvailable = function ()
	-- re-start contract schedule for everyone
	for crewMember in Game.player:EachCrewMember() do
		if not crewMember.player then
			scheduleContract(crewMember)

			-- for old saves compatibility, add empty crew memory bank if none exists
			if not crewMember.memories then
				crewMember.memories = {}
			end
		end
	end
end
Event.Register("crewAvailable", crewAvailable)


--
-- Function: onPlayerCargoChanged
--
-- Added using a custom listener during game start. This gets run whenever the ship cargo
-- changes. Changing cargo can have an impact on crew thoughts and happiness, depending on
-- the legality of the cargo and the crew disposition.
--
-- Parameters:
--
--   comm - commodity type added/removed
--   amount - number of items added/removed
local onPlayerCargoChanged = function(comm, amount)
	if not Game.system:IsCommodityLegal(comm.name) then
		for crewMember in Game.player:EachCrewMember() do
			if not crewMember.player then

				-- crew happy or upset about illegal goods (depending on lawfulness)
				if crewMember.lawfulness > law_upper_threshold and crewMember:TestRoll('lawfulness') then
					crewlife.applyThought(crewMember, crewlife.thoughts['illegal_trading_bad'])
				elseif crewMember.lawfulness < law_lower_threshold and not crewMember:TestRoll('lawfulness') then
					crewlife.applyThought(crewMember, crewlife.thoughts['illegal_trading_good'])
				end
			end
		end
	end
end


--
-- Method: onJoinCrew
--
-- Triggered by event whenever a crew member joins the player ship.
--
-- Parameters:
--
--   ship - a ship object
--   crewMember - a crewMember object
crewlife.onJoinCrew = function(ship, crewMember)
    if ship:IsPlayer() then
        scheduleContract(crewMember)

        -- start with blank memory stack
        crewMember.memories = {}

        -- happy because of employment
        crewlife.applyThought(crewMember, crewlife.thoughts['employment'])

        -- start tracking visits to home
        -- TODO: add home station when creating character, not here
		-- home station should not necessarily be where they hired from
        crewMember.homeStation = ship:GetDockedWith().path
        crewMember.lastHomeVisit = Game.time
    end
end
Event.Register("onJoinCrew", crewlife.onJoinCrew)


--
-- Function: onLeaveCrew
--
-- Triggered by event whenever a crew member leaves the player ship.
--
-- Parameters:
--
--   ship - a ship object
--   crewMember - a crewMember object
local onLeaveCrew = function(ship, crewMember)
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
Event.Register("onLeaveCrew", onLeaveCrew)


--
-- Function: onPlayerDocked
--
-- Triggered by event whenever the player ship docks with a station.
--
-- Parameters:
--
--   ship - a ship object
--   station - a station object
local onPlayerDocked = function(ship, station)
    if not ship:IsPlayer() then return end

    for crewMember in Game.player:EachCrewMember() do
        if not crewMember.player then
            -- check for deserting crew members at each station dock
            desertCrew(crewMember, station)

            -- good thought if visiting home system of the crew member
            -- assumes that the last saved location for this character
            -- is actually it's "home"
            if station.path == crewMember.homeStation then
                -- only triggers if last visit is more than a month ago
                if Game.time - month_in_secs > crewMember.lastHomeVisit then
                    crewlife.applyThought(crewMember, crewlife.thoughts["happy_home"])
                end
                crewMember.lastHomeVisit = Game.time
            end
        end
    end
end
Event.Register("onPlayerDocked", onPlayerDocked)


--
-- Method: recentSystems
--
-- Provide some stats about the recently visited systems.
--
-- Returns:
--
--   Returns a table with multiple average stats about the last visited systems. 
crewlife.recentSystems = function ()
	local hist = 5    -- how many historical systems to consider
	local count = 0
	local visited = {}
	local pops = {}
	local explored = {}
	local stats = {}
	for entry in FlightLog:GetLogEntries({ System = true }, nil) do
		if entry.arrtime and count < hist then
			table.insert(visited, entry.systemp:GetStarSystem())
			count = count + 1
		end
	end

	for _, system in pairs(visited) do
		table.insert(pops, system.population)
		if system.explored then
			table.insert(explored, 1)
		else
			table.insert(explored, 0)
		end
	end

	stats.mean_pop = tablemath(pops, "mean")
	stats.explored = tablemath(explored, "sum")
	stats.n = utils.count(visited)
	return(stats)
end


--
-- Method: onEntersystem
--
-- Triggered by event whenever the player ship enters a system.
--
-- Parameters:
--
--   ship - the player ship
function crewlife.onEnterSystem(ship)
    if not ship:IsPlayer() then return end

    local stats = crewlife.recentSystems()

    for crewMember in Game.player:EachCrewMember() do
		if not crewMember.player then
			
			-- check for player legal status effect on crew happiness
			local status = PlayerState:GetLegalStatus()
			if crewMember.lawfulness > law_upper_threshold and crewMember:TestRoll('lawfulness') then
				if status == 'OFFENDER' then
					crewlife.applyThought(crewMember, crewlife.thoughts['offender_bad'])
				elseif status == 'CRIMINAL' then
					crewlife.applyThought(crewMember, crewlife.thoughts['criminal_bad'])
				elseif status == 'OUTLAW' then
					crewlife.applyThought(crewMember, crewlife.thoughts['outlaw_bad'])
				elseif status == 'FUGITIVE' then
					crewlife.applyThought(crewMember, crewlife.thoughts['fugitive_bad'])
				end
			elseif crewMember.lawfulness < law_lower_threshold and not crewMember:TestRoll('lawfulness') then
				if status == 'OFFENDER' then
					crewlife.applyThought(crewMember, crewlife.thoughts['offender_good'])
				elseif status == 'CRIMINAL' then
					crewlife.applyThought(crewMember, crewlife.thoughts['criminal_good'])
				elseif status == 'OUTLAW' then
					crewlife.applyThought(crewMember, crewlife.thoughts['outlaw_good'])
				elseif status == 'FUGITIVE' then
					crewlife.applyThought(crewMember, crewlife.thoughts['fugitive_good'])
				end
			end

			-- check for system population effect on crew happiness
			-- based on average of recently visited systems
			-- if their civaffinity is high they get good thoughts for visiting busy systems and bad
			-- thoughts for visiting (nearly) empty systems, and vice versa if the civaffinity is low
			if crewMember.civaffinity == 3 then
				if stats.mean_pop > civ_high_threshold then
					crewlife.applyThought(crewMember, crewlife.thoughts['high_civ_good'])
				elseif stats.mean_pop < civ_low_threshold then
					crewlife.applyThought(crewMember, crewlife.thoughts['low_civ_bad'])
				end
			elseif crewMember.civaffinity == 1 then
				if stats.mean_pop > civ_high_threshold then
					crewlife.applyThought(crewMember, crewlife.thoughts['high_civ_bad'])
				elseif stats.mean_pop < civ_low_threshold then
					crewlife.applyThought(crewMember, crewlife.thoughts['low_civ_good'])
				end
			end

			-- check for system exploration status on crew happiness
			-- based on average of recently visited systems
			-- if their civaffinity is high they get bad thoughts if visiting unexplored systems
			-- and vice versa if civaffinity is low
			if crewMember.civaffinity == 3 and stats.explored < explored_threshold then
				crewlife.applyThought(crewMember, crewlife.thoughts['frontier_bad'])
			elseif crewMember.civaffinity == 1 and stats.explored < explored_threshold then
				crewlife.applyThought(crewMember, crewlife.thoughts['frontier_good'])
			end
		end
    end
end
Event.Register("onEnterSystem", crewlife.onEnterSystem)


--
-- Function: onGameStart
--
-- Triggered by event whenever the game is started.
local onGameStart = function ()
	Game.player:GetComponent('CargoManager'):AddListener('crewlife', onPlayerCargoChanged)
	Timer:CallEvery(week_in_secs, decayThoughts)
end
Event.Register("onGameStart", onGameStart)


-- Serializer:Register('CrewLife',serialize,unserialize)

return crewlife
