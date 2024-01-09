-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt


-- Notes:
-- - All station/planet location references in ad/mission are stored as paths for consistency because
--   some can't be accessed as bodies until the player enters the respective system and getting a path from
--   systembody needs to go through body. Variable "location" is a SystemsBody.
-- - Any cargo commodity that needs to be delivered/picked up needs to be intered into "verifyCommodity" function.
--   Some users have issues with commodities not beeing recognized otherwise.

-- TODO:
-- - add degToDegMinSec function to src/LuaFormat.cpp
-- - re-create target ships if player left the system and returns again (works - except for look of ship)
-- - handle situation where player buys a new ship or sells/buys passenger cabins and loaded passengers
--   need to follow into the new ship/cabin


-- Explanations
-- ============
-- Variable flavour.pickup_crew, etc. is the goal, while variable mission.pickup_crew is the meter for
-- actually accomplished parts of the task.
-- Variable mission.pickup_crew_check provides the level of completion:
-- - "NOT"      = not started
-- - "PARTIAL"  = partially completed, but can't do more for now
-- - "ABORT"    = aborted, but can work on it again
-- - "COMPLETE" = finished task


local Engine = require 'Engine'
local Lang = require 'Lang'
local Game = require 'Game'
local Space = require 'Space'
local Comms = require 'Comms'
local Event = require 'Event'
local Mission = require 'Mission'
local Format = require 'Format'
local Serializer = require 'Serializer'
local Character = require 'Character'
local Commodities = require 'Commodities'
local Equipment = require 'Equipment'
local ShipDef = require 'ShipDef'
local Ship = require 'Ship'
local utils = require 'utils'
local Timer = require 'Timer'
local Rand = require 'Rand'
local ModelSkin = require 'SceneGraph.ModelSkin'
local l = Lang.GetResource("module-searchrescue")
local lc = Lang.GetResource 'core'

-- basic variables for mission creation
local max_mission_dist = 30          -- max distance for long distance mission target location [ly]
local max_close_dist = 5000          -- max distance for "CLOSE_PLANET" target location [km]
local max_close_space_dist = 10000   -- max distance for "CLOSE_SPACE" target location [km]
local far_space_orbit_dist = 3.5     -- orbital distance around planet for "FAR_SPACE" target location (number of planet radii)
local min_interaction_dist = 50      -- min distance for successful interaction with target [meters]
local target_interaction_time = 10   -- target interaction time to load/unload one unit of cargo/person [sec]
local max_pass = 20                  -- max number of passengers on target ship
local max_crew = 8                   -- max number of crew on target ship (high max: 8)
local reward_close = 200             -- basic reward for "CLOSE" mission (+/- random half of that)
local reward_medium = 1000           -- basic reward for "MEDIUM" mission (+/- random half of that)
local reward_far = 2000              -- basic reward for "FAR" mission (+/- random half of that)
local ad_freq_max = 3.0              -- maximum frequency for ad creation
local ad_freq_min = 0.3              -- minimum frequency for ad creation

-- global containers and variables
local aircontrol_chars = {}        -- saving specific aircontrol character per spacestation
local ads = {}                     -- currently active ads in the system
local missions = {}                -- accepted missions that are currently active
local leaving_system = false       -- needed for last minute cleanup before leaving system (searchForTarget)
local discarded_ships = {}         -- ships that are not needed any longer

-- Flavour Variables
-- =================

-- Use with caution! There are checks in place but impossible combinations can still break this script.
-- An appropriate ship type will be chosen depending on the mission. Make sure a ship exists that can do
-- what is requested (i.e. has the necessary crew number).

-- id        = 1          : ID to help pull the right ad texts in

-- loctype assumptions       : a) "CLOSE" means there are stations within the system (i.e. target ships don't
--                                necessarily need hyperdrives
--                             b) "PLANET" means any target ship has to have an atmo_shield

-- loctype  = "CLOSE_PLANET" : target on the same planet the player is on (not applicable to non-ground stations)
-- loctype  = "MEDIUM_PLANET": target on a planet within the same system as the player is in
-- loctype  = "CLOSE_SPACE"  : target close to space station player is docked at - but drifting in space
-- loctype  = "FAR_SPACE"    : target in a different system - drifting in space

-- pickup_crew = 0        : how many crew members of the target ship to pick up
-- pickup_pass = 0        : how many passengers of the target ship to pick up
-- pickup_comm = {}       : type and amount of commodity to pick up from target ship

-- deliver_crew = 0       : how many crew members to deliver to target ship
-- deliver_pass = 0       : how many passengers to deliver to target ship
-- deliver_comm = {}      : type and amount of commodity to deliver to target ship

-- urgency   = 1          : factor for how urgent transport ist (most urgent time multiplied by factor)

-- reward_immediate = false : should the mission reward be provided immediately or upon return to station

local flavours = {

	-- rescue all crew + passengers from ship stranded close to starport
	-- SPECIAL: number of crew/pass to pickup is picked randomly during ad creation
	{
		id               = 1,
		loctype          = "MEDIUM_PLANET",
		pickup_crew      = 0,
		pickup_pass      = 0,
		pickup_comm      = {},
		deliver_crew     = 0,
		deliver_pass     = 0,
		deliver_comm     = {},
		urgency          = 1,
		reward_immediate = false,
	},

	-- deliver fuel to ship stranded close to starport
	-- SPECIAL: fuel amount is picked randomly during ad creation
	{
		id               = 2,
		loctype          = "CLOSE_PLANET",
		pickup_crew      = 0,
		pickup_pass      = 0,
		pickup_comm      = {},
		deliver_crew     = 0,
		deliver_pass     = 0,
		deliver_comm     = {},
		urgency          = 5,
		reward_immediate = true
	},

	-- deliver 1 crew to ship stranded close to starport
	{
		id               = 3,
		loctype          = "CLOSE_PLANET",
		pickup_crew      = 0,
		pickup_pass      = 0,
		pickup_comm      = {},
		deliver_crew     = 1,
		deliver_pass     = 0,
		deliver_comm     = {},
		urgency          = 5,
		reward_immediate = true
	},

	-- deliver fuel to ship stranded on planet within same system
	-- SPECIAL: fuel amount is picked randomly during ad creation
	{
		id               = 4,
		loctype          = "MEDIUM_PLANET",
		pickup_crew      = 0,
		pickup_pass      = 0,
		pickup_comm      = {},
		deliver_crew     = 0,
		deliver_pass     = 0,
		deliver_comm     = {},
		urgency          = 5,
		reward_immediate = true
	},

	-- deliver fuel to ship stranded in space close to station player is docked at
	-- SPECIAL: fuel amount is picked randomly during ad creation
	{
		id               = 5,
		loctype          = "CLOSE_SPACE",
		pickup_crew      = 0,
		pickup_pass      = 0,
		pickup_comm      = {},
		deliver_crew     = 0,
		deliver_pass     = 0,
		deliver_comm     = {},
		urgency          = 5,
		reward_immediate = true
	},

	-- rescue all crew + passengers from ship stranded in unoccupied system
	-- SPECIAL: number of crew/pass to pickup is picked randomly during ad creation
	{
		id               = 6,
		loctype          = "FAR_SPACE",
		pickup_crew      = 0,
		pickup_pass      = 0,
		pickup_comm      = {},
		deliver_crew     = 0,
		deliver_pass     = 0,
		deliver_comm     = {},
		urgency          = 5,
		reward_immediate = false
	},

	-- take replacment crew to ship stranded in unoccupied system
	-- SPECIAL: number of crew to deliver is picked randomly during ad creation
	{
		id               = 7,
		loctype          = "FAR_SPACE",
		pickup_crew      = 0,
		pickup_pass      = 0,
		pickup_comm      = {},
		deliver_crew     = 0,
		deliver_pass     = 0,
		deliver_comm     = {},
		urgency          = 5,
		reward_immediate = true
	}
}

-- add strings to flavours
for i = 1,#flavours do
	local f = flavours[i]
	f.adtitle         = l["FLAVOUR_" .. f.id .. "_ADTITLE"]
	f.adtext          = l["FLAVOUR_" .. f.id .. "_ADTEXT"]
	f.introtext       = l["FLAVOUR_" .. f.id .. "_INTROTEXT"]
	f.locationtext    = l["FLAVOUR_" .. f.id .. "_LOCATIONTEXT"]
	f.typeofhelptext  = l["FLAVOUR_" .. f.id .. "_TYPEOFHELPTEXT"]
	f.howmuchtimetext = l["FLAVOUR_" .. f.id .. "_HOWMUCHTIMETEXT"]
	f.successmsg      = l["FLAVOUR_" .. f.id .. "_SUCCESSMSG"]
	f.failuremsg      = l["FLAVOUR_" .. f.id .. "_FAILUREMSG"]
	f.transfermsg     = l["FLAVOUR_" .. f.id .. "_TRANSFERMSG"]
end



-- basic lua helper functions
-- ==========================

local arraySize = function (array)
	-- Return the size (length) of an array that contains arbitrary entries.
	local n = 0
	for _,_ in pairs(array) do n = n + 1 end
	return n
end

local containerContainsKey = function (container, key)
	-- Return true if key is in container and false if not.
	return container[key] ~= nil
end

local copyTable = function (orig)
	-- Return a copy of a table. Copies only the direct children (no deep copy!).
	-- Taken from http://lua-users.org/wiki/CopyTable.
	local orig_type = type(orig)
	local copy
	if orig_type == 'table' then
		copy = {}
		for orig_key, orig_value in pairs(orig) do
			copy[orig_key] = orig_value
		end
	else -- number, string, boolean, etc
		copy = orig
	end
	return copy
end

local compressTableKeys = function (t)
	-- Return the table with all keys in numerical order without gaps.
	-- Taken from http://www.computercraft.info/forums2/index.php?/topic/18380-how-do-i-remove-gaps-in-an-ordered-list/.
	local keySet = {}
	for i in pairs(t) do
		table.insert(keySet, i)
	end

	table.sort(keySet)

	local retVal = {}
	for i = 1, #keySet do
		retVal[i] = t[keySet[i]]
	end

	return retVal
end

-- housekeeping mission functions
-- ==============================

local addMission = function (mission)
	-- Add the supplied mission to the player, generate a unique ID and store this mission within the script.
	table.insert(missions,Mission.New(mission))
end

local removeMission = function (mission)
	-- Remove the supplied mission from the player and from the internal mission storage within the script.
	local ref
	for i,v in pairs(missions) do
		if v == mission then
			ref = i
			break
		end
	end
	mission:Remove()
	missions[ref] = nil
end


-- basic mission functions
-- =======================

local triggerAdCreation = function ()
	-- Return if ad should be created based on lawlessness and min/max frequency values.
	-- Ad number per system is based on how many stations a system has so a player will
	-- be met with a certain number of stations that have one or more ads.
	local stations = Space.GetBodies("SpaceStation")
	local freq = Game.system.lawlessness * ad_freq_max
	if freq < ad_freq_min then freq = ad_freq_min end
	local ad_num_max = freq * #stations
	if arraySize(ads) < ad_num_max then
		if Engine.rand:Integer(0,1) == 1 then
			return true
		end
	end
	return false
end

local getNumberOfFlavours = function (str)
	-- Returns the number of flavours of the given string (assuming first flavour has suffix '_1').
	-- Taken from CargoRun.lua.
	local num = 1
	while l:get(str .. "_" .. num) do
		num = num + 1
	end
	return num - 1
end

local mToAU = function (meters)
	-- Transform meters into AU.
	return meters/149598000000
end

local splitName = function (name)
	-- Splits the supplied name into first and last name and returns a table of both separately.
	-- Idea from http://stackoverflow.com/questions/2779700/lua-split-into-words.
	local names = {}
	for word in name:gmatch("%w+") do table.insert(names, word) end
	return names
end

local decToDegMinSec = function (coord_orig)
	-- Converts geographic coordinates from decimal to degree/minutes/seconds format
	-- and returns a string.
	local coord = math.abs(coord_orig)
	local degrees = math.floor(coord)
	local minutes = math.floor(60*(coord - degrees))
	local seconds = math.floor(3600 * ((coord - degrees) - minutes / 60))
	if coord_orig < 0 then degrees = degrees * -1 end
	local str = string.format("%i° %i' %i\"", degrees, minutes, seconds)
	return str
end

local getAircontrolChar = function (station)
	-- Get the correct aircontrol character for the supplied station. If it does not exist
	-- create one and store it.
	if containerContainsKey(aircontrol_chars, station.path) then
		return aircontrol_chars[station.path]
	else
		local char = Character.New()
		aircontrol_chars[station.path] = char
		return char
	end
end

local randomLatLong = function (station)
	-- Provide a set of random latitude and longitude coordinates for ship placement that are:
	-- (a) random, within max_close_dist from starting base if base is provided, or
	-- (b) completely random.
	local lat, long, dist

	-- calc new lat/lon based on distance and bearing
	-- formulas taken from http://www.movable-type.co.uk/scripts/latlong.html
	if station then
		local old_lat, old_long = station:GetGroundPosition()
		local planet_radius = station.path:GetSystemBody().parent.radius / 1000
		local bearing = math.rad(Engine.rand:Number(0,360))
		dist = Engine.rand:Integer(1,max_close_dist)  -- min distance is 1 km
		lat = math.asin(math.sin(old_lat) * math.cos(dist/planet_radius)
			+ math.cos(old_lat) * math.sin(dist/planet_radius) * math.cos(bearing))
		long = old_long + math.atan2(
			math.sin(bearing) * math.sin(dist/planet_radius) * math.cos(old_lat),
			math.cos(dist/planet_radius) - math.sin(old_lat) * math.sin(lat))
		dist = dist * 1000  -- convert to m for downstream consistency
	else
		lat = Engine.rand:Number(-90,90)
		lat = math.rad(lat)
		long = Engine.rand:Number(-180,180)
		long = math.rad(long)
	end
	return lat, long, dist
end

local shipdefFromName = function (shipdef_name)
	-- Return the corresponding shipdef for the supplied shipdef name. Necessary because serialization
	-- crashes if actual shipdef is stored in ad. There may be a smarter way to do this!
	local shipdefs = utils.build_array(utils.filter(function (_,def) return def.tag == 'SHIP'
			                                   and def.name == shipdef_name end, pairs(ShipDef)))
	return shipdefs[1]
end

local crewPresent = function (ship)
	-- Check if any crew is present on the ship.
	if ship:CrewNumber() > 0 then
		return true
	else
		return false
	end
end

local passengersPresent = function (ship)
	-- Check if any passengers are present on the ship.
	if ship:CountEquip(Equipment.misc.cabin_occupied) > 0 then
		return true
	else
		return false
	end
end

local passengerSpace = function (ship)
	-- Check if the ship has space for passengers.
	if ship:CountEquip(Equipment.misc.cabin) > 0 then
		return true
	else
		return false
	end
end

local cargoPresent = function (ship, item)
	-- Check if this cargo item is present on the ship.
	return ship:GetComponent('CargoManager'):CountCommodity(item) > 0
end

local cargoSpace = function (ship)
	-- Check if the ship has space for additional cargo.
	return ship:GetComponent('CargoManager'):GetFreeSpace() > 0
end

local addCrew = function (ship, crew_member)
	-- Add a crew member to the supplied ship.
	if ship:CrewNumber() == ship.maxCrew then return end
	if not crew_member then
		crew_member = Character.New()
	end
	ship:Enroll(crew_member)
end

local removeCrew = function (ship)
	-- Remove a crew member from the supplied ship.
	if ship:CrewNumber() == 0 then return end
	local crew_member
	for member in ship:EachCrewMember() do  -- only way to get a single crew member?
		crew_member = member
		break
	end
	ship:Dismiss(crew_member)
	return crew_member
end

local addPassenger = function (ship)
	-- Add a passenger to the supplied ship.
	if not passengerSpace(ship) then return end
	ship:RemoveEquip(Equipment.misc.cabin, 1)
	ship:AddEquip(Equipment.misc.cabin_occupied, 1)
end

local removePassenger = function (ship)
	-- Remove a passenger from the supplied ship.
	if not passengersPresent(ship) then return end
	ship:RemoveEquip(Equipment.misc.cabin_occupied, 1)
	ship:AddEquip(Equipment.misc.cabin, 1)
end

local addCargo = function (ship, item)
	-- Add a ton of the supplied cargo item to the ship.
	ship:GetComponent('CargoManager'):AddCommodity(item, 1)
end

local removeCargo = function (ship, item)
	-- Remove a ton of the supplied cargo item from the ship.
	ship:GetComponent('CargoManager'):RemoveCommodity(item, 1)
end

local passEquipmentRequirements = function (requirements)
	-- Check if player ship passes equipment requirements for this mission.
	if requirements == {} then return true end
	for equipment,amount in pairs(requirements) do
		if Game.player:CountEquip(equipment) < amount then return false end
	end
	return true
end

local isQualifiedFor = function(ad)
	-- Return if player is qualified for this mission. Used for example by ads to determine if they are
	-- enabled or disabled for selection on the banter board of if certain missions can be accepted.
	-- TODO: enable reputation based qualifications

	-- collect equipment requirements per mission flavor
	local requirements = {}
	local empty_cabins = ad.pickup_crew + ad.deliver_crew + ad.pickup_pass + ad.deliver_pass
	if empty_cabins > 0 then requirements[Equipment.misc.cabin] = empty_cabins end
	if not passEquipmentRequirements(requirements) then return false end
	return true
end

-- extended mission functions
-- ==========================

local calcReward = function (flavour, pickup_crew, pickup_pass, pickup_comm, deliver_crew, deliver_pass, deliver_comm)
	-- Calculate the appropriate reward for this mission.
	-- TODO: Extend for other mission types.
	-- TODO: Adjust based on urgency + risk?
	local reward
	if flavour.loctype == "CLOSE_PLANET" or flavour.loctype == "CLOSE_SPACE" then
		reward = reward_close + Engine.rand:Number(reward_close / 2 * -1, reward_close / 2)
	elseif flavour.loctype == "MEDIUM_PLANET" then
		reward = reward_medium + Engine.rand:Number(reward_medium / 2 * -1, reward_medium / 2)
	elseif flavour.loctype == "FAR_SPACE" then
		reward = reward_far + Engine.rand:Number(reward_far / 2 * -1, reward_far / 2)
	end

	-- factor in personnel to be delivered or picked up
	local personnel = pickup_crew + pickup_pass + deliver_crew + deliver_pass
	if personnel > 0 then
		reward = reward + (personnel * (Equipment.misc.cabin.price * 0.5))
	end

	-- factor in commodities to be delivered or picked up
	if pickup_comm ~= {} then
		local extra = 0
		for commodity,amount in pairs(pickup_comm) do
			extra = extra + (commodity.price * amount * 10)
		end
		reward = reward + extra
	end
	if deliver_comm ~= {} then
		local extra = 0
		for commodity,amount in pairs(deliver_comm) do
			extra = extra + (commodity.price * amount * 10)
		end
		reward = reward + extra
	end

	reward = math.ceil(reward)
	return reward
end

local createTargetShipParameters = function (flavour, planet)
	-- Create the basic parameters for the target ship. It is important to set these before ad creation
	-- so certain info can be included in the ad text. The actual ship is created once the mission has
	-- been accepted.

	-- pick unique seed for ship and set rand
	local seed = Engine.rand:Integer(1,1000000)
	local rand = Rand.New(seed)

	-- pick appropriate hull type
	local shipdefs = utils.build_array(utils.filter(function (_,def) return def.tag == 'SHIP'
	                                                end, pairs(ShipDef)))

	----> no police ships or other non-buyable ships
	for i,shipdef in pairs(shipdefs) do
		if shipdef.basePrice == 0 then shipdefs[i] = nil end
	end
	----> hyperdrive mandatory (for clean exiting of ships)
	for i,shipdef in pairs(shipdefs) do
		if shipdef.equipSlotCapacity.engine == 0 then shipdefs[i] = nil end
	end
	----> atmo-shield if ship stranded on planet
	if flavour.loctype == "CLOSE_PLANET" or flavour.loctype == "MEDIUM_PLANET" then
		for i,shipdef in pairs(shipdefs) do
			-- make sure that this ship model has atmosferic shield capacity and can take off from the planet
			-- e.g. Natrix with some fuel cant take off from Earth ..
			local fullMass = shipdef.hullMass + shipdef.capacity + shipdef.fuelTankMass
			local UpAccelFull  =  math.abs(shipdef.linearThrust.UP / (1000*fullMass))
			if shipdef.equipSlotCapacity.atmo_shield == 0 or planet:GetSystemBody().gravity > UpAccelFull then
				shipdefs[i] = nil
			end
		end
	end
	----> crew quarters for crew delivery missions
	if flavour.id == 7 then
		for i,shipdef in pairs(shipdefs) do
			if shipdef.maxCrew < 2 or shipdef.minCrew < 2 then shipdefs[i] = nil end
		end
	elseif flavour.deliver_crew > 0 then
		for i,shipdef in pairs(shipdefs) do
			if shipdef.maxCrew < flavour.deliver_crew+1 then shipdefs[i] = nil end
		end
	end
	----> crew quarters for crew pickup missions
	if flavour.pickup_crew > 0 then
		for i,shipdef in pairs(shipdefs) do
			if shipdef.maxCrew < flavour.pickup_crew then shipdefs[i] = nil end
		end
	end
	----> cargo space for passenger pickup missions
	---- (this is just an estimate to make sure enough space remains after
	---- loading drive, weapons etc.
	if flavour.id == 1 or flavour.id == 6 then
		for i,shipdef in pairs(shipdefs) do

			-- get mass of hyperdrive if this ship has a default drive
			-- if no default drive assume lowest mass drive
			-- higher mass drives will only be fitted later at ship creation if capacity is huge
			local drive = Equipment.hyperspace['hyperdrive_'..tostring(shipdef.hyperdriveClass)]
			if not drive then
				local drives = {}
				for i = 9, 1, -1 do
					table.insert(drives, Equipment.hyperspace['hyperdrive_'..tostring(i)])
				end
				table.sort(drives, function (a,b) return a.capabilities.mass < b.capabilities.mass end)
				drive = drives[1]
			end
			if (shipdef.capacity - drive.capabilities.mass - drive.capabilities.hyperclass^2 ) < 2
				or shipdef.equipSlotCapacity.cargo < drive.capabilities.hyperclass^2
				or shipdef.equipSlotCapacity.cabin == 0 then
				shipdefs[i] = nil
			end
		end
	end

	if arraySize(shipdefs) == 0 then
		print("Could not find appropriate ship type for this mission!")
		return
	end

	-- 1. compress table keys to eliminate 'nil' entries
	-- 2. sort shipdefs by name so the list has the same order every time
	-- utils.build_array returns the list with random order
	shipdefs = compressTableKeys(shipdefs)
	table.sort(shipdefs, function (a,b) return a.name < b.name end)
	local shipdef = shipdefs[rand:Integer(1,#shipdefs)]

	-- number of crew
	local crew_num=0
	local pickup_crew=0
	local deliver_crew=0
	if flavour.id == 1 or flavour.id == 6 then
		crew_num = rand:Integer(shipdef.minCrew,shipdef.maxCrew)
		pickup_crew = crew_num
		deliver_crew = flavour.deliver_crew
	elseif flavour.id == 7 then
		if shipdef.maxCrew == 2 then
			crew_num = 1
		else
			crew_num = rand:Integer(1,shipdef.minCrew-1)
		end
		pickup_crew = flavour.pickup_crew
		deliver_crew = shipdef.minCrew - crew_num
	elseif flavour.deliver_crew > 0 then
		crew_num = rand:Integer(flavour.deliver_crew+1,shipdef.maxCrew)
		crew_num = crew_num - flavour.deliver_crew
		deliver_crew = flavour.deliver_crew
	elseif flavour.pickup_crew > 0 then
		crew_num = flavour.pickup_crew
		pickup_crew = flavour.pickup_crew
	else
		crew_num = rand:Integer(shipdef.minCrew,shipdef.maxCrew)
		pickup_crew = flavour.pickup_crew
		deliver_crew = flavour.deliver_crew
	end

	-- determine passengers
	local pickup_pass
	if flavour.id == 1 or flavour.id == 6 then
		local any_pass = rand:Integer(0,1)
		if any_pass > 0 then
			local drive = Equipment.hyperspace['hyperdrive_'..tostring(shipdef.hyperdriveClass)]
			if not drive then
				local drives = {}
				for i = 9, 1, -1 do
					table.insert(drives, Equipment.hyperspace['hyperdrive_'..tostring(i)])
				end
				table.sort(drives, function (a,b) return a.capabilities.mass < b.capabilities.mass end)
				drive = drives[1]
			end
			--after drive, hyper fuel, atmo shield, laser
			local max_cabins = shipdef.capacity - drive.capabilities.mass - drive.capabilities.hyperclass^2 - 2
			max_cabins = math.min(max_cabins, shipdef.equipSlotCapacity.cabin)
			pickup_pass = rand:Integer(1, math.min(max_cabins, max_pass))
		else
			pickup_pass = 0
		end
	else
		pickup_pass = flavour.pickup_pass
	end

	-- label
	local shiplabel = Ship.MakeRandomLabel()

	return shipdef, crew_num, shiplabel, pickup_crew, pickup_pass, deliver_crew, seed
end

local createTargetShip = function (mission)
	-- Create the target ship to be search for.
	local ship
	local shipdef = shipdefFromName(mission.shipdef_name)

	-- set rand with unique ship seed
	local rand = Rand.New(mission.shipseed)

	-- create ship
	if mission.flavour.loctype == "CLOSE_PLANET" then
		ship = Space.SpawnShipLanded(shipdef.id, Space.GetBody(mission.planet_target.bodyIndex), mission.lat, mission.long)
	elseif mission.flavour.loctype == "MEDIUM_PLANET" then
		ship = Space.SpawnShipLanded(shipdef.id, Space.GetBody(mission.planet_target.bodyIndex), mission.lat, mission.long)
	elseif mission.flavour.loctype == "CLOSE_SPACE" then
		ship = Space.SpawnShipNear(shipdef.id, Space.GetBody(mission.station_target.bodyIndex), mission.dist/1000, mission.dist/1000)
	elseif mission.flavour.loctype == "FAR_SPACE" then
		local planet_body = Space.GetBody(mission.planet_target.bodyIndex)
		local orbit_radius = planet_body:GetPhysicalRadius() * far_space_orbit_dist
		ship = Space.SpawnShipOrbit(shipdef.id, planet_body, orbit_radius, orbit_radius)
	end

	-- set ship looks (label, skin, pattern)
	ship:SetLabel(mission.shiplabel)
	local skin = ModelSkin.New():SetRandomColors(rand):SetDecal(shipdef.manufacturer)
	ship:SetSkin(skin)
	local model = Engine.GetModel(shipdef.modelName)
	local pattern
	if model.numPatterns <= 1 then
		pattern = 0
	else
		local pattern = rand:Integer(0,model.numPatterns-1)
	end
	ship:SetPattern(pattern)

	-- load a hyperdrive
	-- 1st try: default drive for this ship class
	-- 2nd try: largest drive possible that doesn't take more than a 10th of available room
	-- fallback: smallest drive
	local drives = {}
	local drive = Equipment.hyperspace['hyperdrive_'..tostring(shipdef.hyperdriveClass)]
	if not drive then
		for i = 9, 1, -1 do
			table.insert(drives, Equipment.hyperspace['hyperdrive_'..tostring(i)])
		end
		table.sort(drives, function (a,b) return a.capabilities.mass < b.capabilities.mass end)
		for i = #drives, 1, -1 do
			local test_drive = drives[i]
			if shipdef.capacity / 10 > test_drive.capabilities.mass then
				drive = test_drive
			end
		end
	end
	if not drive then drive = drives[1] end
	ship:AddEquip(drive)

	-- load passengers
	if mission.pickup_pass > 0 then
		ship:AddEquip(Equipment.misc.cabin_occupied, mission.pickup_pass)
	end

	-- add hydrogen for hyperjumping even for refueling missoins - to reserve the space
	-- for refueling missions it is removed later
	local drive = ship:GetEquip('engine', 1)
	local hypfuel = drive.capabilities.hyperclass ^ 2  -- fuel for max range
	ship:GetComponent('CargoManager'):AddCommodity(drive.fuel or Commodities.hydrogen, hypfuel)

	-- load crew
	for _ = 1, mission.crew_num do
		ship:Enroll(Character.New())
	end

	-- load atmo_shield
	if shipdef.equipSlotCapacity.atmo_shield ~= 0 then
		ship:AddEquip(Equipment.misc.atmospheric_shielding)
	end

	-- load a laser
	local max_laser_size = ship.freeCapacity
	local laserdefs = utils.build_array(utils.filter(function (_,laser)
		return laser:IsValidSlot('laser_front')
			and laser.capabilities.mass <= max_laser_size
			and laser.l10n_key:find("PULSECANNON")
		end, pairs(Equipment.laser) ))
	local laserdef = laserdefs[rand:Integer(1,#laserdefs)]
	ship:AddEquip(laserdef)

	-- remove all fuel for refueling mission
	if mission.flavour.id == 2 or mission.flavour.id == 4 or mission.flavour.id == 5 then
		ship:SetFuelPercent(0)
		ship:GetComponent('CargoManager'):RemoveCommodity(drive.fuel or Commodities.hydrogen, hypfuel)
	end

	return ship
end

local onChat = function (form, ref, option)
	-- Show ad on the banter board.
	local ad = ads[ref]
	form:Clear()

	if option == -1 then
		form:Close()
		return
	end

	form:SetFace(ad.client)

	form:AddNavButton(ad.location)

	--   TODO: work out a better system for equipment qualification check
	--   local qualified = isQualifiedFor(ad)
	--   if not qualified then
	--      local denytext = string.interp(l.EQUIPMENT, {cabins = ad.pickup_crew + ad.deliver_crew +
	--						      ad.pickup_pass + ad.deliver_pass})
	--      form:SetMessage(denytext)
	--      return
	--   end

	if option == 0 then  -- repeat original request
		local introtext = string.interp(ad.flavour.introtext, {
			name         = ad.client.name,
			entity       = ad.entity,
			problem      = ad.problem,
			cash         = Format.Money(ad.reward),
			ship         = ad.shipdef_name,
			starport     = ad.station_local:GetSystemBody().name,
			shiplabel    = ad.shiplabel,
			planet       = ad.planet_target:GetSystemBody().name,
			crew         = ad.crew_num,
		})
		form:SetMessage(introtext)

	elseif option == 1 then  -- where is the target
		local dist
		if ad.flavour.loctype == "CLOSE_PLANET" or ad.flavour.loctype == "CLOSE_SPACE" then
			dist = string.format("%.0f", ad.dist/1000)
		else
			dist = string.format("%.2f", ad.dist)
		end

		local locationtext = string.interp(ad.flavour.locationtext, {
			starport     = ad.station_local:GetSystemBody().name,
			shiplabel    = ad.shiplabel,
			system       = ad.system_target:GetStarSystem().name,
			sectorx      = ad.system_target.sectorX,
			sectory      = ad.system_target.sectorY,
			sectorz      = ad.system_target.sectorZ,
			dist         = dist,
			lat          = decToDegMinSec(math.rad2deg(ad.lat)),
			long         = decToDegMinSec(math.rad2deg(ad.long)),
			planet       = ad.planet_target:GetSystemBody().name
		})
		form:SetMessage(locationtext)

	elseif option == 2 then  -- type of help needed

		-- pick cargo/units to be delivered
		-- TODO: currently, this only works if only one cargo type is delivered (not multiple types or cargo pickup)
		local unit, cargo
		if ad.deliver_comm ~= {} then
			for cargo_obj,cargo_unit in pairs(ad.deliver_comm) do
				cargo = cargo_obj:GetName()
				unit = cargo_unit
				break
			end
		end

		local typeofhelptext = string.interp(ad.flavour.typeofhelptext, {
			starport     = ad.station_local:GetSystemBody().name,
			crew         = ad.crew_num,
			pass         = ad.pickup_pass,
			deliver_crew = ad.deliver_crew,
			unit         = unit,
			cargo        = cargo,
			high_gravity = ad.high_gravity,
		})
		form:SetMessage(typeofhelptext)

	elseif option == 3 then  -- how much time
		local howmuchtimetext = string.interp(ad.flavour.howmuchtimetext, {due = Format.Date(ad.due)})
		form:SetMessage(howmuchtimetext)

	elseif option == 5 then  -- agree to mission

		-- TODO: work out a better system for equipment qualification check (add cargo space check + reputation)
		local qualified = isQualifiedFor(ad)
		if not qualified then
			local cabins = ad.pickup_crew + ad.deliver_crew + ad.pickup_pass + ad.deliver_pass
			local denytext = string.interp(l.EQUIPMENT, {unit = cabins, equipment = l.UNOCCUPIED_PASSENGER_CABINS})
			form:SetMessage(denytext)
			form:RemoveNavButton()
			return
		end

		form:RemoveAdvertOnClose()
		ads[ref] = nil

		local mission = {
			-- these variables are hardcoded and need to be filled
			type	            = "searchrescue",
			client             = ad.client,
			location           = ad.location,
			due                = ad.due,
			reward             = ad.reward,
			status             = "ACTIVE",

			-- these variables are script specific
			station_local      = ad.station_local,
			planet_local       = ad.planet_local,
			system_local       = ad.system_local,
			station_target     = ad.station_target,
			planet_target      = ad.planet_target,
			system_target      = ad.system_target,
			entity             = ad.entity,
			problem            = ad.problem,
			dist               = ad.dist,
			flavour            = ad.flavour,
			target             = nil,
			lat                = ad.lat,
			long               = ad.long,
			shipdef_name       = ad.shipdef_name,
			shiplabel          = ad.shiplabel,
			crew_num           = ad.crew_num,
			shipseed           = ad.shipseed,

			-- "..._orig" => original variables from ad
			pickup_crew_orig   = ad.pickup_crew,
			pickup_pass_orig   = ad.pickup_pass,
			pickup_comm_orig   = copyTable(ad.pickup_comm),
			deliver_crew_orig  = ad.deliver_crew,
			deliver_pass_orig  = ad.deliver_pass,
			deliver_comm_orig  = copyTable(ad.deliver_comm),

			-- variables are changed based on completion status
			pickup_crew        = ad.pickup_crew,
			pickup_pass        = ad.pickup_pass,
			pickup_comm        = copyTable(ad.pickup_comm),
			deliver_crew       = ad.deliver_crew,
			deliver_pass       = ad.deliver_pass,
			deliver_comm       = copyTable(ad.deliver_comm),

			pickup_crew_check  = "NOT",
			pickup_pass_check  = "NOT",
			pickup_comm_check  = {},
			deliver_crew_check = "NOT",
			deliver_pass_check = "NOT",
			deliver_comm_check = {},
			target_destroyed   = "NOT",
			cargo_pass         = {},
			cargo_comm         = {},
			searching          = false    -- makes sure only one search is active for this mission (function "searchForTarget")
		}

		-- create target ship if in the same systems, otherwise create when jumping there
		if mission.flavour.loctype ~= "FAR_SPACE" then
			mission.target = createTargetShip(mission)
		end

		-- load crew/passenger
		if ad.deliver_crew > 0 then
			for _ = 1, ad.deliver_crew do
				local passenger = Character.New()
				addPassenger(Game.player)
				table.insert(mission.cargo_pass, passenger)
			end
		end
		if ad.deliver_pass > 0 then
			for _ = 1, ad.deliver_pass do
				local passenger = Character.New()
				addPassenger(Game.player)
				table.insert(mission.cargo_pass, passenger)
			end
		end

		form:SetMessage(l.THANK_YOU_ACCEPTANCE_TXT)
		addMission(mission)

		-- setup navbutton target (ships in other system don't exist yet!)
		local navbutton_target
		if mission.planet_target:IsSameSystem(Game.system.path) then
			navbutton_target = mission.target
		else
			navbutton_target = mission.planet_target
		end
		form:AddNavButton(navbutton_target)
		return
	end

	form:AddOption(l.WHERE_IS_THE_TARGET, 1)
	form:AddOption(l.TYPE_OF_HELP, 2)
	form:AddOption(l.HOW_MUCH_TIME, 3)
	form:AddOption(l.COULD_YOU_REPEAT_THE_ORIGINAL_REQUEST, 0)
	form:AddOption(l.OK_AGREED, 5)
end

local onDelete = function (ref)
	ads[ref] = nil
end

local isEnabled = function (ref)
	-- Called from within ad to enable or disable it for selecton on the banter board.
	-- TODO: possibly integrate reputation based qualification filter
	return true
end

local findNearbyStations = function (vacuum, body)
	-- Return a list with stations within this system sorted by distance from supplied body (ascending). If vacuum is set to true
	-- then only return orbital stations

	-- get station bodies within current system depending on vacuum variable
	local nearbystations_raw
	if vacuum == true then
		nearbystations_raw = utils.filter_array(Space.GetBodies("SpaceStation"), function (body)
			return body.type == 'STARPORT_ORBITAL' or (not body.path:GetSystemBody().parent.hasAtmosphere)
		end)
	else
		nearbystations_raw = Space.GetBodies("SpaceStation")
	end

	-- determine distance to body
	local nearbystations_dist = {}
	for _,station in pairs(nearbystations_raw) do
		if station ~= body then
			local dist = body:DistanceTo(station)
			table.insert(nearbystations_dist, {station, dist})
		end
	end

	-- sort stations by distance to body (ascending)
	local nearbystations = {}
	table.sort(nearbystations_dist, function (a,b) return a[2] < b[2] end)
	for _,data in ipairs(nearbystations_dist) do
		table.insert(nearbystations, data[1].path)
	end

	return nearbystations
end

local findClosestPlanets = function ()
	-- Return dictionary of stations with the corresponding rocky planets they are closer
	-- to than any other station. Planets with a station are excluded.

	-- get rocky planets
	local rockyplanets = {}
	for _,path in pairs(Game.system:GetBodyPaths()) do
		local sbody = path:GetSystemBody()
		if sbody.superType == "ROCKY_PLANET" then
			table.insert(rockyplanets, Space.GetBody(sbody.index))
		end
	end

	-- get planets with stations and remove from planet list
	local ground_stations = utils.filter_array(Space.GetBodies("SpaceStation"), function (body) return body.type == 'STARPORT_SURFACE' end)
	for _,ground_station in pairs(ground_stations) do
		for i=#rockyplanets, 1, -1 do
			if rockyplanets[i] == Space.GetBody(ground_station.path:GetSystemBody().parent.index) then
				table.remove(rockyplanets, i)
				break
			end
		end
	end

	-- create dictionary of stations
	local closestplanets = {}
	local stations = Space.GetBodies("SpaceStation")
	for _,station in pairs(stations) do closestplanets[station] = {} end

	-- pick closest planets to stations
	for _,planet in pairs(rockyplanets) do
		local nearest_station = planet:FindNearestTo("SPACESTATION")
		table.insert(closestplanets[nearest_station], planet.path)
	end

	return closestplanets
end

local findNearbySystems = function (with_stations)
	-- Return list of systems within max_mission_dist distance and sorted by distance from player system (ascending).

	-- get systems (either inhabited or not - depending on variable with_stations)
	local nearbysystems_raw
	if with_stations == true then
		nearbysystems_raw = Game.system:GetNearbySystems(max_mission_dist, function (s) return #s:GetStationPaths() > 0 end)
	else
		nearbysystems_raw = Game.system:GetNearbySystems(max_mission_dist, function (s) return #s:GetStationPaths() == 0 end)
	end

	-- determine distance to player system
	local nearbysystems_dist = {}
	for _,system in pairs(nearbysystems_raw) do
		local dist = Game.system:DistanceTo(system)
		table.insert(nearbysystems_dist, {system, dist})
	end

	-- sort systems by distance to player system (ascending)
	local nearbysystems = {}
	table.sort(nearbysystems_dist, function (a,b) return a[2] < b[2] end)
	for _,data in ipairs(nearbysystems_dist) do
		table.insert(nearbysystems, data[1].path)
	end
	return nearbysystems
end

local randomPlanet = function (system)
	-- Return random planet located in the provided system.
	local planets = {}
	local paths = system:GetBodyPaths()
	for _,path in pairs(paths) do
		local sbody = path:GetSystemBody()
		local supertype = sbody.superType
		if supertype == "ROCKY_PLANET" or supertype == "GAS_GIANT" then
			table.insert(planets, path)
		end
	end
	if #planets > 0 then return planets[Engine.rand:Integer(1,#planets)] else return nil end
end

local flyToNearbyStation =  function (ship)
	-- Fly the supplied ship to the closest station. If no station is in the system then jump to the
	-- closest system that does have a station.

	-- check if ship has atmo shield and limit to vacuum starports if not
	local vacuum = false
	if ship:CountEquip(Equipment.misc.atmospheric_shielding) == 0 then
		vacuum = true
	end

	local nearbysystems
	local nearbystations = findNearbyStations(vacuum, ship)

	if #nearbystations > 0 then

		-- add ship to discard pile to jump away later
		table.insert(discarded_ships, ship)

		-- blast off ship if LANDED
		if ship.flightState == "LANDED" then
			ship:AIEnterLowOrbit(ship:FindNearestTo("PLANET") or ship:FindNearestTo("STAR"))
			Timer:CallAt(Game.time + 5, function () ship:AIDockWith(Space.GetBody(nearbystations[1].bodyIndex)) end)
		else
			ship:AIDockWith(Space.GetBody(nearbystations[1].bodyIndex))
		end
	else
		local with_stations = true
		nearbysystems = findNearbySystems(with_stations)

		-- blast off ship if LANDED, otherwise hyp away directly
		if #nearbysystems > 0 then
			if ship.flightState == "LANDED" then
				ship:AIEnterLowOrbit(ship:FindNearestTo("PLANET") or ship:FindeNearestTo("STAR"))
			end
			Timer:CallAt(Game.time + 5, function () ship:HyperjumpTo(nearbysystems[1]) end)
		else
			return
		end
	end
end

local discardShip = function (ship)
	-- Gracefully discard ship that is not needed any longer for the ship by either:
	-- 1. hyperjumping to populated system, or
	-- 2. hyperjumping to non-populated system, or
	-- 3. fly to high orbit and explode.
	local with_stations = true
	local nearbysystems = findNearbySystems(with_stations)
	local status, distance, fuel, duration = ship:GetHyperspaceDetails(Game.system.path, nearbysystems[1])
	if #nearbysystems > 0 and status == "OK" then
		Timer:CallAt(Game.time + Engine.rand:Integer(5,10), function ()
			ship:AIEnterLowOrbit(ship:FindNearestTo("PLANET") or ship:FindNearestTo("STAR"))
			Timer:CallAt(Game.time + 30, function () ship:HyperjumpTo(nearbysystems[1]) end)
		end)
	else
		with_stations = false
		nearbysystems = findNearbySystems(with_stations)
		status, distance, fuel, duration = ship:GetHyperspaceDetails(Game.system.path, nearbysystems[1])
		if #nearbysystems > 0 and status == "OK" then
			Timer:CallAt(Game.time + Engine.rand:Integer(5,10), function ()
				ship:AIEnterLowOrbit(ship:FindNearestTo("PLANET") or ship:FindNearestTo("STAR"))
				Timer:CallAt(Game.time + 30, function () ship:HyperjumpTo(nearbysystems[1]) end)
			end)
		else
			Timer:CallAt(Game.time + Engine.rand:Integer(5,10), function ()
				ship:AIEnterHighOrbit(ship:FindNearestTo("PLANET") or ship:FindNearestTo("STAR"))
				Timer:CallAt(Game.time + 600, function () ship:Explode() end)
			end)
		end
	end
end

local placeAdvert = function (station, ad)
	local starport_label, planet_label, system_label
	if ad.station_target then starport_label = ad.station_target:GetSystemBody().name else starport_label = nil end
	if ad.planet_target then planet_label = ad.planet_target:GetSystemBody().name else planet_label = nil end
	if ad.system_target then system_label = ad.system_target:GetStarSystem().name else system_label = nil end
	local desc = string.interp(ad.flavour.adtext, {
		starport = starport_label,
	    planet = planet_label,
	    system = system_label
	})

	local ref = station:AddAdvert({
		title       = ad.flavour.adtitle,
		description = desc,
		icon        = "searchrescue",
		due         = ad.due,
		reward      = ad.reward,
		location    = ad.location,
		onChat      = onChat,
		onDelete    = onDelete,
		isEnabled   = isEnabled
	})
	ads[ref] = ad
end

local makeAdvert = function (station, manualFlavour, closestplanets)
	-- Make a single advertisement for the bulletin board of the supplied station.
	local due, dist, client, entity, problem, location, high_gravity
	local lat = 0
	local long = 0

	-- set flavour (manually if a second arg is given)
	local flavour = flavours[manualFlavour] or flavours[Engine.rand:Integer(1,#flavours)]

	-- abort if flavour incompatible with space station type
	if flavour.loctype == "CLOSE_PLANET" and station.isGroundStation == false then return end
	if flavour.loctype == "CLOSE_SPACE" and station.isGroundStation == true then return end

	local urgency = flavour.urgency

	-- TODO: this will have to be adjusted for missions outside of BBS announcements
	local station_local = station.path
	local planet_local = Space.GetBody(station_local:GetSystemBody().parent.index).path
	local system_local = Game.system.path

	local station_target, planet_target, system_target
	if flavour.loctype == "CLOSE_PLANET" then
		station_target = station_local
		planet_target = planet_local
		system_target = system_local
		location = planet_target
		lat, long, dist = randomLatLong(station)
		due = 60 * 60 * Engine.rand:Number(2,24)        --TODO: adjust due date based on urgency

	elseif flavour.loctype == "MEDIUM_PLANET" then
		station_target = nil

		-- pick an uninhabited, rocky planet this station is closer to than any other station
		if not closestplanets then closestplanets = findClosestPlanets() end
		if #closestplanets[station] == 0 then
			return nil
		elseif #closestplanets[station] == 1 then
			planet_target = closestplanets[station][1]
		else
			planet_target = closestplanets[station][Engine.rand:Integer(1,#closestplanets[station])]
		end

		system_target = system_local
		location = planet_target
		lat, long, dist = randomLatLong()
		dist = station:DistanceTo(Space.GetBody(planet_target.bodyIndex))  --overwrite empty dist from randomLatLong()
		--1 added for short distances when most of the time is spent at low average speed (accelerating and deccelerating)
		due = (mToAU(dist) * 2 + 1) * Engine.rand:Integer(20,24) * 60 * 60     -- TODO: adjust due date based on urgency

	elseif flavour.loctype == "CLOSE_SPACE" then
		station_target = station_local
		planet_target = planet_local
		system_target = system_local
		location = planet_target
		dist = 1000 * Engine.rand:Integer(1,max_close_space_dist)     -- minimum of 1 km distance from station
		due = (60 * 60 * Engine.rand:Number(2,24))        --TODO: adjust due date based on urgency

	elseif flavour.loctype == "FAR_SPACE" then
		local nearbysystems = findNearbySystems(false)  -- setup to only return systems without stations
		station_target = nil
		system_target = nearbysystems[Engine.rand:Integer(1,#nearbysystems)]
		if not system_target then return nil end
		planet_target = randomPlanet(system_target:GetStarSystem())
		if not planet_target then return nil end
		location = planet_target
		dist = system_local:DistanceTo(system_target)
		due = (5 * dist + 4) * Engine.rand:Integer(20,24) * 60 * 60     -- TODO: adjust due date based on urgency
	end

	-- Create message string for the cases where the target landed on a planet that is higher gravity (> 1.2 g)
	high_gravity = ""
	if planet_target:GetSystemBody().gravity / 9.8066 > 1.2 and flavour.id == 4 then -- flavour id 4 is landed on a planet
		high_gravity = string.interp(l["HIGHGRAVITY_" .. Engine.rand:Integer(0, getNumberOfFlavours("HIGHGRAVITY"))], {
			planet       = planet_target:GetSystemBody().name,
			gravity      = string.format("%.2f", planet_target:GetSystemBody().gravity / 9.8066),
		})
	end

	--double the time if return to original station is required
	if flavour.reward_immediate == false then due = 2 * due end
	due = Game.time + due

	-- determine pickup and deliver of items based on mission flavour
	-- appropriate target ship size will be selected later based on this
	local pickup_comm, deliver_comm, deliver_pass

	deliver_pass = flavour.deliver_pass
	pickup_comm = copyTable(flavour.pickup_comm)
	deliver_comm = copyTable(flavour.deliver_comm)

	-- set target ship parameters and determine pickup and delivery of personnel based on mission flavour
	local shipdef, crew_num, shiplabel, pickup_crew, pickup_pass, deliver_crew, shipseed = createTargetShipParameters(flavour, planet_target)

	-- adjust fuel to deliver based on selected ship and mission flavour
	local needed_fuel
	if flavour.id == 2 or flavour.id == 5 then
		needed_fuel = math.max(math.floor(shipdef.fuelTankMass * 0.1), 1)
	elseif flavour.id == 4 then -- different planet
		needed_fuel = math.max(math.floor(shipdef.fuelTankMass * 0.5), 1)
	end
	deliver_comm[Commodities.hydrogen] = needed_fuel

	-- terminate ad creation if no suitable target ship could be created
	if not shipdef then return nil end

	-- collect possible local localities for interesting ad texts
	local localities_local = {system_local:GetStarSystem().name}
	if station_local then
		table.insert(localities_local, station_local:GetSystemBody().name)
	end
	if planet_local then
		table.insert(localities_local, planet_local:GetSystemBody().name)
	end

	-- collect possible target localities for interesting ad texts
	local localities_target = {system_target:GetStarSystem().name}
	if station_target then
		table.insert(localities_target, station_target:GetSystemBody().name)
	end
	if planet_target then
		table.insert(localities_target, planet_target:GetSystemBody().name)
	end

	-- select appropriate posting client, entity, problem
	if flavour.id == 6 then
		client = Character.New()

		-- select posting entity
		local entity_types = {"ENTITY_RESEARCH", "ENTITY_GENERAL"}
		local entity_type = entity_types[Engine.rand:Integer(1, #entity_types)]
		entity = string.interp(l[entity_type .. "_" .. Engine.rand:Integer(1, getNumberOfFlavours(entity_type))],
			{ locality = localities_local[Engine.rand:Integer(1,#localities_local)] })

		-- select problem
		local problem_type
		if entity_type == "ENTITY_RESEARCH" then problem_type = "PROBLEM_RESEARCH"
		else problem_type = "PROBLEM_GENERAL" end
		problem = string.interp(l[problem_type .. "_" ..Engine.rand:Integer(1, getNumberOfFlavours(problem_type))],
			{ locality = localities_target[Engine.rand:Integer(1,#localities_target)] })

	elseif flavour.id == 7 then
		client = Character.New()
		local lastname = splitName(client.name)[2]

		-- select posting entity
		entity = string.interp(l["ENTITY_FAMILY_BUSINESS_" .. Engine.rand:Integer(1, getNumberOfFlavours("ENTITY_FAMILY_BUSINESS"))],
			{ locality = localities_local[Engine.rand:Integer(1,#localities_local)], name = lastname })

		-- select problem
		problem = string.interp(l["PROBLEM_CREW_" .. Engine.rand:Integer(1, getNumberOfFlavours("PROBLEM_CREW"))],
		    { locality = localities_target[Engine.rand:Integer(1,#localities_target)] })
	else
		client = getAircontrolChar(station)
	end

	-- calculate the reward
	local reward = calcReward(flavour, pickup_crew, pickup_pass, pickup_comm, deliver_crew, deliver_pass, deliver_comm)
	reward = utils.round(reward, 100)

	local ad = {
		location       = location,
		station_local  = station_local,
		planet_local   = planet_local,
		system_local   = system_local,
		station_target = station_target,
		planet_target  = planet_target,
		high_gravity   = high_gravity,
		system_target  = system_target,
		flavour	       = flavour,
		client	       = client,
		entity         = entity,
		problem        = problem,
		dist           = dist,
		due	       = due,
		urgency	       = urgency,
		reward         = reward,
		shipdef_name   = shipdef.name, -- saving the actual shipdef causes crash at serialization (ship undock)
		crew_num       = crew_num,
		pickup_crew    = pickup_crew,
		pickup_pass    = pickup_pass,
		pickup_comm    = pickup_comm,
		deliver_crew   = deliver_crew,
		deliver_pass   = deliver_pass,
		deliver_comm   = deliver_comm,
		shiplabel      = shiplabel,
		lat            = lat,
		long           = long,
		shipseed       = shipseed
	}

	placeAdvert(station, ad)

	-- successfully created an advert, return non-nil
	return ad
end

local missionStatus = function (mission)
	-- Return the completion status of the mission.
	local status = "NOT"
	if mission.target_destroyed ~= "NOT" then
		status = "ABORT"
	end
	if mission.pickup_crew_check == "COMPLETE" or mission.pickup_pass_check == "COMPLETE" or
	mission.deliver_crew_check == "COMPLETE" or mission.pickup_pass_check == "COMPLETE" then
		status = "COMPLETE"
	end
	for commodity,_ in pairs(mission.pickup_comm_check) do
		if mission.pickup_comm_check[commodity] == "COMPLETE" then
			status = "COMPLETE"
		end
	end
	for commodity,_ in pairs(mission.deliver_comm_check) do
		if mission.deliver_comm_check[commodity] == "COMPLETE" then
			status = "COMPLETE"
		end
	end
	if mission.pickup_crew_check == "PARTIAL" or mission.pickup_crew_check == "ABORT" or
		mission.pickup_pass_check == "PARTIAL" or mission.pickup_pass_check == "ABORT" or
		mission.deliver_crew_check == "PARTIAL" or mission.deliver_crew_check == "ABORT" or
	mission.deliver_pass_check == "PARTIAL" or mission.deliver_pass_check == "ABORT" then
		status = "PARTIAL"
	end
	for commodity,_ in pairs(mission.pickup_comm_check) do
		if mission.pickup_comm_check[commodity] == "PARTIAL" or mission.pickup_comm_check[commodity] == "ABORT" then
			status = "PARTIAL"
		end
	end
	for commodity,_ in pairs(mission.deliver_comm_check) do
		if mission.deliver_comm_check[commodity] == "PARTIAL" or mission.deliver_comm_check[commodity] == "ABORT" then
			status = "PARTIAL"
		end
	end
	return status
end

local missionStatusReset = function (mission)
	-- Reset the mission status to "NOT". This is important when cargo for a partially completed
	-- mission has been unloaded.
	if mission.pickup_crew_check == "PARTIAL" then mission.pickup_crew_check = "NOT" end
	if mission.pickup_pass_check == "PARTIAL" then mission.pickup_pass_check = "NOT" end
	if mission.deliver_crew_check == "PARTIAL" then mission.deliver_crew_check = "NOT" end
	if mission.deliver_pass_check == "PARTIAL" then mission.deliver_pass_check = "NOT" end
	for commodity,_ in pairs(mission.pickup_comm_check) do
		if mission.pickup_comm_check[commodity] == "PARTIAL" then mission.pickup_comm_check[commodity] = "NOT" end
	end
	for commodity,_ in pairs(mission.deliver_comm_check) do
		if mission.deliver_comm_check[commodity] == "PARTIAL" then mission.deliver_comm_check[commodity] = "NOT" end
	end
end

local closeMission = function (mission)
	-- Close mission and provide reward if appropriate.

	-- determine reward points to add to or subtract from player's records
	local delta_reputation
	local oldReputation = Character.persistent.player.reputation
	if mission.loctype == "CLOSE_PLANET" or mission.loctype == "CLOSE_SPACE" then
		delta_reputation = 0.5
	elseif mission.loctype == "MEDIUM_PLANET" then
		delta_reputation = 0.75
	else
		delta_reputation = 1
	end

	-- if mission due date has passed
	if Game.time > mission.due then
		Comms.ImportantMessage(mission.flavour.failuremsg)
		Character.persistent.player.reputation = Character.persistent.player.reputation - delta_reputation
		Event.Queue("onReputationChanged", oldReputation, Character.persistent.player.killcount,
		            Character.persistent.player.reputation, Character.persistent.player.killcount)
		removeMission(mission)

	-- if mission is still on time
	-- if mission was aborted (target destroyed)
	elseif missionStatus(mission) == "ABORT" then
		if mission.target_destroyed == "BY_PLAYER" then
			Comms.ImportantMessage(l.PLAYER_DESTROYED_TARGET)
			Character.persistent.player.reputation = Character.persistent.player.reputation - 2*delta_reputation
			Event.Queue("onReputationChanged", oldReputation, Character.persistent.player.killcount,
			            Character.persistent.player.reputation, Character.persistent.player.killcount)
			removeMission(mission)
		else
			local destroyedtxt = string.interp(l.ACCIDENT_DESTROYED_TARGET, {shiplabel = mission.shiplabel})
			Comms.ImportantMessage(destroyedtxt)
			Game.player:AddMoney(mission.reward/2)
			Character.persistent.player.reputation = Character.persistent.player.reputation + delta_reputation
			Event.Queue("onReputationChanged", oldReputation, Character.persistent.player.killcount,
			            Character.persistent.player.reputation, Character.persistent.player.killcount)
			removeMission(mission)
		end
	else
		-- mission has been completed
		if missionStatus(mission) == "COMPLETE" then
			local successtxt = string.interp(mission.flavour.successmsg, {cash = Format.Money(mission.reward)})
			Comms.ImportantMessage(successtxt)
			Game.player:AddMoney(mission.reward)
			Character.persistent.player.reputation = Character.persistent.player.reputation + delta_reputation
			Event.Queue("onReputationChanged", oldReputation, Character.persistent.player.killcount,
			            Character.persistent.player.reputation, Character.persistent.player.killcount)
			removeMission(mission)

			-- if mission has been partially completed
		elseif missionStatus(mission) == "PARTIAL" then
			Comms.ImportantMessage(l.PARTIAL)
			missionStatusReset(mission)
		end
	end

	-- clear player cargo
	-- TODO: what to do if player got rid of mission commodity cargo in between (sold?)
	for _ = 1, arraySize(mission.cargo_pass) do
		removePassenger(Game.player)
	end
	for commodity,_ in pairs(mission.cargo_comm) do
		for _ = 1, mission.cargo_comm[commodity] do
			removeCargo(Game.player, commodity)
		end
	end
end

local targetInteractionDistance = function (mission)
	-- Determine if player is within interaction distance from mission target.
	if mission.target and Game.player:DistanceTo(mission.target) <= min_interaction_dist then
		return true
	else
		return false
	end
end

local pickupCrew = function (mission)
	-- Pickup a single crew member from the target ship.
	-- Called during timer loop within "interactWithTarget".
	local todo = mission.pickup_crew_orig

	-- error messages if not all parameters met
	if not crewPresent(mission.target) then
		Comms.ImportantMessage(l.MISSING_CREW)
		mission.pickup_crew_check = "PARTIAL"
		return
	elseif not passengerSpace(Game.player) then
		Comms.ImportantMessage(l.FULL_PASSENGERS)
		local done = mission.pickup_crew_orig - mission.pickup_crew
		local resulttxt = string.interp(l.RESULT_PICKUP_CREW, {todo = todo, done = done})
		Comms.ImportantMessage(resulttxt)
		if mission.pickup_pass > 0 then
			local todo_pass = mission.pickup_pass_orig
			local done_pass = mission.pickup_pass_orig - mission.pickup_pass
			local resulttxt_pass = string.interp(l.RESULT_PICKUP_PASS, {todo = todo_pass, done = done_pass})
			Comms.ImportantMessage(resulttxt_pass)
		end
		mission.pickup_crew_check = "PARTIAL"
		return

	-- pickup crew
	else
		local crew_member = removeCrew(mission.target)
		addPassenger(Game.player)
		table.insert(mission.cargo_pass, crew_member)
		local boardedtxt = string.interp(l.BOARDED_PASSENGER, {name = crew_member.name})
		Comms.ImportantMessage(boardedtxt)
		mission.crew_num = mission.crew_num - 1

		-- if all necessary crew transferred print result message
		mission.pickup_crew = mission.pickup_crew - 1
		local done = mission.pickup_crew_orig - mission.pickup_crew
		if todo == done then
			local resulttxt = string.interp(l.RESULT_PICKUP_CREW, {todo = todo, done = done})
			Comms.ImportantMessage(resulttxt)
			mission.pickup_crew_check = "COMPLETE"
			mission.location = mission.system_local
		end
	end
end

local pickupPassenger = function (mission)
	-- Pickup a single passenger from the target ship.
	-- Called during timer loop within "interactWithTarget".
	local todo = mission.pickup_pass_orig

	-- error messages if not all parameters met
	if not passengersPresent(mission.target) then
		Comms.ImportantMessage(l.MISSING_PASS)
		mission.pickup_pass_check = "PARTIAL"
		return
	elseif not passengerSpace(Game.player) then
		Comms.ImportantMessage(l.FULL_PASSENGERS)
		local done = mission.pickup_pass_orig - mission.pickup_pass
		local resulttxt = string.interp(l.RESULT_PICKUP_PASS, {todo = todo, done = done})
		Comms.ImportantMessage(resulttxt)
		mission.pickup_pass_check = "PARTIAL"
		return

	-- pickup passenger
	else
		local passenger = Character.New()
		removePassenger(mission.target)
		addPassenger(Game.player)
		table.insert(mission.cargo_pass, passenger)
		local boardedtxt = string.interp(l.BOARDED_PASSENGER, {name = passenger.name})
		Comms.ImportantMessage(boardedtxt)

		-- if all necessary passengers have been picked up show result message
		mission.pickup_pass = mission.pickup_pass - 1
		local done = mission.pickup_pass_orig - mission.pickup_pass
		if todo == done then
			local resulttxt = string.interp(l.RESULT_PICKUP_PASS, {todo = todo, done = done})
			Comms.ImportantMessage(resulttxt)
			mission.pickup_pass_check = "COMPLETE"
		end
	end
end

local pickupCommodity = function (mission, commodity)
	-- Pickup a single ton of the supplied commodity from the target ship.
	-- Called during timer loop within "interactWithTarget".
	local todo = mission.pickup_comm_orig[commodity]
	local commodity_name = commodity:GetName()

	-- error messages if parameters not met
	if not cargoPresent(mission.target, commodity) then
		local missingtxt = string.interp(l.MISSING_COMM, {cargotype = commodity_name})
		Comms.ImportantMessage(missingtxt)
		mission.pickup_comm_check[commodity] = "PARTIAL"
		return
	elseif not cargoSpace(Game.player) then
		Comms.ImportantMessage(l.FULL_CARGO)
		mission.pickup_comm_check[commodity] = "PARTIAL"
		return

	-- pickup 1 ton of commodity
	else
		removeCargo(mission.target, commodity)
		addCargo(Game.player, commodity)
		if mission.cargo_comm[commodity] == nil then
			mission.cargo_comm[commodity] = 1
		else
			mission.cargo_comm[commodity] = mission.cargo_comm[commodity] + 1
		end

		-- show result message if done picking up this commodity
		mission.pickup_comm[commodity] = mission.pickup_comm[commodity] - 1
		local done = mission.pickup_comm_orig[commodity] - mission.pickup_comm[commodity]
		if todo == done then
			local resulttxt = string.interp(l.RESULT_PICKUP_COMM, {cargotype = commodity_name, todo = todo, done = done})
			Comms.ImportantMessage(resulttxt)
			mission.pickup_comm_check[commodity] = "COMPLETE"
		end
	end
end

local deliverCrew = function (mission)
	-- Transfer a single crew member to the target ship.
	-- Called during timer loop within "interactWithTarget".
	local todo = mission.deliver_crew_orig
	local maxcrew = ShipDef[mission.target.shipId].maxCrew

	-- error messages if not all parameters met
	if not passengersPresent(Game.player) then
		Comms.ImportantMessage(l.MISSING_PASSENGER)
		mission.deliver_crew_check = "PARTIAL"
		return
	elseif mission.target:CrewNumber() == maxcrew then
		Comms.ImportantMessage(l.FULL_CREW)
		mission.deliver_crew_check = "PARTIAL"
		return

	-- transfer crew
	else
		local crew_member = table.remove(mission.cargo_pass, 1)
		removePassenger(Game.player)
		addCrew(mission.target, crew_member)
		mission.crew_num = mission.crew_num + 1
		local deliverytxt = string.interp(l.DELIVERED_PASSENGER, {name = crew_member.name})
		Comms.ImportantMessage(deliverytxt)

		-- if all necessary crew transferred print result message
		mission.deliver_crew = mission.deliver_crew - 1
		local done = mission.deliver_crew_orig - mission.deliver_crew
		if todo == done then
			local resulttxt = string.interp(l.RESULT_DELIVERY_CREW, {todo = todo, done = done})
			Comms.ImportantMessage(resulttxt)
			mission.deliver_crew_check = "COMPLETE"
		end
	end
end

local deliverPassenger = function (mission)
	-- Transfer a single passenger to the target ship.
	-- Called during timer loop within "interactWithTarget".
	local todo = mission.deliver_pass_orig

	-- error messages if not all parameters met
	if not passengersPresent(Game.player) then
		Comms.ImportantMessage(l.MISSING_PASS)
		mission.deliver_pass_check = "PARTIAL"
		return
	elseif not passengerSpace(mission.target) then
		Comms.ImportantMessage(l.FULL_PASSENGERS)
		mission.deliver_pass_check = "PARTIAL"
		return

	-- transfer passenger
	else
		local passenger = table.remove(mission.cargo_pass, 1)
		removePassenger(Game.player)
		addPassenger(mission.target)
		local deliverytxt = string.interp(l.DELIVERED_PASSENGER, {name = passenger.name})
		Comms.ImportantMessage(deliverytxt)

		-- if all necessary passengers have been transferred show result message
		mission.deliver_pass = mission.deliver_pass - 1
		local done = mission.deliver_pass_orig - mission.deliver_pass
		if todo == done then
			local resulttxt = string.interp(l.RESULT_DELIVERY_PASS, {todo = todo, done = done})
			Comms.ImportantMessage(resulttxt)
			mission.deliver_pass_check = "COMPLETE"
		end
	end
end

local deliverCommodity = function (mission, commodity)
	-- Transfer a single ton of the supplied commodity to the target ship.
	-- Called during timer loop within "interactWithTarget".
	local todo = mission.deliver_comm_orig[commodity]
	local commodity_name = commodity:GetName()

	-- error messages if parameters not met
	if not cargoPresent(Game.player, commodity) then
		local missingtxt = string.interp(l.MISSING_COMM, {cargotype = commodity_name})
		Comms.ImportantMessage(missingtxt)
		mission.deliver_comm_check[commodity] = "PARTIAL"
		return
	elseif not cargoSpace(mission.target) then
		Comms.ImportantMessage(l.FULL_CARGO)
		mission.deliver_comm_check[commodity] = "PARTIAL"
		return

	-- transfer 1 ton of commodity
	else
		removeCargo(Game.player, commodity)
		addCargo(mission.target, commodity)

		-- show result message if done delivering this commodity
		mission.deliver_comm[commodity] = mission.deliver_comm[commodity] - 1
		local done = mission.deliver_comm_orig[commodity] - mission.deliver_comm[commodity]
		if todo == done then
			local resulttxt = string.interp(l.RESULT_DELIVERY_COMM, {done = done, todo = todo, cargotype = commodity_name})
			Comms.ImportantMessage(resulttxt)
			mission.deliver_comm_check[commodity] = "COMPLETE"

			-- if commodity was fuel and the mission was local refuel the ship with it
			if commodity == Commodities.hydrogen then
				if mission.flavour.id == 2 or mission.flavour.id == 4 or mission.flavour.id == 5 then
					mission.target:Refuel(Commodities.hydrogen, mission.deliver_comm_orig[commodity])
				end
			end
		end
	end
end

local interactionCounter = function (counter)
	-- Check if target_interaction_time is reached.
	-- Called during timer loop inside "interactWithTarget".
	counter = counter + 1
	if counter >= target_interaction_time then
		return true, counter
	else
		return false, counter
	end
end

local searchForTarget  -- need to initialize function variable for use in interactWithTarget function
local interactWithTarget = function (mission)
	-- Handle all interaction with mission target once the player ship is within interaction distance.
	if Game.time > mission.due then
		Comms.ImportantMessage(l.SHIP_UNRESPONSIVE)
		return
	else
		-- calculate and display total interaction time
		local packages
		packages = mission.pickup_crew + mission.pickup_pass +
			mission.deliver_crew + mission.deliver_pass
		for _,num in pairs(mission.pickup_comm) do
			packages = packages + num
		end
		for _,num in pairs(mission.deliver_comm) do
			packages = packages + num
		end
		local total_interaction_time = target_interaction_time * packages
		local distance_reached_txt = string.interp(l.INTERACTION_DISTANCE_REACHED, {minutes = total_interaction_time/60})
		Comms.ImportantMessage(distance_reached_txt)
	end

	local counter = 0
	Timer:CallEvery(1, function ()
		local done = true

		-- abort if interaciton distance was not held or target ship destroyed
		-- TODO: set the check mark for each mission right
		if not targetInteractionDistance(mission) or mission.target == nil then
			Comms.ImportantMessage(l.INTERACTION_ABORTED)
			searchForTarget(mission)
			return true
		end

		-- perform action if time limit has passed
		local actiontime
		actiontime, counter = interactionCounter(counter)
		if actiontime then

			-- pickup crew from target ship
			if mission.pickup_crew > 0 then
				pickupCrew(mission)
				if mission.pickup_crew_check ~= "PARTIAL" then
					done = false
				end

				-- transfer crew to target ship
			elseif mission.deliver_crew > 0 then
				deliverCrew(mission)
				if mission.deliver_crew_check ~= "PARTIAL" then
					done = false
				end

				-- pickup passengers from target ship
			elseif mission.pickup_pass > 0 then
				pickupPassenger(mission)
				if mission.pickup_pass_check ~= "PARTIAL" then
					done = false
				end

				-- transfer passengers to target ship
			elseif mission.deliver_pass > 0 then
				deliverPassenger(mission)
				if mission.deliver_pass_check ~= "PARTIAL" then
					done = false
				end

				-- pickup commodity-cargo from target ship
			elseif arraySize(mission.pickup_comm) > 0 then
				for commodity,_ in pairs(mission.pickup_comm) do
					if mission.pickup_comm[commodity] > 0 then
						pickupCommodity(mission, commodity)
						if mission.pickup_comm_check[commodity] == "PARTIAL" then
							done = false
						end
					end
				end

				-- transfer commodity-cargo to target ship
			elseif arraySize(mission.deliver_comm) > 0 then
				for commodity,_ in pairs(mission.deliver_comm) do
					if mission.deliver_comm[commodity] > 0 then
						deliverCommodity(mission, commodity)
						if mission.deliver_comm_check[commodity] ~= "PARTIAL" then
							done = false
						end
					end
				end
			end

			if done then

				-- if mission should close right after transfer do so and send target ship on its way
				if missionStatus(mission) == "COMPLETE" and mission.flavour.reward_immediate == true then
					closeMission(mission)

					-- wait for random time then fly off
					local wait_secs = Engine.rand:Integer(2,5)
					Timer:CallAt(Game.time + wait_secs, function () flyToNearbyStation(mission.target) end)
				end
				return true
			end
		end
	end)
end

-- "searchForTarget" has been locally declared before!
function searchForTarget (mission)
	-- Measure distance to target every second until interaction distance reached.

	if mission.searching == true or mission.target == nil then return end
	mission.searching = true

	-- Counter to show messages only once and not every loop
	local message_counter = {INTERACTION_DISTANCE_REACHED = 1,
	                         PLEASE_LAND = 1}

	Timer:CallEvery(1, function ()

		-- abort if player is about to leave system, target ship is destroyed, or player leaves target frame
		if leaving_system or not mission.target or Game.player.frameBody ~= mission.target.frameBody then
			mission.searching = false
			return true

		else
			-- if distance to target has not been reached keep searching
			if not targetInteractionDistance(mission) then
				if message_counter.INTERACTION_DISTANCE_REACHED == 0 then
					Comms.ImportantMessage(l.INTERACTION_ABORTED)
					message_counter.INTERACTION_DISTANCE_REACHED = 1
					message_counter.PLEASE_LAND = 1
				end
				return false

				-- if distance to target has been reached start target interaction
			else
				if message_counter.INTERACTION_DISTANCE_REACHED > 0 then
					Comms.ImportantMessage(l.INTERACTION_DISTANCE_REACHED)
					message_counter.INTERACTION_DISTANCE_REACHED = 0
				end

				-- if planet-based mission require player to land
				if mission.flavour.loctype == "CLOSE_PLANET" or
				mission.flavour.loctype == "MEDIUM_PLANET" then
					if Game.player.flightState ~= "LANDED" then
						if message_counter.PLEASE_LAND > 0 then
							Comms.ImportantMessage(l.PLEASE_LAND)
							message_counter.PLEASE_LAND = 0
						end
						return false
					end
				end

				-- if mission is overdue
				if Game.time > mission.due then
					Comms.ImportantMessage(l.SHIP_UNRESPONSIVE)
					return true

				else
					-- calculate and display total interaction time
					local packages
					packages = mission.pickup_crew + mission.pickup_pass +
						mission.deliver_crew + mission.deliver_pass
					for _,num in pairs(mission.pickup_comm) do
						packages = packages + num
					end
					for _,num in pairs(mission.deliver_comm) do
						packages = packages + num
					end
					local total_interaction_time = target_interaction_time * packages
					total_interaction_time = string.format("%." .. (1 or 0) .. "f",
															total_interaction_time/60)
					local interaction_time_txt = string.interp(l.TRANSFER_TIME,
																{minutes = total_interaction_time})
					Comms.ImportantMessage(interaction_time_txt)

					-- start interaction with target ship and stop search
					interactWithTarget(mission)
					mission.searching = false
					return true
				end
			end
		end
	end)
end

local onFrameChanged = function (body)
	-- Start a new search for target every time the reference frame for player changes.
	if not body:isa("Ship") or not body:IsPlayer() then return end
	for _,mission in pairs(missions) do
		if Game.system == mission.system_target:GetStarSystem() and mission.target then
			if body.frameBody == mission.target.frameBody then
				searchForTarget(mission)
			end
		end
	end
end

local onShipUndocked = function (ship, station)
	-- Start search immediately if the target is on the same planet as the station.
	if not ship:IsPlayer() then return end
	for _,mission in pairs(missions) do
		if mission.target and ship.frameBody == mission.target.frameBody then
			searchForTarget(mission)
		end
	end
end

local onCreateBB = function (station)
	-- Initial creation of banter boards for current system.

	-- more efficient to determine closest planets per station once per banter board creation
	local closestplanets = findClosestPlanets()

	-- force ad creation for debugging
	local num = 3
	for _ = 1,num do
		-- makeAdvert(station, 1, closestplanets)
		-- makeAdvert(station, 2, closestplanets)
		-- makeAdvert(station, 3, closestplanets)
		-- makeAdvert(station, 4, closestplanets)
		-- makeAdvert(station, 5, closestplanets)
		-- makeAdvert(station, 6, closestplanets)
		-- makeAdvert(station, 7, closestplanets)
	end

	if triggerAdCreation() then makeAdvert(station, nil, closestplanets) end
end

local onUpdateBB = function (station)
	-- Called by game every 1-2 hours to update current banter boards. Ads are removed based on due date and new ones added here.

	-- remove ads based on time until due
	for ref,ad in pairs(ads) do

		-- 30 minute timeout for very close missions (same planet)
		if ad.flavour.loctype == "CLOSE_PLANET" or ad.flavour.loctype == "CLOSE_SPACE" then
			if ad.due < Game.time + 30*60 then
				Space.GetBody(ad.station_local.bodyIndex):RemoveAdvert(ref)
			end

			-- one day timeout for medium distance missions (same system)
		elseif ad.flavour.loctype == "MEDIUM_PLANET" then
			if ad.due < Game.time + 60*60*24 then
				Space.GetBody(ad.station_local.bodyIndex):RemoveAdvert(ref)
			end

			-- five day timeout as catch-all for everything else
		else
			if ad.due < Game.time + 5*60*60*24 then
				Space.GetBody(ad.station_local.bodyIndex):RemoveAdvert(ref)
			end
		end
	end

	-- force ad creation for debugging
	-- local num = 3
	-- for _ = 1,num do
	-- 	makeAdvert(station, 1, closestplanets)
	-- 	makeAdvert(station, 2, closestplanets)
	-- 	makeAdvert(station, 3, closestplanets)
	-- 	makeAdvert(station, 4, closestplanets)
	-- 	makeAdvert(station, 5, closestplanets)
	-- 	makeAdvert(station, 6, closestplanets)
	-- 	makeAdvert(station, 7, closestplanets)
	-- end

	-- trigger new ad creation if appropriate
	if triggerAdCreation() then makeAdvert(station, nil, nil) end
end

local onEnterSystem = function (player)
	if (not player:IsPlayer()) then return end
	leaving_system = false

	local syspath = Game.system.path

	-- spawn mission target ships in this system
	for _,mission in pairs(missions) do
		if mission.system_target:IsSameSystem(syspath) and mission.target_destroyed == "NOT" then
			mission.target = createTargetShip(mission)
		end
	end

	discarded_ships = {}
end

local onLeaveSystem = function (ship)
	if ship:IsPlayer() then
		leaving_system = true    --checked by searchForTarget to abort search

		local syspath = Game.system.path

		-- remove references to ships that are left behind (cause serialization crash otherwise)
		for _,mission in pairs(missions) do
			if mission.system_target:IsSameSystem(syspath) then
				mission.target = nil
			end
		end

		discarded_ships = {}
		-- TODO: put in tracker to recreate mission targets (already transferred personnel, cargo, etc.)
	end
end

local onShipDocked = function (ship, station)
	if ship:IsPlayer() then
		for _,mission in pairs(missions) do
			if mission.station_local:IsSameSystem(Game.system.path) and Space.GetBody(mission.station_local.bodyIndex) == station then
				if missionStatus(mission)~="NOT" or Game.time > mission.due then
					closeMission(mission)
				end
			end
		end
	else
		for i,discarded_ship in pairs(discarded_ships) do
			if ship == discarded_ship then

				-- add thruster fuel
				ship:SetFuelPercent(100)

				-- add hydrogen for hyperjumping
				local drive = ship:GetEquip('engine', 1)
				if drive then
					---@type CargoManager
					local cargoMgr = ship:GetComponent('CargoManager')
					local hypfuel = drive.capabilities.hyperclass ^ 2  -- fuel for max range
					hypfuel = hypfuel - cargoMgr:CountCommodity(Commodities.hydrogen)
					cargoMgr:AddCommodity(Commodities.hydrogen, math.max(hypfuel, 0))
				end

				discardShip(ship)
				table.remove(discarded_ships,i)
			end
		end
	end
end

local onReputationChanged = function (oldRep, oldKills, newRep, newKills)
	--   TODO: possibly include if reputation based ad filtering is set up
	--   for ref,ad in pairs(ads) do
	--      local oldQualified = isQualifiedFor(oldRep, ad)
	--      if isQualifiedFor(newRep, ad) ~= oldQualified then
	--	 Event.Queue("onAdvertChanged", ad.station, ref);
	--      end
	--   end
end

local loaded_data
local onGameStart = function ()

	-- create ads and missions containers
	ads = {}
	missions = {}

	if not loaded_data or not loaded_data.ads then return end

	-- fill the global containers with previously saved data if this is a reload
	for _,ad in pairs(loaded_data.ads) do
		placeAdvert(Space.GetBody(ad.station_local.bodyIndex), ad)
	end
	missions = loaded_data.missions
	aircontrol_chars = loaded_data.aircontrol_chars
	discarded_ships = loaded_data.discarded_ships
	loaded_data = nil

	-- reset searching status for all missions
	for _,mission in pairs(missions) do
		mission.searching = false
	end

	-- check if player is within frame of any mission targets to resume search
	for _,mission in pairs(missions) do
		if mission.target and Game.player.frameBody == mission.target.frameBody then
			searchForTarget(mission)
		end
	end
end

local buildMissionDescription = function(mission)
	local ui = require 'pigui'
	local textTable = require 'pigui.libs.text-table'

	local desc = {}
	local dist = Game.system and string.format("%.2f", Game.system:DistanceTo(mission.system_target:GetStarSystem())) or "???"
	if mission.flavour.loctype ~= "FAR_SPACE" and Game.system then
		dist = ui.Format.Distance(mission.dist)
	else
		dist = dist .." ".. lc.UNIT_LY
	end

	desc.client = mission.client
	desc.location = mission.location

	-- default to place-of-assistance reward
	local paymentAddress = l.PLACE_OF_ASSISTANCE
	local paymentLocation = mission.system_target

	if not mission.flavour.reward_immediate then
		paymentAddress = mission.station_local:GetSystemBody().name
		paymentLocation = mission.system_local
		desc.returnLocation = mission.station_local
	end

	local targetLocation
	if mission.lat == 0 and mission.long == 0 then
		targetLocation = mission.planet_target:GetSystemBody().name..": "..l.ORBIT
	else
		targetLocation = mission.planet_target:GetSystemBody().name..": "..
			l.LAT.." "..decToDegMinSec(math.rad2deg(mission.lat)).." / "..
			l.LON.." "..decToDegMinSec(math.rad2deg(mission.long))
	end

	desc.details = {
		{ l.TARGET_SHIP_ID, mission.shipdef_name.." <"..mission.shiplabel..">" },
		{ l.LAST_KNOWN_LOCATION, targetLocation },
		{ l.SYSTEM, ui.Format.SystemPath(mission.system_target) },
		{ l.DISTANCE, dist },
		false,
		{ l.REWARD, ui.Format.Money(mission.reward) },
		{ l.PAYMENT_LOCATION, paymentAddress },
		{ l.SYSTEM, ui.Format.SystemPath(paymentLocation) },
		{ l.DEADLINE, ui.Format.Date(mission.due) },
	}

	local pickup_comm_text = 0
	local count = 0
	for commodity, amount in pairs(mission.pickup_comm) do
		if count == 0 then
			pickup_comm_text = commodity:GetName().." ("..amount..")"
		else
			pickup_comm_text = pickup_comm_text.."\n"..commodity:GetName().." ("..amount..")"
		end
	end

	local deliver_comm_text = 0
	local count = 0
	for commodity, amount in pairs(mission.deliver_comm) do
		if count == 0 then
			deliver_comm_text = commodity:GetName().." ("..amount..")"
		else
			deliver_comm_text = deliver_comm_text.."\n"..commodity:GetName().." ("..amount..")"
		end
	end

	local pickupTable = {
		{ l.CREW, l.PASSENGERS, l.COMMODITIES },
		{ mission.pickup_crew, mission.pickup_pass, pickup_comm_text },
	}

	local deliverTable = {
		{ l.CREW, l.PASSENGERS, l.COMMODITIES },
		{ mission.deliver_crew, mission.deliver_pass, deliver_comm_text },
	}

	desc.customDetails = function()
		ui.text(l.PICKUP)
		textTable.drawTable(3, nil, pickupTable)
		ui.newLine()
		ui.text(l.DELIVERY)
		textTable.drawTable(3, nil, deliverTable)
	end

	return desc
end

local onGameEnd = function ()
	-- Currently just placeholder.
end

local onShipDestroyed = function (ship, attacker)
	for _,mission in pairs(missions) do
		if mission.target and ship == mission.target then
			mission.target = nil
			mission.target_destroyed = "BY_ACCIDENT"
			if attacker:IsDynamic() and attacker:IsPlayer() then
				mission.target_destroyed = "BY_PLAYER"
			end
		end
	end

	-- stop tracking destroyed ships in discarded_ships list
	for i, discarded_ship in pairs(discarded_ships) do
		if ship == discarded_ship then table.remove(discarded_ships,i) end
	end
end

local serialize = function ()
	return {ads              = ads,
	        missions         = missions,
	        aircontrol_chars = aircontrol_chars,
	        discarded_ships  = discarded_ships}
end

local unserialize = function (data)
	loaded_data = data
end

Event.Register("onCreateBB", onCreateBB)
Event.Register("onUpdateBB", onUpdateBB)
Event.Register("onEnterSystem", onEnterSystem)
Event.Register("onLeaveSystem", onLeaveSystem)
Event.Register("onShipDocked", onShipDocked)
Event.Register("onGameStart", onGameStart)
Event.Register("onGameEnd", onGameEnd)
Event.Register("onReputationChanged", onReputationChanged)
Event.Register("onShipUndocked", onShipUndocked)
Event.Register("onFrameChanged", onFrameChanged)
Event.Register("onShipDestroyed", onShipDestroyed)

Mission.RegisterType("searchrescue",l.SEARCH_RESCUE, buildMissionDescription)

Serializer:Register("searchrescue", serialize, unserialize)
