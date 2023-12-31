-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
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

f:illegal_goods_probability('live_animals',		99)
f:illegal_goods_probability('liquor',		44)
f:illegal_goods_probability('slaves',		43)
f:illegal_goods_probability('nerve_gas',		100)
f:illegal_goods_probability('narcotics',		80)

f:add_to_factions('Dagger Worlds')
