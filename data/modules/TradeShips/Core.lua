-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local Serializer = require 'Serializer'

-- this module is responsible for storing global variables, logging, serialization
local Core = {
	-- CONSTANTS / SETTINGS
	AU = 149598000000.0, -- meters

	MAX_ROUTE_FLOW = 1.8, -- flow to the most popular station in the system, ships/hour
	MAX_BUSY = 0.3, -- maximum station load
	MAX_SHIPS = 300, -- ~ maximim ships in open space (limitation for performance reasons)
	MIN_STATION_DOCKING_TIME = 2, -- hours, minimum average time of the ship's stay at the station

	WAIT_FOR_NEXT_UNDOCK = 600, -- seconds
	MINIMUM_NEAR_SYSTEMS = 10,
	LOG_SIZE = 2000, -- records in log

	-- for debug info
	last_spawn_interval = nil,
}

--[[
	Property: Core.ships
	(serializable table)
		<key> - object returned from Space:SpawnShip*
		<value>: hashtable
			ship_name - of this ship type; string
			starport - at which this ship intends to dock; SpaceStation object
			dest_time - arrival time from hyperspace; number as Game.kjtime
			dest_path - for hyperspace; SystemPath object, may have body index
			from_path - for hyperspace; SystemPath object
			delay - indicates waiting for a Timer to give next action; number as Game.time
			status - of this ship; string, one of:
				hyperspace - yet to arrive
				hyperspace_out - has departed
				inbound - in system and given AIDockWith order
				docked - currently docked or un/docking
				outbound - heading away from starport before hyperspacing
				fleeing - has been attacked and is trying to get away
				(currently still just following whatever AI order it had)
				cowering - docked after having been attacked, waiting for
				           attacker to go away
				orbit - was unable to dock, heading to or waiting in orbit
			attacker: hashtable
				ref - what this was last attacked by; Body object
			chance - used to determine what action to take when attacked; number
			last_flee - when last action was taken, number as Game.time
			no_jump - whether this has tried to hyperspace away so it only tries once; bool
--]]
Core.ships = nil

--[[
	Property: Core.params
	(non-serializable table - generated every time, accorging to the current system)
		ship_names - list of ship names (shipId) that can actually trade in this system
		total_flow - ship flow from hyperspace, (ships per hour)
		local_routes - available routes lookup table by ship name
			<key> - shipId (see Ship.lua)
			<value>: hashtable
				from - body
				to - body
				distance - from -> to
				ndocks - total docks at destination port
				duration - seconds
				weight - in fact, the probability
				flow - of ships on the route (ship per hour)
				amount - average number of ships on the route
		hyper_routes - available hyperspace routes lookup table by ship name
			<key> - shipId (see Ship.lua)
			<value>: hashtable
				from - system path
				distance - l.y.
				fuel - jump fuel consumption
				duration - jump duration
				cloud_duration - how long before the exit from hyperspace does the cloud appear
				flow - of ships on the route (ship per hour)
				ships - average number of ships on the route
		port_params - station parameters lookup table by it's path
			<key> - body, spacestation
			<value>: hashtable
				flow - of ships to the port (ship per hour)
				ndocks - total docks at destination port
				landed - average number of ships landed
				busy - landed / docks
				time - average duration of a ship parted at a station
		imports, exports - in the current system, indexed array of
		         equipment objects (from the 'Equipment' module),
		         updated by spawnInitialShips
--]]
Core.params = nil

-- circular buffer for log messages
Core.log = {
	clear = function(self)
		self.data = {}
		self.size = Core.LOG_SIZE
		self.p = 0
	end,

	-- ship: object
	-- msg: text
	-- we immediately remember the name and model, because after some time the object may become inaccessible
	add = function(self, ship, msg)
		self.data[self.p%self.size] = { time = Game.time, ship = ship, label = ship and ship:GetLabel(), model = ship and ship:GetShipType(),  msg = msg }
		self.p = self.p + 1
	end,

	-- an iterator acting like ipairs
	-- query - the function that receives row, if the function returns false, the row is skipped
	iter = function(query)
		return function(self)
			local stop = self.p > self.size and self.p % self.size or 0
			if query then
				return function(a, i)
					while i ~= stop do
						i = (i - 1 + a.size) % a.size
						if query(a.data[i]) then return i, a.data[i] end
					end
				end, self, self.p
			else
				return function(a, i)
					if i == stop then return nil
					else
						i = (i - 1 + a.size) % a.size
						return i, a.data[i]
					end
				end, self, self.p
			end
		end
	end
}
Core.log:clear()

-- register of ships added as Core.ships[ship].attacker
do
	local cache = {}
	Core.attackers = {
		add = function(trader, attacker)
			-- we additionally wrap it in a table in order to refer to a link in
			-- the trader, and not to the object of the attacking ship itself
			trader.attacker = { ref = attacker }
			if not cache[attacker] then
				-- make weak values so that they are automatically deleted
				cache[attacker] = setmetatable({ trader.attacker }, { __mode = 'v' })
			else
				-- collect links from all traders who were attacked by this ship
				table.insert(cache[attacker], trader.attacker)
			end
		end,
		-- return true if a link to this ship is in the cache, or clean this
		-- key if there are no links to it left
		check = function(attacker)
			if cache[attacker] then
				if #cache[attacker] > 0 then return true
				else cache[attacker] = nil --if the table is empty - delete it
				end
			end
			return false
		end,
		-- remove all keys that no longer have links
		clean = function()
			for k,v in pairs(cache) do
				if #v == 0 then cache[k] = nil end
			end
		end,
		getCache = function() return cache end -- for debug
	}
end

local serialize = function()
	-- all we need to save is Core.ships, the rest can be rebuilt on load
	-- The serializer will crash if we try to serialize dead objects (issue #3123)
	if Core.ships ~= nil then
		local count = 0
		for k,_ in pairs(Core.ships) do
			if type(k) == 'userdata' and not k:exists() then
				count = count + 1
				-- according to the Lua manual, removing items during iteration with pairs() or next() is ok
				Core.ships[k] = nil
			end
		end
		print('TradeShips: Removed ' .. count .. ' ships before serialization')
	end
	return Core.ships
end

local unserialize = function(data)
	Core.ships = data
	-- made for backward compatibility with old saves
	Core.ships.interval = nil
end

Serializer:Register("TradeShips", serialize, unserialize)

return Core
