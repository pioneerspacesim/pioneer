-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Dagger Expanse')
	:description_short('Dagger Expanse')
	:description('Very little is currently known about The Dagger Expanse')
	:homeworld(-26,-55,-55,0,1)
	:foundingDate(3108)
	:expansionRate(1.17048)
	:military_name('Dagger Regiments')
	:police_name('Expanse Interior Ministry')
	:colour(0.309804,0.486275,0.623529)

f:govtype_weight('LIBDEM',		100)
f:govtype_weight('CORPORATE',		76)
f:govtype_weight('SOCDEM',		76)
f:govtype_weight('PLUTOCRATIC',		58)
f:govtype_weight('COMMUNIST',		58)
f:govtype_weight('MILDICT1',		44)
f:govtype_weight('MILDICT2',		44)

f:illegal_goods_probability('live_animals',		59)
f:illegal_goods_probability('slaves',		92)
f:illegal_goods_probability('nerve_gas',		70)

f:add_to_factions('Dagger Expanse')
