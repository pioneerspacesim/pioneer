-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- give name, first and last name functions

local utils = require 'utils'

local de = require '.de' -- german
local da = require '.da' -- danish
local es = require '.es' -- spanish
local en = require '.en' -- english
local fi = require '.fi' -- finish
local fr = require '.fr' -- french
local gd = require '.gd' -- gaelic
local el = require '.el' -- greek
local hu = require '.hu' -- hungarian
local is = require '.is' -- islandic
local it = require '.it' -- italian
local ja = require '.ja' -- japanese
local nl = require '.nl' -- netherlands
local nb = require '.nb' -- norwegian bokmål
local pl = require '.pl' -- polish
local ro = require '.ro' -- romanian
local ru = require '.ru' -- russian
local sv = require '.sv' -- swedish
local us = require '.us' -- USA
local tr = require '.tr' -- turkish
local zh = require '.zh' -- chinese/mandarin
local misc = require '.misc' -- Our old namegen / developer's names

--
-- Class: Culture
--

local Culture = {}

Culture.weights = {
	{lang = da,	weight = 1.0},
	{lang = de,	weight = 3.0},
	{lang = el,	weight = 1.0},
	{lang = en,	weight = 6.0},
	{lang = es,	weight = 3.0},
	{lang = fi,	weight = 1.0},
	{lang = fr,	weight = 3.0},
	{lang = gd,	weight = 0.2},
	{lang = hu,	weight = 1.0},
	{lang = is,	weight = 0.2},
	{lang = it,	weight = 3.0},
	{lang = ja,	weight = 3.0},
	{lang = nb,	weight = 1.0},
	{lang = nl,	weight = 2.0},
	{lang = pl,	weight = 2.0},
	{lang = ro,	weight = 1.0},
	{lang = ru,	weight = 3.0},
	{lang = sv,	weight = 1.0},
	{lang = tr,	weight = 1.0},
	{lang = us,	weight = 5.0},
	{lang = zh,	weight = 3.0},
	{lang = misc, weight = 10.0},
}

-- Normalize weights to sum to 1
utils.normWeights(Culture.weights)

-- Map language string to module table
Culture.lookup = {}
for k, v in pairs(Culture.weights) do
	Culture.lookup[v.lang.name] = v.lang
	-- print("* ", k, v.lang.code, v.lang.name, v.lang, v.weight)
end

--
-- Function: FirstName
--
-- Create first name, from specified culture, or default to weighted
-- probability from pre-set list of available cultures. See parameter
-- and return documentation from Culture:FullName()
--
-- > name = Culture:FirstName(isfemale, rand, culture)
--
function Culture:FirstName (isFemale, rand, culture)
	local c = self.lookup[culture] or utils.chooseNormalized(self.weights, rand).lang
	return c:FirstName(isFemale)
end

--
-- Function: Surname
--
-- Create surname, from specified culture, or default to weighted
-- probability from pre-set list of available cultures. See parameter
-- and return documentation from Culture:FullName().
--
-- > name = Culture:Surname(isfemale, rand, culture, make_ascii)
--
function Culture:Surname (isFemale, rand, culture, ascii)
	local c = self.lookup[culture] or utils.chooseNormalized(self.weights, rand).lang
	return c:Surname(isFemale, rand, ascii)
end


--
-- Function: FullName
--
-- Create a full name, where first and last match the same
-- language/culture. If no culture is specified, one is randomly
-- selected according to pre-set weights. Valid input is one of the
-- following (capitalized) strings:
--
-- Danish German Greek English Spanish Finish French Gaelic Hungarian
-- Icelandic Italian Japanese Norwegian Dutch Polish Russian Swedish
-- Turkish Chinese
--
-- > name = Culture:FullName(isfemale, rand, culture)
--
-- Parameters:
--
--   isfemale - whether to generate a male or female name. true for female,
--              false for male
--
--   rand - the <Rand> object to use to generate the name
--
--   culture - optional string
--
-- Return:
--
--   name - a string containing matching first and last name
--
-- Return full name from the same culture/language
function Culture:FullName (isFemale, rand, culture)
	-- if 'culture' given as a string, e.g. "Russian" use that
	local c = self.lookup[culture] or utils.chooseNormalized(self.weights, rand).lang

	-- local debug_code = "(".. c.code .. ") "
	-- return debug_code .. c:FullName(isFemale, rand)
	return c:FullName(isFemale)
end


return Culture
