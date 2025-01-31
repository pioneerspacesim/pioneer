-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Syon Free Traders & Company')
	:description_short('Independent Merchants Thriving Amidst Galactic Conflict')
    :description('Amidst the ongoing feud between The Federation and Commonwealth, a band of free merchants has adeptly played both sides, establishing an independent governance where disputes are settled over drinks at the local pub. Known for their cunning business acumen and resourcefulness, the Syon Free Traders & Company have carved out a niche as intermediaries and suppliers in the tumultuous trade lanes of the galaxy.')
	:homeworld(	4, 2, 5, 0, 2)
	:foundingDate(3224)
	:expansionRate(2)
	:military_name('Pirates Coalition Of Syon')
	:police_name('Foxi Order Keepers')
	:colour(1,0.372549,0.101961)

f:govtype_weight('MILDICT1',		80)
f:govtype_weight('DISORDER',		100)
f:govtype_weight('PLUTOCRATIC',		20)
f:govtype_weight('CORPORATE',		90)
f:govtype_weight('LIBDEM',		40)

f:illegal_goods_probability('live_animals',		50)
f:illegal_goods_probability('liquor',		0)
f:illegal_goods_probability('slaves',		100)
f:illegal_goods_probability('hand_weapons',		25)
f:illegal_goods_probability('battle_weapons',		0)
f:illegal_goods_probability('narcotics',		0)

f:add_to_factions('Syon Free Traders & Company')
