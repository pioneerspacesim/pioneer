-- Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local s = CustomSystem:new('Sol', { 'STAR_G' })
	:govtype('EARTHDEMOC')
	:lawlessness(f(1,100)) -- 1/100th from a peaceful eden
	:short_desc('The historical birthplace of humankind')
	:long_desc([[Sol is a fine joint]])

local sol = CustomSystemBody:new('Sol', 'STAR_G')
	:radius(f(1,1))
	:mass(f(1,1))
	:temp(5700)

local mercury = CustomSystemBody:new('Mercury', 'PLANET_TERRESTRIAL')
	:seed(6)
	:radius(f(38,100))
	:mass(f(55,1000))
	:temp(340)
	:semi_major_axis(f(387,1000))
	:eccentricity(f(205,1000))
	:inclination(math.deg2rad(7.0))
	:rotation_period(f(59,1))
	:axial_tilt(fixed.deg2rad(f(1,100)))
	:metallicity(f(9,10))
	:volcanicity(f(1,2))
	:atmos_density(f(0,1))
	:atmos_oxidizing(f(0,1))
	:ocean_cover(f(0,1))
	:ice_cover(f(2,100))
	:life(f(0,1))
	:orbital_phase_at_start(fixed.deg2rad(f(286,1)))

local venus = CustomSystemBody:new('Venus', 'PLANET_TERRESTRIAL')
	:radius(f(95,100))
	:mass(f(815,1000))
	:temp(735)
	:semi_major_axis(f(723,1000))
	:eccentricity(f(7,1000))
	:inclination(math.deg2rad(3.39))
	:rotation_period(f(243,1))
	:axial_tilt(fixed.deg2rad(f(26,10)))
	:metallicity(f(1,2))
	:volcanicity(f(8,10))
	:atmos_density(f(53,1))
	:atmos_oxidizing(f(12,100))
	:ocean_cover(f(0,1))
	:ice_cover(f(0,1))
	:life(f(0,1))
	:orbital_phase_at_start(fixed.deg2rad(f(248,1)))

local earth = CustomSystemBody:new('Earth', 'PLANET_TERRESTRIAL')
	:radius(f(1,1))
	:mass(f(1,1))
	:temp(288)
	:semi_major_axis(f(1,1))
	:eccentricity(f(167,10000))
	:rotation_period(f(1,1))
	:axial_tilt(fixed.deg2rad(f(2344,100)))
	:rotational_phase_at_start(fixed.deg2rad(f(170,1)))
	:height_map('earth.hmap',0)
	:metallicity(f(1,2))
	:volcanicity(f(1,10))
	:atmos_density(f(1,1))
	:atmos_oxidizing(f(99,100))
	:ocean_cover(f(7,10))
	:ice_cover(f(5,10))
	:life(f(9,10))
	:orbital_phase_at_start(fixed.deg2rad(f(336,1)))

local earth_starports = {
	CustomSystemBody:new('Shanghai', 'STARPORT_SURFACE')
		:latitude(math.deg2rad(31))
		:longitude(math.deg2rad(-121)),
	CustomSystemBody:new('Mexico City', 'STARPORT_SURFACE')
		:latitude(math.deg2rad(19))
		:longitude(math.deg2rad(99)),
	CustomSystemBody:new('London', 'STARPORT_SURFACE')
		:latitude(math.deg2rad(51))
		:longitude(0)
		:space_station_type("ground_station"),
	CustomSystemBody:new('Moscow', 'STARPORT_SURFACE')
		:latitude(math.deg2rad(55))
		:longitude(math.deg2rad(-37.5)),
	CustomSystemBody:new('Brasilia', 'STARPORT_SURFACE')
		:latitude(math.deg2rad(-15.4))
		:longitude(math.deg2rad(48)),
	CustomSystemBody:new('Los Angeles', 'STARPORT_SURFACE')
		:latitude(math.deg2rad(34))
		:longitude(math.deg2rad(118)),
	CustomSystemBody:new('Gates Spaceport', 'STARPORT_ORBITAL')
		:seed(1)
		:semi_major_axis(f(100,100000))
		:rotation_period(f(1,24*60*3)),
	CustomSystemBody:new('Jobs Pad', 'STARPORT_ORBITAL')
		:seed(13)
		:semi_major_axis(f(100,100000))
		:rotation_period(f(1,24*60*3)),
	CustomSystemBody:new('Torvalds Platform', 'STARPORT_ORBITAL')
		:seed(0)
		:semi_major_axis(f(5.0,100000))
		:rotation_period(f(1,24*60*3))
		:orbital_phase_at_start(fixed.deg2rad(f(0,1)))
		:axial_tilt(fixed.deg2rad(f(668,100))),

}

local moon = {
	CustomSystemBody:new('Moon', 'PLANET_TERRESTRIAL')
		:seed(-5)
		:radius(f(273,1000))
		:mass(f(12,1000))
		:temp(220)
		:semi_major_axis(f(257,100000))
		:eccentricity(f(549,10000))
		:height_map('moon.hmap',1)
		:inclination(math.deg2rad(5.145))
		:rotation_period(f(273,10))
		:axial_tilt(fixed.deg2rad(f(668,100)))
		:orbital_phase_at_start(fixed.deg2rad(f(0,1)))
		:rotational_phase_at_start(fixed.deg2rad(f(0,1)))
		:volcanicity(f(0,1)),
	{
		CustomSystemBody:new('Tranquility Base', 'STARPORT_SURFACE')
			:latitude(math.deg2rad(0.6875))
			:longitude(math.deg2rad(23.4334))
	},
}

local mars = CustomSystemBody:new('Mars', 'PLANET_TERRESTRIAL')
	:seed(-1317315059)
	:radius(f(533,1000))
	:mass(f(107,1000))
	:temp(278)
	:semi_major_axis(f(152,100))
	:eccentricity(f(933,10000))
	:height_map('mars.hmap',0)  --
	:inclination(math.deg2rad(1.85))
	:rotation_period(f(1027,1000))
	:axial_tilt(fixed.deg2rad(f(2519,100)))
	-- XXX composition copied from earth until there's a way to indicate terraformed
	:metallicity(f(14.5,5))
	:volcanicity(f(2,10))
	:atmos_density(f(489,1000))
	:atmos_oxidizing(f(950001,1000000))
	:ocean_cover(f(10,100))
	:ice_cover(f(440,1000))
	:life(f(10,100))
	:orbital_phase_at_start(fixed.deg2rad(f(12,1)))

local mars_starports = {
	CustomSystemBody:new('Bradbury Landing', 'STARPORT_SURFACE')
		:latitude(math.deg2rad(-4.5895))
		:longitude(math.deg2rad(-137.4417)),
	CustomSystemBody:new('Cydonia', 'STARPORT_SURFACE')
		:latitude(math.deg2rad(-29))
		:longitude(math.deg2rad(124)),
	CustomSystemBody:new('Olympus Mons', 'STARPORT_SURFACE')
		:latitude(math.deg2rad(25.60955))
		:longitude(math.deg2rad(-41.35269)),
	CustomSystemBody:new('Mars High', 'STARPORT_ORBITAL')
		:semi_major_axis(f(5068,100000000))
		:rotation_period(f(11,24)),
}

local mars_moons = {
	CustomSystemBody:new('Phobos', 'PLANET_ASTEROID')
		:seed(439771126)
		:radius(f(21,10000))
		:mass(f(1775,1000000000000)) -- 10.6e15 kg = 1.775e-9 EM
		:semi_major_axis(f(627,10000000))
		:rotation_period(f(319,1000))
		:temp(233)
		:eccentricity(f(151,10000))
		:inclination(math.deg2rad(1.093))
		:metallicity(f(4,5))
		:volcanicity(f(3,4)),
	{
		CustomSystemBody:new('Phobos Base', 'STARPORT_SURFACE')
			:latitude(math.deg2rad(5))
			:longitude(math.deg2rad(-5)),
	},
	CustomSystemBody:new('Deimos', 'PLANET_ASTEROID')
		:seed(439771126)
		:radius(f(12,10000))
		:mass(f(247,1000000000000)) -- 1.48e15 kg = 2.47e-10 EM - comes out as 1
		:semi_major_axis(f(1568,10000000))
		:rotation_period(f(1263,1000))
		:temp(233)
		:eccentricity(f(2,10000))
		:inclination(math.deg2rad(0.93))
		:metallicity(f(7,10))
		:volcanicity(f(1,1)),
	{
		CustomSystemBody:new('Tomm\'s Sanctuary', 'STARPORT_SURFACE'),
	},
}

local eros = CustomSystemBody:new('Eros', 'PLANET_TERRESTRIAL')
	:seed(842785371)
	:radius(f(3,1000)) -- 16.84km mean
	:mass(f(112,100000000000)) -- 6.687e15 kg
	:temp(164)
	:semi_major_axis(f(1458,1000)) -- 1.458 AU
	:eccentricity(f(223,1000)) -- 0.223
	:inclination(math.deg2rad(10.829)) -- 10.829°
	:rotation_period(f(2194,10000)) -- 5h16m
	:orbital_phase_at_start(fixed.deg2rad(f(230,1))) -- random, to get it away from Luna

local pallas = CustomSystemBody:new('Pallas', 'PLANET_TERRESTRIAL')
	:radius(f(08,100)) -- 512km
	:mass(f(353,10000000)) -- 2.11e20 kg
	:temp(164)
	:semi_major_axis(f(27716,10000)) -- 2.7716 AU
	:eccentricity(f(23127,100000)) -- 0.23127
	:inclination(math.deg2rad(34.84)) -- 34.84°
	:rotation_period(f(78,10)) -- 7.8h
	:axial_tilt(fixed.deg2rad(f(84,1))) -- 84°

local vesta = CustomSystemBody:new('Vesta', 'PLANET_TERRESTRIAL')
	:radius(f(082,1000)) -- 525km
	:mass(f(4338,100000000)) -- 2.590e20 kg
	:temp(177) -- 85 - 270K
	:semi_major_axis(f(236179,100000)) -- 2.36179 AU
	:eccentricity(f(08874,100000)) -- 0.08874
	:inclination(math.deg2rad(7.14043)) -- 7.14043°
	:rotation_period(f(2226,10000)) -- 5.342h
	:axial_tilt(fixed.deg2rad(f(29,1))) -- 29°

local ceres = CustomSystemBody:new('Ceres', 'PLANET_TERRESTRIAL')
	:radius(f(074,1000)) -- 473km
	:mass(f(15,100000)) -- 9.393e20 kg
	:temp(168)
	:semi_major_axis(f(27675,10000)) -- 2.7675 AU
	:eccentricity(f(758,10000)) -- 0.075823
	:inclination(math.deg2rad(10.593)) -- 10.593°
	:rotation_period(f(3781,10000)) -- 9h
	:axial_tilt(fixed.deg2rad(f(4,1))) -- 4°

local jupiter = CustomSystemBody:new('Jupiter', 'PLANET_GAS_GIANT')
	:seed(786424632)
	:radius(f(11,1))
	:mass(f(3178,10))
	:temp(165)
	:atmos_density(f(163,100))
	:atmos_oxidizing(f(8,10))
	:semi_major_axis(f(5204,1000))
	:eccentricity(f(488,10000))
	:inclination(math.deg2rad(1.305))
	:rotation_period(f(4,10))
	:axial_tilt(fixed.deg2rad(f(313,100)))
	:rings(f(11176,10000), f(11769,10000), {0.61, 0.48, 0.384, 0.8})
	:orbital_phase_at_start(fixed.deg2rad(f(75,1)))

local jupiter_moons = {

	CustomSystemBody:new('Metis', 'PLANET_ASTEROID')
		:seed(-98)
		:radius(f(337,100000))
		:mass(f(633,100000000000)) -- 3.6e16 kg = 6.33e-9 EM
		:semi_major_axis(f(856,1000000))
		:rotation_period(f(2948,10000))
		:temp(123)
		:eccentricity(f(2,10000))
		:inclination(math.deg2rad(1.57))
		:metallicity(f(2,1000))
		:volcanicity(f(0,1))
		:ice_cover(f(100,1)),

	CustomSystemBody:new('Adrastea', 'PLANET_ASTEROID')
		:seed(-3981)
		:radius(f(338,100000))
		:mass(f(352,1000000000000)) -- 0.2e16 kg = 3.52e-10 EM
		:semi_major_axis(f(862,1000000))
		:rotation_period(f(2983,10000))
		:temp(122)
		:eccentricity(f(15,10000))
		:inclination(math.deg2rad(0.03))
		:metallicity(f(7,10))
		:volcanicity(f(1,1)),

	CustomSystemBody:new('Amalthea', 'PLANET_ASTEROID')
		:seed(-9982)
		:radius(f(13,1000))
		:mass(f(348,1000000000)) -- 208e16 kg = 3.48e-7 EM
		:temp(112)
		:semi_major_axis(f(121,100000))
		:eccentricity(f(3,1000))
		:inclination(math.deg2rad(0.374))
		:rotation_period(f(498179,1000000))
		:metallicity(f(7,10))
		:volcanicity(f(1,1)),


	CustomSystemBody:new('Thebe', 'PLANET_ASTEROID')
		:seed(-989982)
		:radius(f(773,100000))
		:mass(f(72,1000000000)) -- 43e16 kg = 7.2e-8 EM
		:temp(124)
		:semi_major_axis(f(148,100000))
		:eccentricity(f(175,10000))
		:inclination(math.deg2rad(1.076))
		:rotation_period(f(674536,1000000))
		:metallicity(f(7,10))
		:volcanicity(f(1,1)),
	{
		CustomSystemBody:new('Thebe Gas Refinery', 'STARPORT_SURFACE')
			:latitude(math.deg2rad(-0.1))
			:longitude(math.deg2rad(21.2)),
	},

	CustomSystemBody:new('Io', 'PLANET_TERRESTRIAL')
		:seed(-4955979)
		:radius(f(286,1000))
		:mass(f(15,1000))
		:temp(130)
		:semi_major_axis(f(282,100000))
		:eccentricity(f(41,10000))
		:inclination(math.deg2rad(2.21))
		:rotation_period(f(177,100))
		:metallicity(f(9,10))
		:volcanicity(f(7,10))
		:atmos_density(f(100,1225))
		:atmos_oxidizing(f(12,100))
		:ocean_cover(f(0,1))
		:ice_cover(f(0,1))
		:life(f(1,10)),
	{
		CustomSystemBody:new('Dante\'s Base', 'STARPORT_SURFACE')
			:latitude(math.deg2rad(-56.38))
			:longitude(math.deg2rad(9.51)),
	},
	CustomSystemBody:new('Europa', 'PLANET_TERRESTRIAL')
		:seed(2102431459)
		:radius(f(245,1000))
		:mass(f(8,1000))
		:temp(102)
		:semi_major_axis(f(441,100000))
		:eccentricity(f(9,1000))
		:rotation_period(f(355,100))
		:ocean_cover(f(9,10))
		:ice_cover(f(1,1))
		:atmos_density(f(7,100))
		:atmos_oxidizing(f(1,1))
		:metallicity(f(3,4))
		:volcanicity(f(0,1)),
	{
		CustomSystemBody:new('Clarke\'s Station', 'STARPORT_ORBITAL')
			:semi_major_axis(f(12,500000))
			:rotation_period(f(1,24*60*3)),
	},
	CustomSystemBody:new('Ganymede', 'PLANET_TERRESTRIAL')
		:seed(-1232011660)
		:radius(f(413,1000))
		:mass(f(25,1000))
		:temp(180)
		:atmos_oxidizing(f(1,1))
		:ocean_cover(f(3,10))
		:ice_cover(f(7,10))
		:metallicity(f(3,5))
		:semi_major_axis(f(72,10000))
		:eccentricity(f(13,10000))
		:inclination(math.deg2rad(0.2))
		:atmos_density(f(83,1000))
		:rotation_period(f(72,10)),
	{
		CustomSystemBody:new('Enki Catena', 'STARPORT_SURFACE')
			:latitude(math.deg2rad(84))
			:longitude(math.deg2rad(96)),
	},
	CustomSystemBody:new('Callisto', 'PLANET_TERRESTRIAL')
		:seed(1272712740)
		:radius(f(378,1000))
		:mass(f(18,1000))
		:temp(134)
		:metallicity(f(1,4))
		:semi_major_axis(f(126,10000))
		:eccentricity(f(74,10000))
		:inclination(math.deg2rad(0.192))
		:rotation_period(f(167,10)),

	CustomSystemBody:new('Discovery Base', 'STARPORT_ORBITAL')
		:semi_major_axis(f(14,1000))
		:rotation_period(f(204,10)),

	CustomSystemBody:new('Themisto', 'PLANET_ASTEROID')
		:seed(134102334)
		:radius(f(627,1000000))
		:mass(f(115,1000000000000)) -- 6.9e14 kg, 1.15e-10 EM
		:temp(124)
		:semi_major_axis(f(494,10000))
		:eccentricity(f(2006,10000))
		:inclination(math.deg2rad(47.48))
		:rotation_period(f(12982,100))
		:metallicity(f(2,1000))
		:volcanicity(f(0,1)),

	CustomSystemBody:new('Leda', 'PLANET_ASTEROID')
		:seed(-83484668)
		:radius(f(156,100000))
		:mass(f(837,1000000000000)) -- 5e15 kg, 8.37e-10 EM
		:temp(124)
		:semi_major_axis(f(745,10000))
		:eccentricity(f(16,100))
		:inclination(math.deg2rad(29.1))
		:rotation_period(f(12982,100))
		:metallicity(f(2,1000))
		:volcanicity(f(0,1))
		:orbital_phase_at_start(fixed.deg2rad(f(315,1))),

	CustomSystemBody:new('Himalia', 'PLANET_ASTEROID')
		:seed(1344978)
		:radius(f(1334,100000))
		:mass(f(1122,1000000000)) -- 670e16 kg, 1.122e-6 EM
		:temp(124)
		:semi_major_axis(f(766,10000))
		:eccentricity(f(16,100))
		:inclination(math.deg2rad(29.59))
		:rotation_period(f(12982,100))
		:metallicity(f(22,1000))
		:volcanicity(f(1,1))
		:orbital_phase_at_start(fixed.deg2rad(f(105,1))),

	CustomSystemBody:new('Lysithea', 'PLANET_ASTEROID')
		:seed(3934)
		:radius(f(282,100000))
		:mass(f(1055,100000000000)) -- 6.3e16 kg, 1.055e-8 EM
		:temp(124)
		:semi_major_axis(f(783,10000))
		:eccentricity(f(11,100))
		:inclination(math.deg2rad(25.77))
		:rotation_period(f(1292,100))
		:metallicity(f(2,1000))
		:volcanicity(f(0,1))
		:orbital_phase_at_start(fixed.deg2rad(f(12,1))),

	CustomSystemBody:new('Elara', 'PLANET_ASTEROID')
		:seed(128860219)
		:radius(f(675,100000))
		:mass(f(1457,10000000000)) -- 87e16 kg, 1.457e-7 EM
		:temp(124)
		:semi_major_axis(f(781,10000))
		:eccentricity(f(22,100))
		:inclination(math.deg2rad(30.66))
		:rotation_period(f(5,10))
		:metallicity(f(8,1000))
		:volcanicity(f(0,1))
		:orbital_phase_at_start(fixed.deg2rad(f(92,1))),

	CustomSystemBody:new('Aega', 'PLANET_ASTEROID')
		:seed(6953)
		:radius(f(313,1000000))
		:mass(f(1055,100000000000))
		:temp(113)
		:semi_major_axis(f(808,10000))
		:eccentricity(f(21,100))
		:inclination(math.deg2rad(28.2))
		:rotation_period(f(8,10))
		:metallicity(f(8,1000))
		:volcanicity(f(0,1))
		:orbital_phase_at_start(fixed.deg2rad(f(193,1))),

}

local saturn = CustomSystemBody:new('Saturn', 'PLANET_GAS_GIANT')
	:seed(174249538)
	:radius(f(9,1))
	:mass(f(95152,1000))
	:temp(134)
	:semi_major_axis(f(9582,1000))
	:eccentricity(f(557,10000))
	:inclination(math.deg2rad(2.485))
	:rotation_period(f(4,10))
	:axial_tilt(fixed.deg2rad(f(2673,100)))
	:rings(f(1298,1000), f(2383,1000), {0.435, 0.412, 0.335, 0.9})
	:orbital_phase_at_start(fixed.deg2rad(f(217,1)))

local saturn_moons = {
	CustomSystemBody:new('Tethys', 'PLANET_TERRESTRIAL')
		:radius(f(083,1000))
		:mass(f(103,1000000))
		:temp(86)
		:semi_major_axis(f(002,1000))
		:eccentricity(f(0,1))
		:inclination(math.deg2rad(1.12))
		:rotation_period(f(1887,1000)),
	CustomSystemBody:new('Dione', 'PLANET_TERRESTRIAL')
		:seed(-562018355)
		:radius(f(881,10000))
		:mass(f(328,1000000))
		:temp(87)
		:metallicity(f(1,2))
		:semi_major_axis(f(252,100000))
		:eccentricity(f(22,10000))
		:inclination(math.deg2rad(0.019))
		:rotation_period(f(2737,1000)),
	CustomSystemBody:new('Rhea', 'PLANET_TERRESTRIAL')
		:radius(f(12,100))
		:mass(f(406,100000000))
		:semi_major_axis(f(352,100000))
		:rotation_period(f(452,100))
		:temp(81)
		:eccentricity(f(126,100000))
		:inclination(math.deg2rad(0.345))
		:atmos_density(f(82,1000)),
	CustomSystemBody:new('Titan', 'PLANET_TERRESTRIAL')
		:seed(0)
		:radius(f(400,1000))
		:mass(f(225,10000))
		:temp(94)
		:semi_major_axis(f(82,10000))
		:eccentricity(f(288,10000))
		:inclination(math.deg2rad(0.34854))
		:atmos_density(f(122,10))
		:atmos_oxidizing(f(6,10))
		:rotation_period(f(15945,1000)),
	{
		CustomSystemBody:new('Oasis City', 'STARPORT_SURFACE')
			:latitude(math.deg2rad(-65.4))
			:longitude(math.deg2rad(83)),
		CustomSystemBody:new('Port Makenzie', 'STARPORT_SURFACE')
			:latitude(math.deg2rad(31))
			:longitude(math.deg2rad(121)),
		CustomSystemBody:new('Daniel\'s Haven', 'STARPORT_ORBITAL')
			:semi_major_axis(f(12,500000))
			:eccentricity(f(50,1000))
			:rotation_period(f(11,9)),
	},
	CustomSystemBody:new('Iapetus', 'PLANET_TERRESTRIAL')
		:seed(-644022643)
		:radius(f(1155,10000))
		:mass(f(3,10000))
		:temp(115)
		:metallicity(f(4,5))
		:semi_major_axis(f(238,10000))
		:eccentricity(f(29,1000))
		:inclination(math.deg2rad(15.47))
		:rotation_period(f(7932,100)),
	CustomSystemBody:new('Phoebe', 'PLANET_TERRESTRIAL')
		:seed(1740171277)
		:radius(f(17,1000))
		:mass(f(139,100000000))
		:temp(115)
		:semi_major_axis(f(87,1000))
		:eccentricity(f(156,1000))
		:inclination(math.deg2rad(151.78))
		:rotation_period(f(386,1000)),

}

local uranus = CustomSystemBody:new('Uranus', 'PLANET_GAS_GIANT')
	:seed(1365118445)
	:radius(f(4,1))
	:mass(f(145,10))
	:temp(76)
	:semi_major_axis(f(19229,1000))
	:eccentricity(f(444,10000))
	:inclination(math.deg2rad(0.772))
	:rotation_period(f(7,10))
	:axial_tilt(fixed.deg2rad(f(9777,100)))
	:rings(f(17528,10000), f(2,1), {0.51, 0.48, 0.384, 0.8})
	:orbital_phase_at_start(fixed.deg2rad(f(245,1)))

local uranus_moons = {
	CustomSystemBody:new('Ariel', 'PLANET_TERRESTRIAL')
		:radius(f(908,10000))
		:mass(f(226,1000000))
		:temp(60)
		:semi_major_axis(f(1277,1000000))
		:eccentricity(f(12,10000))
		:inclination(math.deg2rad(0.26))
		:rotation_period(f(252,100)),
	CustomSystemBody:new('Umbriel', 'PLANET_TERRESTRIAL')
		:radius(f(92,1000))
		:mass(f(2,10000))
		:temp(75)
		:semi_major_axis(f(178,100000))
		:eccentricity(f(39,10000))
		:inclination(math.deg2rad(0.128))
		:rotation_period(f(4144,1000)),
	CustomSystemBody:new('Titania', 'PLANET_TERRESTRIAL')
		:radius(f(1235,10000))
		:mass(f(5908,10000000))
		:temp(70)
		:semi_major_axis(f(2913,1000000))
		:eccentricity(f(11,10000))
		:inclination(math.deg2rad(0.34))
		:rotation_period(f(87,10))
		:atmos_density(f(82,1000)),
	CustomSystemBody:new('Oberon', 'PLANET_TERRESTRIAL')
		:radius(f(1194,10000))
		:mass(f(5046,10000000))
		:temp(75)
		:semi_major_axis(f(39,10000))
		:eccentricity(f(14,10000))
		:inclination(math.deg2rad(0.058))
		:rotation_period(f(135,10)),
}

local neptune = CustomSystemBody:new('Neptune', 'PLANET_GAS_GIANT')
	:seed(1365118457)
	:radius(f(38,10))
	:mass(f(17147,1000))
	:temp(72)
	:semi_major_axis(f(30104,1000))
	:eccentricity(f(112,10000))
	:inclination(math.deg2rad(1.768))
	:rotation_period(f(75,100))
	:axial_tilt(fixed.deg2rad(f(2832,100)))
	:rings(f(2195,1000), f(236,100), {0.71, 0.68, 0.684, 0.75})
	:orbital_phase_at_start(fixed.deg2rad(f(353,1)))

local neptune_moons = {
	CustomSystemBody:new('Proteus', 'PLANET_ASTEROID')
		:seed(1251043226)
		:metallicity(f(7,10))
		:radius(f(310,10000))
		:mass(f(843,100000000)) -- 5035e16 kg, 8.43e-6 EM
		:temp(51)
		:semi_major_axis(f(786,1000000))
		:eccentricity(f(53,100000))
		:inclination(math.deg2rad(0.524))
		:rotation_period(f(1122,1000)),
	CustomSystemBody:new('Triton', 'PLANET_TERRESTRIAL')
		:radius(f(2122,10000))
		:mass(f(359,100000))
		:temp(38)
		:semi_major_axis(f(2371,100000))
		:eccentricity(f(16,1000000))
		:volcanicity(f(3,10)) -- Cryovolcanos!
		:inclination(math.deg2rad(156.885))
		:rotation_period(f(141,24))
		:atmos_density(f(100,1225))
		:atmos_oxidizing(f(1,20)),
	{
		CustomSystemBody:new('Poseidon Station', 'STARPORT_ORBITAL')
			:semi_major_axis(f(12,500000))
			:rotation_period(f(11,7)),
	},
	CustomSystemBody:new('Nereid', 'PLANET_ASTEROID')
		:radius(f(2668,100000))
		:mass(f(452,100000000)) -- 2700e16 kg, 4.52e-6 EM
		:temp(50)
		:semi_major_axis(f(3685,100000))
		:eccentricity(f(75,100))
		:inclination(math.deg2rad(32.55))
		:rotation_period(f(115,240)),
}

local pluto = CustomSystemBody:new('Pluto', 'PLANET_TERRESTRIAL')
	:radius(f(18,100))
	:mass(f(21,10000))
	:temp(44)
--	:atmos_oxidizing(f(66,100))
	:semi_major_axis(f(394,10))
	:eccentricity(f(249,1000))
	:inclination(math.deg2rad(11.88))
	:rotation_period(f(153,24))
	:axial_tilt(fixed.deg2rad(f(296,10)))
	:orbital_phase_at_start(fixed.deg2rad(f(315,1)))

local pluto_starports = {
	CustomSystemBody:new('Pluto Research Base', 'STARPORT_SURFACE')
		:latitude(math.deg2rad(84))
		:longitude(math.deg2rad(96)),
}

local charon = {
	CustomSystemBody:new('Charon', 'PLANET_TERRESTRIAL')
		:radius(f(9,100))
		:mass(f(2,10000))
		:temp(44)
		:semi_major_axis(f(1172,10000000))
		:eccentricity(f(220,10000))
		:inclination(math.deg2rad(119.5))
		:rotation_period(f(6,10))
		:axial_tilt(fixed.deg2rad(f(668,100)))
		:volcanicity(f(0,1)),
}

s:bodies(sol, {
	mercury,
	venus,
	earth,
		earth_starports,
		moon,
	mars,
		mars_starports,
		mars_moons,
	eros,
	pallas,
	vesta,
	ceres,
	jupiter,
		jupiter_moons,
	saturn,
		saturn_moons,
	uranus,
		uranus_moons,
	neptune,
		neptune_moons,
	pluto,
		pluto_starports,
		charon,
})

s:add_to_sector(0,0,0,v(0.001,0.001,0.001))
