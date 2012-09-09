local f = Faction:new('Confederation of Independent Systems')
	:description_short('Socially democratic grouping of independent Star Systems')
	:description('Socially democratic grouping of independent Star Systems, I dunno, added them because they seem hard coded into the politics.')
	:govtype('CISSOCDEM')
	:homeworld(6,12,0,1,3)
	:foundingDate(3125.0)
	:expansionRate(1.0)
	:military_name('Confederation Fleet')
	:police_name('Confederal Police')
	
f:illegal_goods_probability('ANIMAL_MEAT',4,1)	-- fed/cis
f:illegal_goods_probability('LIVE_ANIMALS',4,1)	-- fed/cis
f:illegal_goods_probability('HAND_WEAPONS',3,0)	-- cis
f:illegal_goods_probability('BATTLE_WEAPONS',2,0)	--fed/cis
f:illegal_goods_probability('NERVE_GAS',0,0)--fed/cis
f:illegal_goods_probability('NARCOTICS',7,0)--cis
f:illegal_goods_probability('SLAVES',0,0)--fed/cis

f:add_to_factions('CIS')
