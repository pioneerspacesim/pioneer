-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
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

-- maximum number of factions to generate
--	* can't be more than available names (prefixes*suffixes)
--  * not all factions will be created as a faction can't be based in an empty sector
--  * for now more factions == richer environment, but worse sector view performance
local num_factions = 100  

-- earliest and latest founding date for the factions we're generating
local earliest_founding = 3050 		-- assuming feds are one of the original factions
local latest_founding   = 3180

-- minium and maxium expansion rate
local min_expansion = 0.5
local max_expansion = 2.0	-- makes max radius 300 ly

-- farthest sector x,y or z we will place homeworlds
local sector_cutoff = 60	-- -60..60

-- salt for the hash, changing it produce a different faction map
local salt = 'djk;aurn'

-- BEGIN 
------------------------------------------------------------------------------
assert(num_factions <= (#prefixes * #suffixes), 'not enough random faction names')

-- this tells the C++ side that if the homeworld sector is empty it should skip
-- creation of the faction, otherwise to use the first (non-custom) system
local MAGIC_SYSTEM_INDEX = -1

-- build faction names
local faction_names = {}

for i1,prefix in ipairs(prefixes) do
	for i2,suffix in ipairs(suffixes) do
		table.insert(faction_names, prefix .. ' ' .. suffix)
	end
end

while #faction_names > num_factions do
	index = util.hash_random(#faction_names, 0, (#faction_names) - 1)
	table.remove(faction_names, index)
end

-- add a faction of each name
for i,faction_name in ipairs(faction_names) do
	
	local seed = salt .. faction_name
	
	-- roll a homeworld sector\s
	local sector_x = util.hash_random(seed .. 'x', -sector_cutoff, sector_cutoff)
	local sector_y = util.hash_random(seed .. 'y', -sector_cutoff, sector_cutoff)
	local sector_z = util.hash_random(seed .. 'x', -sector_cutoff, sector_cutoff)
	
	-- roll a faction colour 
	local r = util.hash_random(seed .. 'colour_r') 
	local g = util.hash_random(seed .. 'colour_g')
	local b = util.hash_random(seed .. 'colour_b')
			
	-- make the faction
	local f = Faction:new(faction_name)
		:description_short(faction_name)
		:description(string.format('Very little is currently known about The %s',faction_name))
		:homeworld(sector_x, sector_y, sector_y, MAGIC_SYSTEM_INDEX, 0)
		:foundingDate(util.hash_random(seed, earliest_founding, latest_founding))
		:expansionRate((util.hash_random(seed)  * (max_expansion - min_expansion)) + min_expansion)
		:colour(r,g,b)
		
	-- and add it to rhe factions list
	f:add_to_factions(faction_name)
end

------------------------------------------------------------------------------
-- END generation