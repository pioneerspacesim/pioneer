-- Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Independent Expanse')
	:description_short('Independent Expanse')
	:description('Very little is currently known about The Independent Expanse')
	:homeworld(11,38,38,0,7)
	:foundingDate(3059)
	:expansionRate(0.610352)
	:military_name('Expanse Legion')
	:police_name('Independent Police')
	:colour(0.027451,0.898039,0.647059)

f:govtype_weight('CORPORATE',		100)
f:govtype_weight('PLUTOCRATIC',		6)
f:govtype_weight('LIBDEM',		6)
f:govtype_weight('MILDICT1',		0)
f:govtype_weight('SOCDEM',		0)
f:govtype_weight('DISORDER',		0)
f:govtype_weight('COMMUNIST',		0)

f:illegal_goods_probability('LIVE_ANIMALS',		100)
f:illegal_goods_probability('ROBOTS',		36)
f:illegal_goods_probability('SLAVES',		98)
f:illegal_goods_probability('BATTLE_WEAPONS',		58)
f:illegal_goods_probability('NERVE_GAS',		92)
f:illegal_goods_probability('NARCOTICS',		100)

f:add_to_factions('Independent Expanse')


