-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Far Horde')
	:description_short('Far Horde')
	:description('Very little is currently known about The Far Horde')
	:homeworld(57,-15,-15,0,6)
	:foundingDate(3126)
	:expansionRate(1.3788)
	:military_name('Far Regiments')
	:police_name('Far Security')
	:colour(0.454902,0.929412,1)

f:govtype_weight('SOCDEM',		100)
f:govtype_weight('LIBDEM',		60)
f:govtype_weight('COMMUNIST',		60)
f:govtype_weight('CORPORATE',		36)
f:govtype_weight('MILDICT2',		36)

f:illegal_goods_probability('animal_meat',		51)
f:illegal_goods_probability('robots',		100)
f:illegal_goods_probability('hand_weapons',		100)
f:illegal_goods_probability('battle_weapons',		100)
f:illegal_goods_probability('nerve_gas',		100)

f:add_to_factions('Far Horde')
