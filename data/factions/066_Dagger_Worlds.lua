-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Dagger Worlds')
	:description_short('Dagger Worlds')
	:description('Very little is currently known about The Dagger Worlds')
	:homeworld(4,-59,-59,0,16)
	:foundingDate(3137)
	:expansionRate(1.50077)
	:military_name('Dagger Militia')
	:police_name('Dagger Justiciars')
	:colour(0.960784,0.368627,1)

f:govtype_weight('DISORDER',		100)
f:govtype_weight('MILDICT1',		23)
f:govtype_weight('PLUTOCRATIC',		5)
f:govtype_weight('CORPORATE',		1)

f:illegal_goods_probability('LIVE_ANIMALS',		99)
f:illegal_goods_probability('LIQUOR',		44)
f:illegal_goods_probability('SLAVES',		43)
f:illegal_goods_probability('NERVE_GAS',		100)
f:illegal_goods_probability('NARCOTICS',		80)

f:add_to_factions('Dagger Worlds')


