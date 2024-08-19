-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Tolan Kingdom')
	:description_short('Tolan Kingdom')
	:description('The Tolan Kingdom, famous (or infamous, depending on who you ask) for repelling Haber invasion forces 500 years ago.')
	:homeworld(2,-9,-19,2,4) -- edge of Habers sphere of influence
	:foundingDate(2337) -- as per timeline
	:expansionRate(0.01) -- a dozen systems
	:military_name('Tolan Defense Force')
	:police_name('Sky Marshal')
	:colour(0.85098,0.87451,0.152941) -- there is no feudal type!

f:govtype_weight('MILDICT1', 100)

-- due to early colonizing efforts often lacking live animals, Tolan Kingdom cultivated local species
f:illegal_goods_probability('live_animals',		100)

-- king has absolute authority, no internal struggles allowed except in sanctioned chivalry displays or duels
f:illegal_goods_probability('hand_weapons',		100)
f:illegal_goods_probability('battle_weapons',	100)
f:illegal_goods_probability('nerve_gas',		100)

-- narcotics traded permitted only by royal decree usually only by favored lords
f:illegal_goods_probability('narcotics',		100)

f:add_to_factions('Tolan Kingdom')
