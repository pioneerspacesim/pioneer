-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- This module defines "Search and Rescue" flavours

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

local Lang = require 'Lang'
local l = Lang.GetResource("module-searchrescue")

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
		description      = "pickup random crew/pass from uninhabited, close by planet"
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
		reward_immediate = true,
		description      = "deliver random fuel close to ground station"
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
		reward_immediate = true,
		description      = "deliver 1 crew to close to ground station"
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
		reward_immediate = true,
		description      = "deliver random fuel to planet in system"
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
		reward_immediate = true,
		description      = "deliver random fuel close to space station"
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
		reward_immediate = false,
		description      = "pickup random crew/pass from distant system"
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
		reward_immediate = true,
		description      = "deliver random crew to distant system"
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

return flavours
