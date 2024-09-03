-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('United Council')
	:description_short('United Council')
	:description('Very little is currently known about The United Council')
	:homeworld(-18,-6,-6,1,10)
	:foundingDate(3054)
	:expansionRate(0.551866)
	:military_name('United Navy')
	:police_name('Council Prefecture')
	:colour(0.819608,0.352941,0.176471)

f:govtype_weight('MILDICT1',		100)
f:govtype_weight('DISORDER',		33)
f:govtype_weight('PLUTOCRATIC',		33)
f:govtype_weight('CORPORATE',		11)
f:govtype_weight('LIBDEM',		3)

f:illegal_goods_probability('live_animals',		55)
f:illegal_goods_probability('liquor',		100)
f:illegal_goods_probability('robots',		100)
f:illegal_goods_probability('slaves',		100)
f:illegal_goods_probability('hand_weapons',		64)
f:illegal_goods_probability('battle_weapons',		82)
f:illegal_goods_probability('nerve_gas',		52)

f:add_to_factions('United Council')
