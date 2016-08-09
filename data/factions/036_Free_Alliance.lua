-- Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Free Alliance')
	:description_short('Free Alliance')
	:description('Very little is currently known about The Free Alliance')
	:homeworld(38,-56,-56,0,3)
	:foundingDate(3122)
	:expansionRate(1.32526)
	:military_name('Free Militia')
	:police_name('Free Prefecture')
	:colour(0.462745,1,1)

f:govtype_weight('SOCDEM',		100)
f:govtype_weight('LIBDEM',		47)
f:govtype_weight('COMMUNIST',		47)
f:govtype_weight('CORPORATE',		22)
f:govtype_weight('MILDICT2',		22)

f:illegal_goods_probability('ANIMAL_MEAT',		79)
f:illegal_goods_probability('LIQUOR',		100)
f:illegal_goods_probability('SLAVES',		100)
f:illegal_goods_probability('HAND_WEAPONS',		100)
f:illegal_goods_probability('NERVE_GAS',		30)
f:illegal_goods_probability('NARCOTICS',		78)

f:add_to_factions('Free Alliance')


