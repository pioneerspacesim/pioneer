---------------
-- Fuel Club --
---------------

-- The fuel club is an organisation that provides subsidized hydrogen fuel,
-- military fuel and radioactives processing for its members. Membership is
-- normally annual. A Goods Trader interface is provided. Facilities do not
-- exist on every station in the galaxy.

-- Get the translator function
local t = Translate:GetTranslator()

-- Default numeric values --
----------------------------
local oneyear = 31557600 -- One standard Julian year
-- 10, guaranteed random by D16 dice roll.
-- This is to make the BBS name different from the station welcome character.

-- 27, guaranteed random by D100 dice roll.
-- This is to make the BBS name different from the station welcome character.
-- This will be unnecessary once ported to Character
local seedbump = 27

local ads = {}
local membership = { -- commented values represent 1-year membership
--	joined = 0,
--	expiry = oneyear,
}

-- 1 / probability that you'll see one in a BBS
local chance_of_availability = 4
