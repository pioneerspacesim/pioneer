-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

--
-- Interface: NameGen
--
-- Functions for generating names.
--

local Engine = require 'Engine'
local Culture = require 'culture/culture'

local r = function (t, rand) return t[rand:Integer(1,#t)] end

local NameGen
NameGen = {
	outdoorPlanetFormats = {},
	rockPlanetFormats = {},

	orbitalStarportFormats = {},
	surfaceStarportFormats = {},

--
-- Function: FullName
--
-- Create a full name (first + surname) string
--
-- > name = Namegen.FullName(isfemale, rand)
--
-- Parameters:
--
--   isfemale - whether to generate a male or female name. true for female,
--              false for male
--
--   rand - optional, the <Rand> object to use to generate the name. if
--          omitted/nil, <Engine.rand> will be used
--
-- Return:
--
--   name - a string containing the name
--
-- Availability:
--
--   alpha 10
--
-- Status:
--
--   stable
--

	FullName = function (isfemale, rand)
		if not rand then rand = Engine.rand end

		return Culture:FullName(isfemale, rand)
	end,

--
-- Function: Surname
--
-- Create a surname string
--
-- > name = Namegen.Surname(rand)
--
-- Parameters:
--
--   rand - optional, the <Rand> object to use to generate the name. if
--          omitted, <Engine.rand> will be used
--
--   ascii - optional, if true, replace non-ascii characters in name,
--         	 this is needed for any location that should be searchable,
--           by input from e.g. standard US keyboard
--
-- Return:
--
--   name - a string containing the name
--
-- Availability:
--
--   alpha 10
--
-- Status:
--
--   stable
--

	Surname = function (rand, ascii)
		if not rand then rand = Engine.rand end

		-- Coinflip, some last names are gender specific
		local isfemale = rand:Integer(1) == 1

		return Culture:Surname(isfemale, rand, nil, ascii)
	end,

--
-- Function: BodyName
--
-- Create a planet name
--
-- > name = Namegen.BodyName(body, rand)
--
-- Parameters:
--
--   body - the <SystemBody> object to provide a name for. Currently must of type
--          STARPORT_ORBITAL, STARPORT_SURFACE or ROCKY_PLANET. Any other types
--          a Lua error.
--
--   rand - optional, the <Rand> object to use to generate the name. if
--          omitted, <Engine.rand> will be used
--
-- Return:
--
--   name - a string containing the name
--
-- Availability:
--
--   alpha 19
--
-- Status:
--
--   experimental
--
	BodyName = function (body, rand)
		local ascii = true -- want only ascii compatible characers in name

		if not rand then rand = Engine.rand end

		if body.type == "STARPORT_ORBITAL" then
			return string.interp(r(NameGen.orbitalStarportFormats, rand), { name = NameGen.Surname(rand, ascii) })
		end

		if body.type == "STARPORT_SURFACE" then
			return string.interp(r(NameGen.surfaceStarportFormats, rand), { name = NameGen.Surname(rand, ascii) })
		end

		if body.superType == "ROCKY_PLANET" then

			-- XXX -15-50C is "outdoor". once more planet composition
			-- attributes are exposed we can do better here
			if body.averageTemp >= 258 and body.averageTemp <= 323 then
				return string.interp(r(NameGen.outdoorPlanetFormats, rand), { name = NameGen.Surname(rand, ascii) })
			end

			return string.interp(r(NameGen.rockPlanetFormats, rand), { name = NameGen.Surname(rand, ascii) })
		end

		error("No available namegen for body type '" .. body.type .. "'")
	end
}

NameGen.outdoorPlanetFormats = {
	"{name}",
	"{name}'s World",
	"{name}world",
	"{name} Colony",
	"{name}'s Hope",
	"{name}'s Dream",
	"New {name}",
}

NameGen.rockPlanetFormats = {
	"{name}'s Mine",
	"{name}'s Claim",
	"{name}'s Folly",
	"{name}'s Grave",
	"{name}'s Misery",
	"{name} Colony",
	"{name}'s Rock",
	"{name} Settlement",
}

NameGen.orbitalStarportFormats = {
	"{name}",
	"{name} Spaceport",
	"{name} High",
	"{name} Orbiter",
	"{name} Base",
	"{name} Station",
	"{name} Outpost",
	"{name} Citadel",
	"{name} Platform",
	"{name} Ring",
	"{name} Residence",
	"{name} Orbital",
	"{name} Habitat",
	"{name} Hub",
	"{name} Terminal",
	"{name} Trade Center",
	"{name} Commercial Center",
	"{name} Science Station",
	"{name} Observation Post",
	"{name} Facility",
	"{name} Industrial Center",
	"{name} Refinery",
	"{name} Dock",
	"{name} Depot",
	"{name} Anchorage",
}

NameGen.surfaceStarportFormats = {
	"{name}",
	"{name}",
	"{name} Starport",
	"{name} Spaceport",
	"{name} Town",
	"{name} City",
	"{name} Village",
	"Fort {name}",
	"Fortress {name}",
	"{name} Base",
	"{name} Station",
	"{name}ton",
	"{name}ville",
	"Port {name}",
	"{name} Port",
	"{name} Pad",
	"{name} Terminal",
	"{name} Oasis",
}

return NameGen
