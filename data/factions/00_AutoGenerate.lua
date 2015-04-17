-- Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

--
-- A Simple Automatic Faction generator
--

--
-- Because Factions run in their own Lua sandbox, we can't access lots of
-- stuff here, hence usage of util.hash_random, although that is likely
-- to be the correct way to go anyway.
--

-- CONFIGURATION
------------------------------------------------------------------------------

local prefixes = { 'Rim', 'Obsidian', 'United', 'Shattered','Stellar','Free'
				 , 'Blood', 'True', 'Far', 'Dagger', 'Liberation', 'Provisional'
				 , 'Continuity', 'Independent', 'Inner', 'Outer' }

local suffixes = { 'Alliance', 'Union', 'Expanse', 'Horde', 'Faction'
                 , 'Network', 'League', 'Empire', 'Kingdoms', 'Systems'
				 , 'Council', 'Worlds', 'Commonwealth', 'Territories'
				 , 'Republic' }

local police_suffixes   = {'Police', 'Constabulary', 'Interior Ministry', 'Security', 'Inquisition', 'Prefecture'
                          ,'Justiciars'}
local military_suffixes = {'Navy', 'Defense Force', 'Militia', 'Legion', 'War Fleet', 'Guards', 'Space Patrol', 'Regiments'
                          ,'Space Arm', 'Defense Wing', 'Battle Flight'}

local possible_govtypes = { 'DISORDER', 'MILDICT1', 'PLUTOCRATIC', 'CORPORATE',
                            'LIBDEM', 'SOCDEM', 'COMMUNIST', 'MILDICT2', 'DISORDER' }

local possible_illegal = { 'NERVE_GAS', 'SLAVES', 'BATTLE_WEAPONS', 'NARCOTICS', 'HAND_WEAPONS',
                           'LIVE_ANIMALS', 'ANIMAL_MEAT', 'LIQUOR', 'ROBOTS'}


-- chances that goods in the possible_illegal list are actually illegal
local illegal_chance_facbase = 0.8  -- chance that first good will be added to the illegal list
local illegal_chance_facdec  = 0.06 -- decrease in chance as we go through the list
local illegal_system_min     = 25   -- minimum illegal goods percentage if the good is in the list
local illegal_system_max     = 175  -- maximun illegal goods percentage > 100 is coerced to 100

-- limits to the number government types allowed in a faction
local min_govtypes = 2
local max_govtypes = 6

-- maximum number of factions to generate
--  * can't be more than available names (prefixes*suffixes)
--  * not all factions will be created as a faction can't be based in an empty sector
--  * for now more factions == richer environment, but potentially worse sector view performance
local num_factions = 100

-- earliest and latest founding date for the factions we're generating
local earliest_founding = 3050 		-- assuming feds are one of the original factions
local latest_founding   = 3180

-- minimum and maxium expansion rate
local min_expansion = 0.5
local max_expansion = 2.0	-- makes max radius 300 ly

-- constraints on sectors we will place homeworlds
local sector_cutoff  = 60	-- -60..60 : maximum/minimum x,y or z
local sector_yz_tilt = 1     -- fixed relationship between y and x

-- salt for the hash, changing it produce a different faction map
local salt = 'djk;aurn'

-- BEGIN
------------------------------------------------------------------------------
assert(num_factions <= (#prefixes * #suffixes), 'not enough random faction names')

-- this tells the C++ side that if the homeworld sector is empty it should skip
-- creation of the faction, otherwise to use the first (non-custom) system
local MAGIC_SYSTEM_INDEX = -1

-- build faction names
local faction_names  = {}
local police_names   = {}
local military_names = {}

for i1,prefix in ipairs(prefixes) do
	for i2,suffix in ipairs(suffixes) do
		local faction_name = prefix .. ' ' .. suffix
		local seed = salt .. faction_name

		table.insert(faction_names, faction_name)
		if util.hash_random(seed..'police')   < 0.5 then table.insert(police_names,   prefix..' '..police_suffixes  [util.hash_random(seed..'policep', 1, #police_suffixes)])
		                                            else table.insert(police_names,   suffix..' '..police_suffixes  [util.hash_random(seed..'polices', 1, #police_suffixes)])
		end
		if util.hash_random(seed..'military') < 0.5 then table.insert(military_names, prefix..' '..military_suffixes[util.hash_random(seed..'militp',  1, #military_suffixes)])
		                                            else table.insert(military_names, suffix..' '..military_suffixes[util.hash_random(seed..'milits',  1, #military_suffixes)])
		end
	end
end

while #faction_names > num_factions do
	index = util.hash_random(#faction_names, 0, (#faction_names) - 1)
	table.remove(faction_names,  index)
	table.remove(police_names,   index)
	table.remove(military_names, index)
end

-- add a faction of each name
for i,faction_name in ipairs(faction_names) do
	local seed        = salt..faction_name

	---------------------------------------------------------------------------
	-- roll a homeworld sector
	local sector_x = util.hash_random(seed .. 'x', -sector_cutoff, sector_cutoff)
	local sector_y = util.hash_random(seed .. 'y', -sector_cutoff, sector_cutoff)

	-- deliberately don't make z independent, as we have so much volume to fill we
	-- get more interesting border layouts when generating the factions on one plane
	local sector_z = sector_y * sector_yz_tilt

	---------------------------------------------------------------------------
	-- roll a faction colour
	local r = util.hash_random(seed .. 'colour_r')
	local g = util.hash_random(seed .. 'colour_g')
	local b = util.hash_random(seed .. 'colour_b')

	-- attempt to punch up the luminance in a super naive way
	local luminance = (r * 0.3) + (g * 0.6) * (b * 0.1)
	if luminance < 1 then
		r = math.min(r * 1.2, 1)
		g = math.min(g * 1.2, 1)
		b = math.min(b * 1.2, 1)
	end;

	---------------------------------------------------------------------------
	-- make the faction
	local f = Faction:new(faction_name)
		:description_short(faction_name)
		:description(string.format('Very little is currently known about The %s',faction_name))
		:homeworld(sector_x, sector_y, sector_z, MAGIC_SYSTEM_INDEX, 0)
		:foundingDate(util.hash_random(seed, earliest_founding, latest_founding))
		:expansionRate((util.hash_random(seed)  * (max_expansion - min_expansion)) + min_expansion)
		:colour(r,g,b)
		:police_name(police_names[i])
		:military_name(military_names[i])

	---------------------------------------------------------------------------
	-- roll government type parameters
	local favoured_gov_idx = util.hash_random(seed .. 'favouredgov', 1, #possible_govtypes)
	local govtype_halfspan = math.ceil(util.hash_random(seed .. 'govspan', min_govtypes, max_govtypes) / 2)
	local govtype_flatness = util.hash_random(seed .. 'govflatness') * 0.8
	local weight           = 100

	-- add favoured gov type at highest weight
	f:govtype_weight(possible_govtypes[favoured_gov_idx], weight)

	-- add additional gov types at decreasing weights
	for i=1, govtype_halfspan do
		weight = math.floor(weight * govtype_flatness)
		if (favoured_gov_idx - i) >= 1                  then f:govtype_weight(possible_govtypes[favoured_gov_idx - i], weight) end
		if (favoured_gov_idx + i) <= #possible_govtypes then f:govtype_weight(possible_govtypes[favoured_gov_idx + i], weight) end
	end

	---------------------------------------------------------------------------
	-- roll illegal goods chances
	local illegal_chance_faction = illegal_chance_facbase
	for i,good in ipairs(possible_illegal) do
		if util.hash_random(seed..good) < illegal_chance_faction then
			f:illegal_goods_probability(good, math.min(100, util.hash_random(seed..good..'chance', illegal_system_min, illegal_system_max)))
		end;
		illegal_chance_faction = illegal_chance_faction - illegal_chance_facdec
	end

	---------------------------------------------------------------------------
	-- and add the faction to the factions list
	f:add_to_factions(faction_name)
end

------------------------------------------------------------------------------
-- END generation
