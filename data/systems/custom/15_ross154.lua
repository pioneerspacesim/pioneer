-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local system = CustomSystem:new('Ross 154', { 'STAR_M',  })
	:govtype('EARTHCOLONIAL')
	:short_desc('Civilian research facilities.')
	:long_desc([[Ross 154 may look like a typical red star, but is uncommon in that it hosts several planets in its habitable zone, whose mass and gravity would make them suitable for an earth-like atmosphere. This is why the Solar Federation decided to locate here its "Mass Type-M Terraforming Research Project", or MTMRP for short.
It took some debate in the Houses of Parliament to fund this project. After all, mankind started terraforming Mars about a millennium ago, and the process is common knowledge -a "child’s play" according to some Members of Parliament.
Indeed, the base principles discovered by our distant forefathers are still the basics of the terraforming process in present day. They have been refined over the course of history and past mistakes have been learned from, like the failure of 61 Cygni. But the problems from yesteryear are still real to this day: terraforming a planet is very resource intensive, takes a very long time, and is extremely delicate.
The MTMRP, nicknamed "Life bomb project", aims to revolutionise the terraforming technology. Rather than making step-by-step improvements, the principle is to engineer a planetary catastrophe, so that the environment becomes quickly suitable for human life. Scientists are free to experiment with Ross 154's planets, which are now nicknamed after the various catastrophes they have been subjected to.
Should the project succeed, terraforming could be done fast and cheap. This would benefit those citizens who want to leave overcrowded planets with overpriced real estate. And it would benefit the Federation, as these colonists will move to its core systems rather than to the frontier.]])




local ross154 = CustomSystemBody:new("Ross 154", 'STAR_M')
	:radius(f(3100,10000))
	:mass(f(4500,10000))
	:seed(545472947)
	:temp(2341)

local ross154a = CustomSystemBody:new("Radiation", 'PLANET_TERRESTRIAL')
	:radius(f(4113,10000))
	:mass(f(696,10000))
	:seed(1727018950)
	:temp(450)
	:semi_major_axis(f(195,10000))
	:eccentricity(f(541,10000))
	:rotation_period(f(14781,10000))
	:axial_tilt(fixed.deg2rad(f(0,10000)))
	:metallicity(f(982,10000))
	:volcanicity(f(20,10000))
	:atmos_density(f(0,10000))
	:atmos_oxidizing(f(5829,10000))
	:ocean_cover(f(0,10000))
	:ice_cover(f(0,10000))
	:life(f(0,10000))

local ross154b = CustomSystemBody:new("GrayGoo", 'PLANET_TERRESTRIAL')
	:radius(f(9649,10000))
	:mass(f(8984,10000))
	:seed(516645858)
	:temp(358)
	:semi_major_axis(f(352,10000))
	:eccentricity(f(24,10000))
	:rotation_period(f(35948,10000))
	:axial_tilt(fixed.deg2rad(f(0,10000)))
	:metallicity(f(677,10000))
	:volcanicity(f(1814,10000))
	:atmos_density(f(3452,10000))
	:atmos_oxidizing(f(3746,10000))
	:ocean_cover(f(846,10000))
	:ice_cover(f(118,10000))
	:life(f(2629,10000))

local ross154c = CustomSystemBody:new("Impact", 'PLANET_TERRESTRIAL')
	:radius(f(17572,10000))
	:mass(f(30879,10000))
	:seed(3927562870)
	:temp(220)
	:semi_major_axis(f(578,10000))
	:eccentricity(f(1688,10000))
	:rotation_period(f(75567,10000))
	:axial_tilt(fixed.deg2rad(f(0,10000)))
	:metallicity(f(383,10000))
	:volcanicity(f(2455,10000))
	:atmos_density(f(131,10000))
	:atmos_oxidizing(f(9807,10000))
	:ocean_cover(f(49,10000))
	:ice_cover(f(11,10000))
	:life(f(0,10000))
	

local ross154d = CustomSystemBody:new("Supervolcano", 'PLANET_TERRESTRIAL')
	:radius(f(20210,10000))
	:mass(f(40845,10000))
	:seed(3892437323)
	:temp(699)
	:semi_major_axis(f(929,10000))
	:eccentricity(f(64,10000))
	:rotation_period(f(154036,10000))
	:axial_tilt(fixed.deg2rad(f(0,10000)))
	:metallicity(f(241,10000))
	:volcanicity(f(9526,10000))
	:atmos_density(f(59909,10000))
	:atmos_oxidizing(f(8306,10000))
	:ocean_cover(f(7999,10000))
	:ice_cover(f(572,10000))
	:life(f(0,10000))


local ross154d_starports = 	{
	CustomSystemBody:new("MTMRP Support Facility", 'STARPORT_ORBITAL')
		:seed(1720938802)
		:temp(220)
		:semi_major_axis(f(700,1000000))
		:eccentricity(f(0,10000))
		:rotation_period(f(3,10000)),
	}

local ross154e = CustomSystemBody:new("Apocalypse", 'PLANET_GAS_GIANT')
	:radius(f(72879,10000))
	:mass(f(531128,10000))
	:seed(652776390)
	:temp(141)
	:semi_major_axis(f(1970,10000))
	:eccentricity(f(3180,10000))
	:rotation_period(f(470619,10000))

local ross154f = CustomSystemBody:new("Frashokereti", 'PLANET_GAS_GIANT') --End of the world (actually, the final triumph of good over evil) in Zoroastrian religion
	:radius(f(119012,10000))
	:mass(f(1416375,10000))
	:seed(3251980880)
	:temp(89)
	:semi_major_axis(f(4958,10000))
	:eccentricity(f(674,10000))
	:rotation_period(f(673380,10000))
	:axial_tilt(fixed.deg2rad(f(792,10000)))

local ross154g = CustomSystemBody:new("Ragnarok", 'PLANET_GAS_GIANT')
	:radius(f(136459,10000))
	:mass(f(2971545,10000))
	:seed(1576731514)
	:temp(69)
	:semi_major_axis(f(8115,10000))
	:eccentricity(f(727,10000))
	:rotation_period(f(14167,10000))
	:axial_tilt(fixed.deg2rad(f(3336,10000)))

local ross154h = CustomSystemBody:new("Ollaio", 'PLANET_GAS_GIANT')--A homage to the generated system Ollaio located -1,0,-8, which I copypasted here
	:radius(f(119481,10000))
	:mass(f(1427569,10000))
	:seed(3872239538)
	:temp(53)
	:semi_major_axis(f(13458,10000))
	:eccentricity(f(701,10000))
	:rotation_period(f(7083,10000))
	:axial_tilt(fixed.deg2rad(f(289,10000)))

system:bodies(ross154, 
	{
	ross154a, 
	ross154b, 
	ross154c,
	ross154d, 
		ross154d_starports,
	ross154e, 
	ross154f, 
	ross154g, 
	ross154h, 
	})

system:add_to_sector(-2,-1,-1,v(0.918,0.761,0.510))
