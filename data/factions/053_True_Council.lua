-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('True Council')
	:description_short('True Council')
	:description('Very little is currently known about The True Council')
	:homeworld(-50,46,46,0,7)
	:foundingDate(3152)
	:expansionRate(1.67588)
	:military_name('True Battle Flight')
	:police_name('True Interior Ministry')
	:colour(0.913726,1,0.372549)

f:govtype_weight('LIBDEM',		100)
f:govtype_weight('CORPORATE',		12)
f:govtype_weight('SOCDEM',		12)
f:govtype_weight('PLUTOCRATIC',		1)
f:govtype_weight('COMMUNIST',		1)
f:govtype_weight('MILDICT1',		0)
f:govtype_weight('MILDICT2',		0)

f:illegal_goods_probability('live_animals',		100)
f:illegal_goods_probability('liquor',		100)
f:illegal_goods_probability('slaves',		62)
f:illegal_goods_probability('battle_weapons',		100)
f:illegal_goods_probability('nerve_gas',		35)
f:illegal_goods_probability('narcotics',		92)

f:add_to_factions('True Council')
