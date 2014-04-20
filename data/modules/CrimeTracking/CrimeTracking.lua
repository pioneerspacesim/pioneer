-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Lang = import("Lang")
local Game = import("Game")
local Event = import("Event")
local Comms = import("Comms")
local EquipDef = import("EquipDef")
local Space = import("Space")

local l = Lang.GetResource("module-crimetracking")

-- POLICE string is here:
local lcore = Lang.GetResource("ui-core")

-- Constant. In src/Polit.cpp 100 km is the hard coded range for
-- police intervention.
local lawenforcedRange = 100000

-- table of stations within lawenforcedRange range, to be updated if too old
local nearestStations = {}
local timeLastCheck = 0

local isLawenforced = false

local isWithinLawenforcedRange = function (ship)

	-- Small optimization: don't recheck if two events very close in time
	if Game.time - timeLastCheck < 10 then
		return isLawenforced
	else
		-- return list of all stations within a certain range
		local stations = Space.GetBodies(function (body) return body.superType == 'STARPORT' end)

		timeLastCheck = Game.time
		nearestStations = {}

		for key, station in pairs(stations) do
			local dist = station:DistanceTo(ship)
			if dist <= lawenforcedRange then
				table.insert(nearestStations, station)
				isLawenforced = true
			end
		end

		return isLawenforced
	end
end


local onJettison = function(ship, cargo)
	local lawlessness = Game.system.lawlessness
	if ship:IsPlayer() and isWithinLawenforcedRange(ship) then

		-- check if cargo is unwanted in _any_ of the stations within range
		for k, station in pairs(nearestStations) do
			if station:GetEquipmentPrice(cargo) <= 0 then
				Comms.ImportantMessage(string.interp(l.CRIME_JETTISON, {cargo = EquipDef[cargo].name}), lcore.POLICE)
				local fine = 500*(1 - lawlessness)
				if fine > 0 then
					ship:AddCrime("DUMPING", fine)
					return
				end
			end
		end
    end
end

Event.Register("onJettison", onJettison)
