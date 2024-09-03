-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Rim Commonwealth')
	:description_short('Rim Commonwealth')
	:description('Very little is currently known about The Rim Commonwealth')
	:homeworld(-2,-26,-26,0,12)
	:foundingDate(3055)
	:expansionRate(0.564889)
	:military_name('Rim Militia')
	:police_name('Commonwealth Security')
	:colour(0.894118,0.945098,0.4)

f:govtype_weight('PLUTOCRATIC',		100)
f:govtype_weight('MILDICT1',		53)
f:govtype_weight('CORPORATE',		53)
f:govtype_weight('DISORDER',		28)
f:govtype_weight('LIBDEM',		28)
f:govtype_weight('SOCDEM',		14)

f:illegal_goods_probability('liquor',		58)
f:illegal_goods_probability('slaves',		53)
f:illegal_goods_probability('nerve_gas',		100)

f:add_to_factions('Rim Commonwealth')
