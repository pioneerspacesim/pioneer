-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Dagger Kingdoms')
	:description_short('Dagger Kingdoms')
	:description('Very little is currently known about The Dagger Kingdoms')
	:homeworld(35,58,58,2,9)
	:foundingDate(3069)
	:expansionRate(0.723071)
	:military_name('Kingdoms Navy')
	:police_name('Kingdoms Prefecture')
	:colour(0.886275,0.380392,0.219608)

f:govtype_weight('LIBDEM',		100)
f:govtype_weight('CORPORATE',		69)
f:govtype_weight('SOCDEM',		69)
f:govtype_weight('PLUTOCRATIC',		48)
f:govtype_weight('COMMUNIST',		48)
f:govtype_weight('MILDICT1',		33)
f:govtype_weight('MILDICT2',		33)

f:illegal_goods_probability('live_animals',		52)
f:illegal_goods_probability('robots',		100)
f:illegal_goods_probability('slaves',		100)
f:illegal_goods_probability('hand_weapons',		57)
f:illegal_goods_probability('nerve_gas',		40)
f:illegal_goods_probability('narcotics',		100)

f:add_to_factions('Dagger Kingdoms')
