-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Continuity Council')
	:description_short('Continuity Council')
	:description('Very little is currently known about The Continuity Council')
	:homeworld(42,-10,-10,1,4)
	:foundingDate(3084)
	:expansionRate(0.892816)
	:military_name('Continuity Battle Flight')
	:police_name('Council Justiciars')
	:colour(0.592157,1,0.588235)

f:govtype_weight('LIBDEM',		100)
f:govtype_weight('CORPORATE',		12)
f:govtype_weight('SOCDEM',		12)
f:govtype_weight('PLUTOCRATIC',		1)
f:govtype_weight('COMMUNIST',		1)
f:govtype_weight('MILDICT1',		0)
f:govtype_weight('MILDICT2',		0)

f:illegal_goods_probability('animal_meat',		44)
f:illegal_goods_probability('live_animals',		100)
f:illegal_goods_probability('robots',		100)
f:illegal_goods_probability('battle_weapons',		69)

f:add_to_factions('Continuity Council')
