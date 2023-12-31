-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('United Commonwealth')
	:description_short('United Commonwealth')
	:description('Very little is currently known about The United Commonwealth')
	:homeworld(-42,46,46,1,17)
	:foundingDate(3072)
	:expansionRate(0.760194)
	:military_name('Commonwealth Militia')
	:police_name('United Constabulary')
	:colour(1,0.952941,0.176471)

f:govtype_weight('DISORDER',		100)
f:govtype_weight('MILDICT1',		71)

f:illegal_goods_probability('animal_meat',		33)
f:illegal_goods_probability('slaves',		73)
f:illegal_goods_probability('nerve_gas',		80)
f:illegal_goods_probability('narcotics',		70)

f:add_to_factions('United Commonwealth')
