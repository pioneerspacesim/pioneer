-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Blood Territories')
	:description_short('Blood Territories')
	:description('Very little is currently known about The Blood Territories')
	:homeworld(51,44,44,1,6)
	:foundingDate(3088)
	:expansionRate(0.939597)
	:military_name('Territories Navy')
	:police_name('Blood Interior Ministry')
	:colour(0.803922,0.905882,0.313726)

f:govtype_weight('MILDICT1',		100)
f:govtype_weight('DISORDER',		8)
f:govtype_weight('PLUTOCRATIC',		8)
f:govtype_weight('CORPORATE',		0)
f:govtype_weight('LIBDEM',		0)

f:illegal_goods_probability('live_animals',		78)
f:illegal_goods_probability('liquor',		47)
f:illegal_goods_probability('slaves',		100)
f:illegal_goods_probability('nerve_gas',		100)

f:add_to_factions('Blood Territories')
