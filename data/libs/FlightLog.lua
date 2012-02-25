--
-- Class: FlightLog
--
-- A flight log, containing the last systems and stations visited by the
-- player. Can be used by scripts to find out where the player has been
-- recently.


-- default values (private)
local FlightLogSystemQueueLength = 1000
local FlightLogStationQueueLength = 1000

-- private data - the log itself
local FlightLogSystem = {}
local FlightLogStation = {}

FlightLog = {

--
-- Group: Methods
--

--
-- Method: GetSystemPaths
--
-- Returns an iterator returning a SystemPath object for each system visited
-- by the player, backwards in turn, starting with the most recent. If count
-- is specified, returns no more than that many systems.
--
-- iterator = FlightLog.GetSystemPaths(count)
--
-- Parameters:
--
--   count - Optional. The maximum number of systems to return.
--
-- Return:
--
--   iterator - A function which will generate the paths from the log, returning
--              one each time it is called until it runs out, after which it
--              returns nil. It also returns, as a secondary value, the game
--              time at shich the system was left.
--
-- Example:
--
-- Print the names and departure times of the last five systems visited by
-- the player
--
-- > for systemp,deptime in FlightLog.GetSystemPaths(5) do
-- >   print(systemp:GetStarSystem().name, Format.Date(deptime))
-- > end

	GetSystemPaths = function (maximum)
	end,

--
-- Method: GetStationPaths
--
-- Returns an iterator returning a SystemPath object for each station visited
-- by the player, backwards in turn, starting with the most recent. If count
-- is specified, returns no more than that many stations.
--
-- iterator = FlightLog.GetStationPaths(count)
--
-- Parameters:
--
--   count - Optional. The maximum number of systems to return.
--
-- Return:
--
--   iterator - A function which will generate the paths from the log, returning
--              one each time it is called until it runs out, after which it
--              returns nil. It also returns, as a secondary value, the game
--              time at which the player undocked.
--
-- Example:
--
-- Print the names and departure times of the last five stations visited by
-- the player
--
-- > for systemp, deptime in FlightLog.GetStationPaths(5) do
-- >   print(systemp:GetSystemBody().name, Format.Date(deptime))
-- > end

	GetStationPaths = function (maximum)
	end,

--
-- Method: GetPreviousSystemPath
--
-- Returns a SystemPath object that points to the star system where the
-- player was before jumping to this one. If none is on record (such as
-- before any hyperjumps have been made) it returns nil.
--
-- path = FlightLog.GetPreviousSystemPath()
--
-- Return:
--
--   path - a SystemPath object
--
-- Availability:
--
--   alpha 20
--
-- Status:
--
--   experimental
--

	GetPreviousSystemPath = function ()
		if FlightLogSystem[1] then
			return FlightLogSystem[1][1]
		else return nil end
	end,

--
-- Method: GetPreviousStationPath
--
-- Returns a SystemPath object that points to the starport most recently
-- visited. If the player is currently docked, then the starport prior to
-- the present one (which might be the same one, if the player launches
-- and lands in the same port). If none is on record (such as before the
-- player has ever launched) it returns nil.
--
-- path = FlightLog.GetPreviousStationPath()
--
-- Return:
--
--   path - a SystemPath object
--
-- Availability:
--
--   alpha 20
--
-- Status:
--
--   experimental
--

	GetPreviousStationPath = function ()
		if FlightLogStation[1] then
			return FlightLogStation[1][1]
		else return nil end
	end,

}

-- onLeaveSystem
local AddSystemToLog = function (ship)
	if not ship:IsPlayer() then return end
	table.insert(FlightLogSystem,1,{Game.system.path,Game.time})
end

-- onShipUndocked
local AddStationToLog = function (ship, station)
	if not ship:IsPlayer() then return end
	table.insert(FlightLogStation,1,{station.path,Game.time})
end

EventQueue.onLeaveSystem:Connect(AddSystemToLog)
EventQueue.onShipUndocked:Connect(AddStationToLog)
