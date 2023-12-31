-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Commonwealth of Independent Worlds')
	:description_short('Socially democratic grouping of independent Star Systems')
	:description('Socially democratic grouping of independent Star Systems, I dunno, added them because they seem hard coded into the politics.')
	:homeworld(1,-1,-1,0,3)
	:foundingDate(3125)
	:expansionRate(1)
	:military_name('Confederation Fleet')
	:police_name('Confederal Police')
	:police_ship('pumpkinseed_police')
	:colour(0.4,1,0.4)

f:govtype_weight('CISSOCDEM',		80)
f:govtype_weight('CISLIBDEM',		20)

f:illegal_goods_probability('animal_meat',		75)
f:illegal_goods_probability('live_animals',		75)
f:illegal_goods_probability('slaves',		100)
f:illegal_goods_probability('hand_weapons',		25)
f:illegal_goods_probability('battle_weapons',		50)
f:illegal_goods_probability('nerve_gas',		100)
f:illegal_goods_probability('narcotics',		86)

f:add_to_factions('Commonwealth of Independent Worlds')
