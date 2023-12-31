-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('United Union')
	:description_short('United Union')
	:description('Very little is currently known about The United Union')
	:homeworld(48,-17,-17,1,7)
	:foundingDate(3101)
	:expansionRate(1.08399)
	:military_name('United Battle Flight')
	:police_name('Union Police')
	:colour(0.968628,0.690196,0.498039)

f:govtype_weight('COMMUNIST',		100)
f:govtype_weight('SOCDEM',		66)
f:govtype_weight('MILDICT2',		66)

f:illegal_goods_probability('animal_meat',		100)
f:illegal_goods_probability('live_animals',		33)
f:illegal_goods_probability('liquor',		87)
f:illegal_goods_probability('robots',		100)
f:illegal_goods_probability('hand_weapons',		100)
f:illegal_goods_probability('battle_weapons',		70)
f:illegal_goods_probability('narcotics',		100)

f:add_to_factions('United Union')
