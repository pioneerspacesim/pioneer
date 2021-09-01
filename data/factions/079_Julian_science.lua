-- Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local f = Faction:new('Julian science community')
	:description_short('Worlds who have submitted to the rule of a group of elite scientists called the Julians')
	:description('')
	:homeworld(4,-1,9, 0, 0)
	:foundingDate(2993)
	:expansionRate(0)
	:military_name('Julian defence force')
	:police_name('Guardian')
	:colour(0.7,0.3,1)
	
f:govtype_weight('PLUTOCRATIC',10)
f:govtype_weight('EARTHDEMOC',30)
f:govtype_weight('SOCDEM',30)
f:govtype_weight('LIBDEM',30)

-- Julians are a pacifist bunch, only having a navy for self-defence
f:illegal_goods_probability('BATTLE_WEAPONS',100)

-- Julians see alcohol as mostly an unhealthy distraction and try to ban it when local cultures permits it
f:illegal_goods_probability('LIQUOR',50)

-- chemical weapons clash with pacifist/humanist agenda
f:illegal_goods_probability('NERVE_GAS',100)

-- another distraction, use is not criminal unless it affects someone else
f:illegal_goods_probability('NARCOTICS',75)

-- Absolute enforcement of human rights
f:illegal_goods_probability('SLAVES',100)

-- Julians havent expanded. rather, other systems have seen the Julians prosper and negotiated to be included into their sphere of influence
f:claim(3,-1,9,0)
f:claim(4,-1,9,1)
f:claim(4,-1,9,2)
f:claim(4,-2,10,2)
f:claim(5,-1,10,0)
f:claim(5,-2,10,1)

f:add_to_factions('Julian science community')


