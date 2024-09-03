-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('True Territories')
	:description_short('True Territories')
	:description('Very little is currently known about The True Territories')
	:homeworld(36,-3,-2,1,14)
	:foundingDate(3091)
	:expansionRate(0.972914)
	:military_name('Territories Militia')
	:police_name('True Security')
	:colour(0.2,0.501961,0.823529)

f:govtype_weight('LIBDEM',		100)
f:govtype_weight('CORPORATE',		61)
f:govtype_weight('SOCDEM',		61)
f:govtype_weight('PLUTOCRATIC',		37)
f:govtype_weight('COMMUNIST',		37)
f:govtype_weight('MILDICT1',		22)
f:govtype_weight('MILDICT2',		22)

f:illegal_goods_probability('animal_meat',		60)
f:illegal_goods_probability('battle_weapons',		88)
f:illegal_goods_probability('narcotics',		83)

f:add_to_factions('True Territories')
