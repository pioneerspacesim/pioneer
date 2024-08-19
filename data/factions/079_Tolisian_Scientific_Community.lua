-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Tolisian Scientific Community"')
	:description_short('Worlds who have submitted to the rule of a group of elite scientists called the Tolisians')
	:description('')
	:homeworld(4,-1,9, 0, 0)
	:foundingDate(2993)
	:expansionRate(0)
	:military_name('Tolisian Defense Force')
	:police_name('Guardian')
	:colour(0.7,0.3,1)

f:govtype_weight('PLUTOCRATIC',10)
f:govtype_weight('EARTHDEMOC',30)
f:govtype_weight('SOCDEM',30)
f:govtype_weight('LIBDEM',30)

-- Tolisians are a pacifist bunch, only having a navy for self-defence
f:illegal_goods_probability('battle_weapons',100)

-- Tolisians see alcohol as mostly an unhealthy distraction and try to ban it when local cultures permits it
f:illegal_goods_probability('liquor',50)

-- chemical weapons clash with pacifist/humanist agenda
f:illegal_goods_probability('nerve_gas',100)

-- another distraction, use is not criminal unless it affects someone else
f:illegal_goods_probability('narcotics',75)

-- Absolute enforcement of human rights
f:illegal_goods_probability('slaves',100)

-- Tolisians haven't expanded. Rather, other systems have seen the Tolisian prosper and negotiated to be included into their sphere of influence
f:claim(3,-1,9,0)
f:claim(4,-1,9,1)
f:claim(4,-1,9,2)
f:claim(4,-2,10,2)
f:claim(5,-1,10,0)
f:claim(5,-2,10,1)

f:add_to_factions('Tolisian Scientific Community')
