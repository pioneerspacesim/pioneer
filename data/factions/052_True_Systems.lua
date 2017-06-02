-- Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('True Systems')
	:description_short('True Systems')
	:description('Very little is currently known about The True Systems')
	:homeworld(-8,-37,-37,0,15)
	:foundingDate(3069)
	:expansionRate(0.71887)
	:military_name('Systems Defense Wing')
	:police_name('True Prefecture')
	:colour(0.552941,0.0980392,0.65098)

f:govtype_weight('SOCDEM',		100)
f:govtype_weight('LIBDEM',		58)
f:govtype_weight('COMMUNIST',		58)
f:govtype_weight('CORPORATE',		33)
f:govtype_weight('MILDICT2',		33)

f:illegal_goods_probability('LIVE_ANIMALS',		100)
f:illegal_goods_probability('ROBOTS',		39)
f:illegal_goods_probability('SLAVES',		100)
f:illegal_goods_probability('NERVE_GAS',		100)
f:illegal_goods_probability('NARCOTICS',		100)

f:add_to_factions('True Systems')


