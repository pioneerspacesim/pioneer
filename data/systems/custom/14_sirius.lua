-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local system = CustomSystem:new('Sirius', { 'STAR_A', 'WHITE_DWARF',  })
	:govtype('CISLIBDEM')
	:short_desc('Loose confederacy of city states')
	:long_desc([[One of the first systems settled by mankind, Sirius was founded in 2328 when colony ship "Ibn Battuta" landed on Planet Antar. 
Antar is not an outdoor world per se, as humans and earthling crops cannot breathe its atmosphere. Yet, its mild pressure and temperature makes it easy to build structures with life support. As such, the Sirians live solely in cities: they are, and have always been, a urban civilization.
A Sirian city is typically built around an abundant source of liquid water -"oases"- and hosts a wide range of activities, from hydroponic farming to high tech industry. A city's growth is limited by the water resource, and new cities end up being founded on other oases. As a result, Sirius is dotted with city-states.
These city-states have never been united by a strong government. The very loose "Sirius confederacy" merely host the talks between mayors and ensures no rogue city breaks the few consensual rules that shape Sirius' identity: open trade, no armed agression, no nosy bureaucracy interfering with the city-states' business.
After the War of Hope, the confederacy's role extended to represent Sirius in diplomatic dealings with foreign systems. Forced to choose a side, the Sirians favoured the one which seemed to offer the best opportunities. As a co-founder of the Commonwealth and making up one of its economic powerhouses, Sirius was and still is highly influential: its efforts have been instrumental for the current detente between the two superpowers.
While they barely ever waged war during their many centuries old history, the Sirians did wage fierce economic competition among themselves. Great riches have been amassed, yet they are highly concentrated in the hands of a few cities and corporations. 
The lack of cooperation explains why this wealthy system never terraformed planet Antar despite its potential. Travellers not only awe at the sight of Sirius' extravagant monuments, but also at the thought they are located right next to the direst of slums.]])

--Names do not break any copyright
--Bodies' names come from various inspiration from the Islamic golden age
--Starports' names are actual places in Iraq or Syria

local siriusaXb = CustomSystemBody:new("Sirius A,B", 'GRAVPOINT')
	:radius(f(0,10000))
	:mass(f(26800,10000))

local siriusa = CustomSystemBody:new("Sirius A", 'STAR_A')
	:radius(f(16600,10000))
	:mass(f(18300,10000))
	:seed(3030931680)
	:temp(8414)
	:semi_major_axis(f(3806,10000))
	:eccentricity(f(1583,10000))
	:orbital_offset(fixed.deg2rad(f(1114747,10000)))

local siriusaa = CustomSystemBody:new("Gherib", 'PLANET_TERRESTRIAL')--Gherib / Agib characters in 1001 nights
	:radius(f(2379,10000))
	:mass(f(135,10000))
	:seed(4102160151)
	:temp(2645)
	:semi_major_axis(f(392,10000))
	:eccentricity(f(667,10000))
	:rotation_period(f(20920,10000))
	:axial_tilt(fixed.deg2rad(f(0,10000)))
	:metallicity(f(1223,10000))
	:volcanicity(f(104,10000))
	:atmos_density(f(0,10000))
	:atmos_oxidizing(f(4620,10000))
	:ocean_cover(f(0,10000))
	:ice_cover(f(0,10000))
	:life(f(0,10000))

local siriusab = CustomSystemBody:new("Agib", 'PLANET_TERRESTRIAL')--Gherib / Agib characters in 1001 nights
	:radius(f(2467,10000))
	:mass(f(150,10000))
	:seed(1399601161)
	:temp(1812)
	:semi_major_axis(f(834,10000))
	:eccentricity(f(1936,10000))
	:rotation_period(f(65030,10000))
	:axial_tilt(fixed.deg2rad(f(0,10000)))
	:rotational_phase_at_start(fixed.deg2rad(f(0,10000)))
	:orbital_phase_at_start(fixed.deg2rad(f(0,10000)))
	:orbital_offset(fixed.deg2rad(f(0,10000)))
	:metallicity(f(783,10000))
	:volcanicity(f(47,10000))
	:atmos_density(f(0,10000))
	:atmos_oxidizing(f(4478,10000))
	:ocean_cover(f(0,10000))
	:ice_cover(f(0,10000))
	:life(f(0,10000))

local siriusb = CustomSystemBody:new("Sirius B", 'WHITE_DWARF')
	:radius(f(100,10000))
	:mass(f(8500,10000))
	:seed(1246053655)
	:temp(35610)
	:semi_major_axis(f(8194,10000))
	:eccentricity(f(1583,10000))
	:rotation_period(f(0,10000))
	:axial_tilt(fixed.deg2rad(f(0,10000)))
	:rotational_phase_at_start(fixed.deg2rad(f(0,10000)))
	:orbital_phase_at_start(fixed.deg2rad(f(1800000,10000)))
	:orbital_offset(fixed.deg2rad(f(1114747,10000)))

local siriusba = CustomSystemBody:new("Mukhariq", 'PLANET_TERRESTRIAL')--Mukhariq famous singer during the islamic golden age
	:radius(f(1265,10000))
	:mass(f(20,10000))
	:seed(2575500751)
	:temp(740)
	:semi_major_axis(f(595,10000))
	:eccentricity(f(1435,10000))
	:rotation_period(f(57427,10000))
	:axial_tilt(fixed.deg2rad(f(0,10000)))
	:rotational_phase_at_start(fixed.deg2rad(f(0,10000)))
	:orbital_phase_at_start(fixed.deg2rad(f(0,10000)))
	:orbital_offset(fixed.deg2rad(f(0,10000)))
	:metallicity(f(1329,10000))
	:volcanicity(f(8,10000))
	:atmos_density(f(0,10000))
	:atmos_oxidizing(f(9101,10000))
	:ocean_cover(f(0,10000))
	:ice_cover(f(0,10000))
	:life(f(0,10000))

local siriusaXba = CustomSystemBody:new("Nasreddin", 'PLANET_GAS_GIANT')--Nasreddin philospoher
	:radius(f(138466,10000))
	:mass(f(2520065,10000))
	:seed(1788263119)
	:temp(270)
	:semi_major_axis(f(40157,10000))
	:eccentricity(f(6,10000))
	:rotation_period(f(42917,10000))
	:axial_tilt(fixed.deg2rad(f(3867,10000)))
	:rotational_phase_at_start(fixed.deg2rad(f(0,10000)))
	:orbital_phase_at_start(fixed.deg2rad(f(0,10000)))
	:orbital_offset(fixed.deg2rad(f(0,10000)))

local siriusaXba1 = CustomSystemBody:new("Antar", 'PLANET_TERRESTRIAL')--Antar famous pre-islamc poet
	:radius(f(533,1000))
	:mass(f(107,1000))
	:seed(18)
	:temp(280)
	:semi_major_axis(f(81,10000))
	:eccentricity(f(191,10000))
	:rotation_period(f(18386,10000))
	:metallicity(f(214,10000))
	:volcanicity(f(20,10000))
	:atmos_density(f(8695,10000))
	:atmos_oxidizing(f(8208,10000))
	:ocean_cover(f(542,10000))
	:ice_cover(f(873,10000))
	:life(f(3569,10000))

local siriusaXba1_starports = 	{
	CustomSystemBody:new("Samarra", 'STARPORT_SURFACE')
		:latitude(math.deg2rad(32.02528))
		:longitude(math.deg2rad(-5.260278)),
	CustomSystemBody:new("Majadil", 'STARPORT_SURFACE')
		:latitude(math.deg2rad(-29.36111))
		:longitude(math.deg2rad(20.39389)),
	CustomSystemBody:new("Rofush", 'STARPORT_SURFACE')
		:latitude(math.deg2rad(-27.185))
		:longitude(math.deg2rad(-145.0358)),
	CustomSystemBody:new("Makhan station", 'STARPORT_ORBITAL')
		:seed(575259125)
		:temp(245)
		:semi_major_axis(f(160,1000000))
		:eccentricity(f(0,10000))
		:rotation_period(f(3,10000)),
	}

local siriusaXbb = CustomSystemBody:new("Bulukiya", 'PLANET_GAS_GIANT')--Bulukiya character in 1001 nights
	:radius(f(128657,10000))
	:mass(f(5774990,10000))
	:seed(4128389807)
	:temp(190)
	:semi_major_axis(f(75357,10000))
	:eccentricity(f(87,10000))
	:rotation_period(f(29167,10000))
	:axial_tilt(fixed.deg2rad(f(1786,10000)))

local siriusaXbb1 = CustomSystemBody:new("Kamil", 'PLANET_TERRESTRIAL')--Kamil character in one of the earliest science fiction works
	:radius(f(4532,10000))
	:mass(f(931,10000))
	:seed(694636352)
	:temp(122)
	:semi_major_axis(f(31,10000))
	:eccentricity(f(108,10000))
	:rotation_period(f(15079,10000))
	:metallicity(f(7144,10000))
	:volcanicity(f(642,10000))
	:atmos_density(f(0,10000))
	:atmos_oxidizing(f(1769,10000))
	:ocean_cover(f(0,10000))
	:ice_cover(f(0,10000))
	:life(f(0,10000))

local siriusaXbb1_starports = 	{
	CustomSystemBody:new("Kamonah", 'STARPORT_SURFACE')
		:latitude(math.deg2rad(147.2))
		:longitude(math.deg2rad(90.0)),
	}

local siriusaXbb2 = CustomSystemBody:new("Tarafa", 'PLANET_TERRESTRIAL')--Tarafa pre-islamic poet
	:radius(f(3209,10000))
	:mass(f(331,10000))
	:seed(3532897279)
	:temp(122)
	:semi_major_axis(f(51,10000))
	:eccentricity(f(348,10000))
	:rotation_period(f(31902,10000))
	:axial_tilt(fixed.deg2rad(f(0,10000)))
	:metallicity(f(6251,10000))
	:volcanicity(f(184,10000))
	:atmos_density(f(0,10000))
	:atmos_oxidizing(f(8238,10000))
	:ocean_cover(f(0,10000))
	:ice_cover(f(0,10000))
	:life(f(0,10000))

local siriusaXbb2_starports = 	{
	CustomSystemBody:new("Shahba", 'STARPORT_SURFACE')
		:latitude(math.deg2rad(105.7))
		:longitude(math.deg2rad(90.0)),
	}

local siriusaXbb3 = CustomSystemBody:new("Abu Nuwas", 'PLANET_TERRESTRIAL')--Abu Nuwas Hedonistic poet during the islamic golden age
	:radius(f(7777,10000))
	:mass(f(4703,10000))
	:seed(3368095255)
	:temp(122)
	:semi_major_axis(f(107,10000))
	:eccentricity(f(720,10000))
	:rotation_period(f(96844,10000))
	:axial_tilt(fixed.deg2rad(f(0,10000)))
	:metallicity(f(5321,10000))
	:volcanicity(f(442,10000))
	:atmos_density(f(0,10000))
	:atmos_oxidizing(f(3658,10000))
	:ocean_cover(f(0,10000))
	:ice_cover(f(0,10000))
	:life(f(0,10000))

local siriusaXbb3_starports = {
	CustomSystemBody:new("Al Mutassim", 'STARPORT_SURFACE')
		:latitude(math.deg2rad(145.1))
		:longitude(math.deg2rad(90.0)),
	CustomSystemBody:new("Shawish", 'STARPORT_SURFACE')
		:latitude(math.deg2rad(0))
		:longitude(math.deg2rad(0)),
	CustomSystemBody:new("Dojama High", 'STARPORT_ORBITAL')
		:seed(575259125)
		:temp(122)
		:semi_major_axis(f(160,1000000))
		:eccentricity(f(0,10000))
		:rotation_period(f(3,10000)),
	}

local siriusaXbc = CustomSystemBody:new("Avicenna", 'PLANET_GAS_GIANT')--Avicenna a most prominent scholar from the islamic golden age
	:radius(f(109277,10000))
	:mass(f(36460730,10000))
	:seed(922749454)
	:temp(139)
	:semi_major_axis(f(140767,10000))
	:eccentricity(f(984,10000))
	:rotation_period(f(46250,10000))
	:axial_tilt(fixed.deg2rad(f(906,10000)))

local siriusaXbc1 = CustomSystemBody:new("Aladdin", 'PLANET_TERRESTRIAL')--Aladdin character in Arabian nights
	:radius(f(3257,10000))
	:mass(f(345,10000))
	:seed(2462666269)
	:temp(89)
	:semi_major_axis(f(112,10000))
	:eccentricity(f(598,10000))
	:rotation_period(f(41542,10000))
	:axial_tilt(fixed.deg2rad(f(0,10000)))
	:metallicity(f(8049,10000))
	:volcanicity(f(271,10000))
	:atmos_density(f(0,10000))
	:atmos_oxidizing(f(7962,10000))
	:ocean_cover(f(0,10000))
	:ice_cover(f(0,10000))
	:life(f(0,10000))

local siriusaXbc2 = CustomSystemBody:new("Ja'far", 'PLANET_TERRESTRIAL')--Ja'far character in 1001 nights
	:radius(f(9074,10000))
	:mass(f(7472,10000))
	:seed(2499434247)
	:temp(89)
	:semi_major_axis(f(218,10000))
	:eccentricity(f(1081,10000))
	:rotation_period(f(112575,10000))
	:axial_tilt(fixed.deg2rad(f(0,10000)))
	:metallicity(f(7306,10000))
	:volcanicity(f(5752,10000))
	:atmos_density(f(1965,10000))
	:atmos_oxidizing(f(8745,10000))
	:ocean_cover(f(1414,10000))
	:ice_cover(f(794,10000))
	:life(f(0,10000))

local siriusaXbc3 = CustomSystemBody:new("Juha", 'PLANET_TERRESTRIAL')--Juha character in 1001 nights
	:radius(f(9060,10000))
	:mass(f(7436,10000))
	:seed(3784383480)
	:temp(89)
	:semi_major_axis(f(352,10000))
	:eccentricity(f(520,10000))
	:rotation_period(f(229204,10000))
	:axial_tilt(fixed.deg2rad(f(3,10000)))
	:metallicity(f(229,10000))
	:volcanicity(f(1269,10000))
	:atmos_density(f(2125,10000))
	:atmos_oxidizing(f(2740,10000))
	:ocean_cover(f(90,10000))
	:ice_cover(f(50,10000))
	:life(f(0,10000))

local siriusaXbc4 = CustomSystemBody:new("Attaf", 'PLANET_TERRESTRIAL')--Attaf character in 1001 nights
	:radius(f(12504,10000))
	:mass(f(15635,10000))
	:seed(95434140)
	:temp(109)
	:semi_major_axis(f(523,10000))
	:eccentricity(f(14,10000))
	:rotation_period(f(402114,10000))
	:axial_tilt(fixed.deg2rad(f(27,10000)))
	:metallicity(f(5679,10000))
	:volcanicity(f(6491,10000))
	:atmos_density(f(6551,10000))
	:atmos_oxidizing(f(2879,10000))
	:ocean_cover(f(4713,10000))
	:ice_cover(f(2648,10000))
	:life(f(0,10000))

local siriusaXbc4_starports = 	{
	CustomSystemBody:new("Jalaa", 'STARPORT_SURFACE')
		:latitude(math.deg2rad(29.39194))
		:longitude(math.deg2rad(-32.44389)),
	}

local siriusaXbc5 = CustomSystemBody:new("Ali Zaybaq", 'PLANET_TERRESTRIAL')--Ali Zaybaq character in 1001 nights
	:radius(f(3265,10000))
	:mass(f(348,10000))
	:seed(3142057167)
	:temp(89)
	:semi_major_axis(f(830,10000))
	:eccentricity(f(305,10000))
	:rotation_period(f(22083,10000))
	:axial_tilt(fixed.deg2rad(f(701,10000)))
	:metallicity(f(2221,10000))
	:volcanicity(f(345,10000))
	:atmos_density(f(0,10000))
	:atmos_oxidizing(f(151,10000))
	:ocean_cover(f(0,10000))
	:ice_cover(f(0,10000))
	:life(f(0,10000))

local siriusaXbd = CustomSystemBody:new("Mu'allaqah", 'PLANET_GAS_GIANT')--Mu'allaqah Very famous pre-islamic poems
	:radius(f(108235,10000))
	:mass(f(40625910,10000))
	:seed(1602046787)
	:temp(96)
	:semi_major_axis(f(290886,10000))
	:eccentricity(f(2776,10000))
	:rotation_period(f(62083,10000))
	:axial_tilt(fixed.deg2rad(f(3606,10000)))

local siriusaXbd1 = CustomSystemBody:new("Bayad", 'PLANET_TERRESTRIAL')--Bayad/Riyad characters in a love poem
	:radius(f(2091,10000))
	:mass(f(91,10000))
	:seed(3207495726)
	:temp(62)
	:semi_major_axis(f(120,10000))
	:eccentricity(f(2157,10000))
	:rotation_period(f(43726,10000))
	:axial_tilt(fixed.deg2rad(f(0,10000)))
	:metallicity(f(1944,10000))
	:volcanicity(f(71,10000))
	:atmos_density(f(0,10000))
	:atmos_oxidizing(f(3954,10000))
	:ocean_cover(f(0,10000))
	:ice_cover(f(0,10000))
	:life(f(0,10000))

local siriusaXbd2 = CustomSystemBody:new("Riyad", 'PLANET_TERRESTRIAL')--Bayad/Riyad characters in a love poem
	:radius(f(4380,10000))
	:mass(f(840,10000))
	:seed(2208963838)
	:temp(62)
	:semi_major_axis(f(256,10000))
	:eccentricity(f(1974,10000))
	:rotation_period(f(135196,10000))
	:axial_tilt(fixed.deg2rad(f(0,10000)))
	:metallicity(f(4680,10000))
	:volcanicity(f(707,10000))
	:atmos_density(f(0,10000))
	:atmos_oxidizing(f(7114,10000))
	:ocean_cover(f(0,10000))
	:ice_cover(f(0,10000))
	:life(f(0,10000))

local siriusaXbd3 = CustomSystemBody:new("Zuhayr", 'PLANET_TERRESTRIAL')--Zuhayr pre-islamic poet
	:radius(f(1785,10000))
	:mass(f(57,10000))
	:seed(1969414909)
	:temp(62)
	:semi_major_axis(f(481,10000))
	:eccentricity(f(1003,10000))
	:rotation_period(f(208364,10000))
	:axial_tilt(fixed.deg2rad(f(2448,10000)))
	:metallicity(f(7747,10000))
	:volcanicity(f(17,10000))
	:atmos_density(f(0,10000))
	:atmos_oxidizing(f(9759,10000))
	:ocean_cover(f(0,10000))
	:ice_cover(f(0,10000))
	:life(f(0,10000))

local siriusaXbd4 = CustomSystemBody:new("Nabati", 'PLANET_TERRESTRIAL')--Nabati Vernacular poetry
	:radius(f(7063,10000))
	:mass(f(3523,10000))
	:seed(3542381727)
	:temp(62)
	:semi_major_axis(f(792,10000))
	:eccentricity(f(684,10000))
	:rotation_period(f(382417,10000))
	:axial_tilt(fixed.deg2rad(f(334,10000)))
	:metallicity(f(1628,10000))
	:volcanicity(f(1765,10000))
	:atmos_density(f(0,10000))
	:atmos_oxidizing(f(1572,10000))
	:ocean_cover(f(0,10000))
	:ice_cover(f(0,10000))
	:life(f(0,10000))

local siriusaXbd5 = CustomSystemBody:new("Mawwal", 'PLANET_TERRESTRIAL')--Mawwal Vernacular poetry
	:radius(f(12147,10000))
	:mass(f(14754,10000))
	:seed(2699833775)
	:temp(62)
	:semi_major_axis(f(1420,10000))
	:eccentricity(f(1456,10000))
	:rotation_period(f(37500,10000))
	:axial_tilt(fixed.deg2rad(f(1085,10000)))
	:metallicity(f(3461,10000))
	:volcanicity(f(9289,10000))
	:atmos_density(f(5078,10000))
	:atmos_oxidizing(f(1293,10000))
	:ocean_cover(f(4534,10000))
	:ice_cover(f(3656,10000))
	:life(f(0,10000))

system:bodies(siriusaXb, 
	{
	siriusa, 
		{
		siriusaa, 
		siriusab, 
		}, 
	siriusb, 
		{
		siriusba, 
		}, 
	siriusaXba,
		{
		siriusaXba1,
			siriusaXba1_starports,
		},
 	siriusaXbb,
		{
		siriusaXbb1,
			siriusaXbb1_starports,
		siriusaXbb2,
			siriusaXbb2_starports,
		siriusaXbb3,
			siriusaXbb3_starports,
		},
 	siriusaXbc,
		{
		siriusaXbc1,
		siriusaXbc2,
		siriusaXbc3,
		siriusaXbc4,
			siriusaXbc4_starports,
		siriusaXbc5,
		},
	siriusaXbd,
		{
		siriusaXbd1,
		siriusaXbd2,
		siriusaXbd3,
		siriusaXbd4,
		siriusaXbd5,
		},
	})

system:add_to_sector(1,0,-1,v(0.0100,0.2010,0.6910))
