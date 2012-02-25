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
--   iterator: A function which will generate the paths from the log, returning
--             one each time it is called until it runs out, after which it
--             returns nil.
--
-- Example:
--
-- Print the names of the last five systems visited by the player
--
-- > for systemp in FlightLog.GetSystemPaths(5) do
-- >   print(systemp:GetStarSystem().name)
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
--   iterator: A function which will generate the paths from the log, returning
--             one each time it is called until it runs out, after which it
--             returns nil.
--
-- Example:
--
-- Print the names of the last five stations visited by the player
--
-- > for systemp in FlightLog.GetStationPaths(5) do
-- >   print(systemp:GetSystemBody().name)
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
	end,

}

-- onLeaveSystem
local AddSystemToLog = function (ship)
	if not ship:IsPlayer() then return end
end

-- onShipUndocked
local AddStationToLog = function (ship, station)
	if not ship:IsPlayer() then return end
end

EventQueue.onLeaveSystem:Connect(AddSystemToLog)
EventQueue.onShipUndocked:Connect(AddStationToLog)
