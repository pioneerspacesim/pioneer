-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('United League')
	:description_short('United League')
	:description('Very little is currently known about The United League')
	:homeworld(21,54,54,1,9)
	:foundingDate(3065)
	:expansionRate(0.673286)
	:military_name('United Regiments')
	:police_name('United Interior Ministry')
	:colour(0.878431,1,0.862745)

f:govtype_weight('DISORDER',		100)
f:govtype_weight('MILDICT2',		65)
f:govtype_weight('COMMUNIST',		42)

f:illegal_goods_probability('live_animals',		100)
f:illegal_goods_probability('slaves',		100)
f:illegal_goods_probability('hand_weapons',		96)
f:illegal_goods_probability('nerve_gas',		100)

f:add_to_factions('United League')
