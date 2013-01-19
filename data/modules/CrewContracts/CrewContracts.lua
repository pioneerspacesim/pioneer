-- Copyright © 2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- This module allows the player to hire crew members through BB adverts
-- on stations, and handles periodic events such as their wages.

-- Get the translator function
local t = Translate:GetTranslator()
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

---------------------- Part 1 ----------------------
-- Life aboard ship

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
		{'navigation',crewMember.piloting},
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
		else
			contract.outstanding = contract.outstanding + contract.wage
		end
		
		-- Attempt to pay off any arrears
		local arrears = math.min(Game.player:GetMoney(),contract.outstanding)
		Game.player:AddMoney(0 - arrears)
		contract.outstanding = contract.outstanding - arrears

		-- The crew gain experience each week, and might get better
		boostCrewSkills(crewMember)

		-- Schedule the next pay day, if there is one.
		if contract.payday then
			contract.payday = contract.payday + wage_period
			Timer:CallAt(contract.payday,payWages)
		end
	end

	Timer:CallAt(crewMember.contract.payday,payWages)
end

-- This gets run just after crew are restored from a saved game
Event.Register('crewAvailable',function()
	-- scheduleWages() for everybody
	for crewMember in Game.player:EachCrewMember() do
		scheduleWages(crewMember)
	end
end)

-- This gets run whenever a crew member joins a ship
Event.Register('onJoinCrew',function(ship, crewMember)
	if ship:IsPlayer() then
		scheduleWages(crewMember)
	end
end)

-- This gets run whenever a crew member leaves a ship
Event.Register('onLeaveCrew',function(ship, crewMember)
	if ship:IsPlayer() and crewMember.contract then
		crewMember.contract.payday = nil
	end
end)

---------------------- Part 2 ----------------------
-- The bulletin board

local onChat = function (form)
end

local onCreateBB = function (station)
	-- Only one crew hiring thingy per station
	station:AddAdvert(t('Crew for hire'), onChat)
end

Event.Register("onCreateBB", onCreateBB)
