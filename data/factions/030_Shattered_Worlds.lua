-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Shattered Worlds')
	:description_short('Shattered Worlds')
	:description('Very little is currently known about The Shattered Worlds')
	:homeworld(-5,41,41,0,10)
	:foundingDate(3068)
	:expansionRate(0.712091)
	:military_name('Worlds Regiments')
	:police_name('Worlds Justiciars')
	:colour(1,1,0.67451)

f:govtype_weight('LIBDEM',		100)
f:govtype_weight('CORPORATE',		70)
f:govtype_weight('SOCDEM',		70)
f:govtype_weight('PLUTOCRATIC',		49)
f:govtype_weight('COMMUNIST',		49)

f:illegal_goods_probability('animal_meat',		45)
f:illegal_goods_probability('live_animals',		100)
f:illegal_goods_probability('slaves',		46)
f:illegal_goods_probability('nerve_gas',		100)

f:add_to_factions('Shattered Worlds')
