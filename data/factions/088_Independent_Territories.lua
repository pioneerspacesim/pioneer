-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Independent Territories')
	:description_short('Independent Territories')
	:description('Very little is currently known about The Independent Territories')
	:homeworld(15,-27,-27,0,9)
	:foundingDate(3096)
	:expansionRate(1.03758)
	:military_name('Territories Regiments')
	:police_name('Independent Inquisition')
	:colour(0.596078,0.729412,0.368627)

f:govtype_weight('DISORDER',		100)
f:govtype_weight('MILDICT1',		27)
f:govtype_weight('PLUTOCRATIC',		7)

f:illegal_goods_probability('live_animals',		89)
f:illegal_goods_probability('slaves',		98)
f:illegal_goods_probability('hand_weapons',		100)
f:illegal_goods_probability('narcotics',		100)

f:add_to_factions('Independent Territories')
