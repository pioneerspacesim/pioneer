local f = Faction:new('Confederation of Independent Systems')
	:description_short('Socially democratic grouping of independent Star Systems')
	:description('Socially democratic grouping of independent Star Systems, I dunno, added them because they seem hard coded into the politics.')
	:govtype('CISSOCDEM')
	:homeworld(1,-1,-1,0,3)
	:foundingDate(3125.0)
	:expansionRate(1.0)
	:military_name('Confederation Fleet')
	:police_name('Confederal Police')
	:colour(0.4,1.0,0.4)

f:illegal_goods_probability('ANIMAL_MEAT',75)	-- fed/cis
f:illegal_goods_probability('LIVE_ANIMALS',75)	-- fed/cis
f:illegal_goods_probability('HAND_WEAPONS',25)	-- cis
f:illegal_goods_probability('BATTLE_WEAPONS',50)	--fed/cis
f:illegal_goods_probability('NERVE_GAS',100)--fed/cis
f:illegal_goods_probability('NARCOTICS',86)--cis
f:illegal_goods_probability('SLAVES',100)--fed/cis

f:add_to_factions('CIS')
