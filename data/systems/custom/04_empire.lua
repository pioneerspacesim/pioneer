-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- create a custom system
local sys = CustomSystem:new('Achernar',{'STAR_B'})
	:govtype('EMPIRERULE')
	:short_desc('Imperial seat of the Empire')
	:long_desc([[...mostly, they raise chickens, giant low-gravity chickens]])

-- Achernar - Type 'B' hot blue star
-- Mass: 7186786.11 Earth masses (== 21.60733079 solar masses)
-- Surface temp: 20000 deg C
local Achernar = CustomSystemBody:new("Achernar",'STAR_B')
   :seed(5437)
   :radius(f(2479,1000))
   :mass(f(2161,100))
   :temp(20273)

-- Achernar1 - rocky planet with a thin atmosphere
-- Mass: 0.15 Earth masses
-- Surface temp: 1208 deg C
-- Orbital period: 146 days (0.4 years)
-- Orbital radius: 1.514AU
-- Orbit Ecc. and Incl.: 0.000, 357.5 deg C
local Achernar1 = CustomSystemBody:new('Achernar1', 'PLANET_TERRESTRIAL')
   :seed(9872)
   :radius(f(10,10))
   :mass(f(15,100))
   :temp(1481)
   :rotation_period(f(4,10))
   :semi_major_axis(f(1514,1000))
   :eccentricity(f(0,1))
   :inclination(math.deg2rad(357.5))
   :metallicity(f(1,10))
   :volcanicity(f(0,1))
   :atmos_density(f(1,10))
   :atmos_oxidizing(f(3,10))
   :ocean_cover(f(0,1))
   :ice_cover(f(0,1))
   :life(f(0,1))

-- Achernar2 - rocky planet with a thick corrosive atmosphere
-- Mass: 1.22 Earth masses
-- Surface temp: 1227 deg C
-- Orbital period: 316 days (0.8657 years)
-- Orbital radius: 2.528AU
-- Orbit Ecc. and Incl.: 0.000, 348.9 deg C
local Achernar2 = CustomSystemBody:new('Achernar2', 'PLANET_TERRESTRIAL')
   :seed(1862)
   :radius(f(84,10))
   :mass(f(122,100))
   :temp(1500)
   :rotation_period(f(866,1000))
   :semi_major_axis(f(2528,1000))
   :eccentricity(f(0,1))
   :inclination(math.deg2rad(348.9))
   :metallicity(f(7,10))
   :volcanicity(f(1,2))
   :atmos_density(f(0,1))
   :atmos_oxidizing(f(0,1))
   :ocean_cover(f(0,1))
   :ice_cover(f(2,100))
   :life(f(0,1))

-- Achernar3 - medium gas giant
-- Mass: 41.12 Earth masses
-- Surface temp: 611 deg C
-- Orbital period: 1.8 years
-- Orbital radius: 4.218AU
-- Orbit Ecc. and Incl.: 0.000, 0.2 deg C
local Achernar3 = CustomSystemBody:new('Achernar3', 'PLANET_GAS_GIANT')
   :seed(89372)
   :radius(f(142,100))
   :mass(f(4112,100))
   :temp(884)
   :rotation_period(f(18,10))
   :semi_major_axis(f(4218,1000))
   :eccentricity(f(0,1))
   :inclination(math.deg2rad(0.2))

-- Achernar4 - medium gas giant
-- Mass: 115.65 Earth masses
-- Surface temp: 262 deg C
-- Orbital period: 8.6 years
-- Orbital radius: 11.686AU
-- Orbit Ecc. and Incl.: 0.000, 7.4 deg C
local Achernar4 = CustomSystemBody:new('Achernar4', 'PLANET_GAS_GIANT')
   :seed(92174645)
   :radius(f(400,100))
   :mass(f(11565,100))
   :temp(535)
   :rotation_period(f(86,10))
   :semi_major_axis(f(11686,1000))
   :eccentricity(f(0,1))
   :inclination(math.deg2rad(7.4))

	-- Achernar4a - barren rocky planetoid
	-- Mass: 0.02 Earth masses
	-- Surface temp: 252 deg C
	-- Orbital period: 9 days (0.02465 years)
	-- Orbital radius: 0.006AU
	-- Orbit Ecc. and Incl.: 0.000, 0.2 deg C
	local Achernar4a = CustomSystemBody:new('Achernar4a', 'PLANET_TERRESTRIAL')
	   :seed(23567)
	   :radius(f(10,100))
	   :mass(f(2,100))
	   :temp(525)
	   :rotation_period(f(246,10000))
	   :semi_major_axis(f(6,100))
	   :eccentricity(f(0,1))
	   :inclination(math.deg2rad(0.2))
	   :metallicity(f(9,10))
	   :volcanicity(f(1,2))
	   :atmos_density(f(0,1))
	   :atmos_oxidizing(f(0,1))
	   :ocean_cover(f(0,1))
	   :ice_cover(f(2,100))
	   :life(f(0,1))

-- Achernar5 - Rocky world with a thick corrosive atmosphere
-- Mass: 0.69 Earth masses
-- Surface temp: 260 deg C
-- Orbital period: 37.5 years
-- Orbital radius: 31.162AU
-- Orbit Ecc. and Incl.: 0.000, 0.0 deg C
local Achernar5 = CustomSystemBody:new('Achernar5', 'PLANET_TERRESTRIAL')
   :seed(9364522)
   :radius(f(43,10))
   :mass(f(69,100))
   :temp(533)
   :rotation_period(f(375,10))
   :semi_major_axis(f(31162,1000))
   :eccentricity(f(0,1))
   :inclination(math.deg2rad(0.0))
   :metallicity(f(1,1))
   :volcanicity(f(1,1))
   :atmos_density(f(2,1))
   :atmos_oxidizing(f(8,10))
   :ocean_cover(f(0,1))
   :ice_cover(f(2,100))
   :life(f(0,1))

-- Achernar6 - Very large gas giant
-- Mass: 2173.91 Earth masses
-- Surface temp: 179 deg C
-- Orbital period: 77.8 years
-- Orbital radius: 50.654AU
-- Orbit Ecc. and Incl.: 0.000, 2.4 deg C
local Achernar6 = CustomSystemBody:new('Achernar6', 'PLANET_GAS_GIANT')
   :seed(08265528)
   :radius(f(7525,100))
   :mass(f(217391,100))
   :temp(452)
   :rotation_period(f(778,10))
   :semi_major_axis(f(50654,1000))
   :eccentricity(f(0,1))
   :inclination(math.deg2rad(2.4))

local Achernar6_moons = {
	-- Yamaha's Grave - barren rocky planetoid
	-- Mass: 0.02 Earth masses
	-- Surface temp: -21 deg C
	-- Orbital period: 20 days
	-- Orbital radius: 0.027AU
	-- Orbit Ecc. and Incl.: 0.000, 2.0 deg C
	CustomSystemBody:new('Yamaha\'s Grave', 'PLANET_TERRESTRIAL')
	   :seed(8977544)
	   :radius(f(10,100))
	   :mass(f(2,100))
	   :temp(252)
	   :rotation_period(f(547,10000))
	   :semi_major_axis(f(27,1000))
	   :eccentricity(f(0,1))
	   :inclination(math.deg2rad(2.0))
	   :metallicity(f(7,10))
	   :volcanicity(f(1,2))
	   :atmos_density(f(0,1))
	   :atmos_oxidizing(f(0,1))
	   :ocean_cover(f(0,1))
	   :ice_cover(f(2,100))
	   :life(f(0,1)),
	{
		-- Major Starport: Schmidt
		CustomSystemBody:new('Schmidt', 'STARPORT_SURFACE')
			:latitude(math.deg2rad(-0.5))
			:longitude(math.deg2rad(26.2)),

		-- Baker Terminal - Orbital trading post
		-- Type: Enclosed station
		-- Orbital period: 20 days
		-- Orbital radius: 0.000AU
		-- Orbit Ecc. and Incl.: 0.000, 0.0 deg C
		CustomSystemBody:new('Baker Terminal', 'STARPORT_ORBITAL')
			:semi_major_axis(f(12,500000))
			:eccentricity(f(0,1))
			:rotation_period(f(547,10000)),
	},


	-- New World - Small sustained terraformed world
	-- Mass: 0.20 Earth masses
	-- Surface temp: 22 deg C
	-- Major Starports: Hoopertown, Swallow Landing
	-- Orbital period: 70 days
	-- Orbital radius: 0.062AU
	-- Orbit Ecc. and Incl.: 0.000, 7.3 deg C
	CustomSystemBody:new('New World', 'PLANET_TERRESTRIAL')
	   :seed(348872)
	   :radius(f(13,10))
	   :mass(f(2,10))
	   :temp(295)
	   :rotation_period(f(1917,10000))
	   :semi_major_axis(f(62,1000))
	   :eccentricity(f(0,1))
	   :inclination(math.deg2rad(7.3))
	   :metallicity(f(8,10))
	   :volcanicity(f(1,2))
	   :atmos_density(f(1,1))
	   :atmos_oxidizing(f(1,1))
	   :ocean_cover(f(5,1))
	   :ice_cover(f(2,100))
	   :life(f(75,100)),
	{
		-- Major Starport: Hoopertown
		CustomSystemBody:new('Hoopertown', 'STARPORT_SURFACE')
			:latitude(math.deg2rad(-0.5))
			:longitude(math.deg2rad(26.2)),

		-- Major Starport: Swallow Landing
		CustomSystemBody:new('Swallow Landing', 'STARPORT_SURFACE')
			:latitude(math.deg2rad(187.3))
			:longitude(math.deg2rad(17.0)),

		-- Bell Terminal - Orbital city
		-- Orbital period: 13 hours
		-- Orbital radius: 0.000AU
		-- Orbit Ecc. and Incl.: 0.000, 0.0 deg C
		CustomSystemBody:new('Bell Terminal', 'STARPORT_ORBITAL')
			:semi_major_axis(f(14,50000))
			:eccentricity(f(0,1))
			:rotation_period(f(13,10000)),
	},

	-- Conversion - Terraformed world with introduced life
	-- Mass: 0.69 Earth masses
	-- Surface temp: 22 deg C
	-- Major Starports: Shepherd City, Newtown, Morris base, Chekovport
	-- Orbital period: 236 days
	-- Orbital radius: 0.139AU
	-- Orbit Ecc. and Incl.: 0.000, 358.9 deg C
	CustomSystemBody:new('Conversion', 'PLANET_TERRESTRIAL')
	   :seed(343212)
	   :radius(f(345,100))
	   :mass(f(69,100))
	   :temp(295)
	   :rotation_period(f(6465,10000))
	   :semi_major_axis(f(139,1000))
	   :eccentricity(f(0,1))
	   :inclination(math.deg2rad(358.9))
	   :metallicity(f(3,10))
	   :volcanicity(f(1,2))
	   :atmos_density(f(1,1))
	   :atmos_oxidizing(f(1,1))
	   :ocean_cover(f(3,10))
	   :ice_cover(f(3,100))
	   :life(f(9,10)),
	{
		-- Major Starport: Shepherd City
		CustomSystemBody:new('Shepherd City', 'STARPORT_SURFACE')
			:latitude(math.deg2rad(-0.5))
			:longitude(math.deg2rad(21.2)),

		-- Major Starport: Newtown
		CustomSystemBody:new('Newtown', 'STARPORT_SURFACE')
			:latitude(math.deg2rad(155.5))
			:longitude(math.deg2rad(11.0)),

		-- Major Starport: Morris base
		CustomSystemBody:new('Morris base', 'STARPORT_SURFACE')
			:latitude(math.deg2rad(45.3))
			:longitude(math.deg2rad(-37.0)),

		-- Major Starport: Chekovport
		CustomSystemBody:new('Chekovport', 'STARPORT_SURFACE')
			:latitude(math.deg2rad(-7.3))
			:longitude(math.deg2rad(81.0)),

		-- Macmillan Terminal - Orbital city
		-- Orbital period: 7 hours
		-- Orbital radius: 0.000AU
		-- Orbit Ecc. and Incl.: 0.000, 0.0 deg C
		CustomSystemBody:new('Macmillan Terminal', 'STARPORT_ORBITAL')
			:semi_major_axis(f(24,50000))
			:eccentricity(f(0,1))
			:rotation_period(f(7,10000)),
	},

	-- Capitol - World with indigenous life and oxygen atmosphere
	-- Mass: 0.55 Earth masses
	-- Surface temp: 19 deg C
	-- Major Starports: Duval City, Fortress Cambridge, Camp Denver, Maxwell's Camp
	-- Orbital period: 823 days
	-- Orbital radius: 0.320AU
	-- Orbit Ecc. and Incl.: 0.000, 5.2 deg C
	CustomSystemBody:new('Capitol', 'PLANET_TERRESTRIAL')
	   :seed(-1420311881)
	   :radius(f(285,100))
	   :mass(f(55,100))
	   :temp(292)
	   :rotation_period(f(22547,10000))
	   :semi_major_axis(f(320,1000))
	   :eccentricity(f(0,1))
	   :inclination(math.deg2rad(5.2))
	   :metallicity(f(3,10))
	   :volcanicity(f(1,2))
	   :atmos_density(f(1,1))
	   :atmos_oxidizing(f(1,1))
	   :ocean_cover(f(3,10))
	   :ice_cover(f(3,100))
	   :life(f(1,1)),
	{
		-- Major Starport: Duval City
		CustomSystemBody:new('Duval City', 'STARPORT_SURFACE')
			:latitude(math.deg2rad(-0.5))
			:longitude(math.deg2rad(21.2)),

		-- Major Starport: Fortress Cambridge
		CustomSystemBody:new('Fortress Cambridge', 'STARPORT_SURFACE')
			:latitude(math.deg2rad(155.5))
			:longitude(math.deg2rad(11.0)),

		-- Major Starport: Camp Denver
		CustomSystemBody:new('Camp Denver', 'STARPORT_SURFACE')
			:latitude(math.deg2rad(45.3))
			:longitude(math.deg2rad(-37.0)),

		-- Major Starport: Maxwell's Camp
		CustomSystemBody:new('Maxwell\'s Camp', 'STARPORT_SURFACE')
			:latitude(math.deg2rad(-7.3))
			:longitude(math.deg2rad(81.0)),

		-- Fort Donalds - Orbital city
		-- Type: Hammer4 station
		-- Orbital period: 8 hours
		-- Orbital radius: 0.000AU
		-- Orbit Ecc. and Incl.: 0.000, 0.0 deg C
		CustomSystemBody:new('Fort Donalds', 'STARPORT_ORBITAL')
			:semi_major_axis(f(24,50000))
			:eccentricity(f(0,1))
			:rotation_period(f(8,10000)),
	},

}

-- Achernar7 - Medium gas giant
-- Mass: 71.10 Earth masses
-- Surface temp: -69 deg C
-- Orbital period: 168.9 years
-- Orbital radius: 84.904AU
-- Orbit Ecc. and Incl.: 0.000, 6.3 deg C
local Achernar7 = CustomSystemBody:new('Achernar7', 'PLANET_GAS_GIANT')
   :seed(185266)
   :radius(f(242,100))
   :mass(f(7110,100))
   :temp(204)
   :rotation_period(f(1689,10))
   :semi_major_axis(f(84904,1000))
   :eccentricity(f(0,1))
   :inclination(math.deg2rad(6.3))

-- Achernar8 - Medium gas giant
-- Mass: 95.09 Earth masses
-- Surface temp: -147 deg C
-- Orbital period: 797.8 years
-- Orbital radius: 238.984AU
-- Orbit Ecc. and Incl.: 0.000, 351.2 deg C
local Achernar8 = CustomSystemBody:new('Achernar8', 'PLANET_GAS_GIANT')
   :seed(567987)
   :radius(f(329,100))
   :mass(f(9509,100))
   :temp(126)
   :rotation_period(f(7978,10))
   :semi_major_axis(f(238984,1000))
   :eccentricity(f(0,1))
   :inclination(math.deg2rad(351.2))

-- Achernar9 - Brown dwarf substellar object
-- Mass: 21745.37 Earth masses (==0.0653782 solar masses)
-- Surface temp: -1000 deg C
-- Orbital period: 1768.2 years
-- Orbital radius: 406.368AU
-- Orbit Ecc. and Incl.: 0.000, 354.5 deg C
local Achernar9 = CustomSystemBody:new('Achernar9', 'BROWN_DWARF')
   :seed(23492)
   :radius(f(7526,10000))
   :mass(f(6537,100000))
   :temp(5273)
   :semi_major_axis(f(406368,1000))
   :eccentricity(f(0,1))
   :inclination(math.deg2rad(354.5))

-- Achernar10 - Very large gas giant
-- Mass: 2293.86 Earth masses
-- Surface temp: -5 deg C
-- Orbital period: 3932.0 years
-- Orbital radius: 692.000AU
-- Orbit Ecc. and Incl.: 0.000, 5.2 deg C
local Achernar10 = CustomSystemBody:new('Achernar10', 'PLANET_GAS_GIANT')
   :seed(567987)
   :radius(f(7939,100))
   :mass(f(229386,100))
   :temp(268)
   :rotation_period(f(7978,10))
   :semi_major_axis(f(692000,1000))
   :eccentricity(f(0,1))
   :inclination(math.deg2rad(351.2))

-- Add the CustomSystemBody(ies) we've created to the CustomSystem
sys:bodies(Achernar, {
	Achernar1,
	Achernar2,
	Achernar3,
	Achernar4,
		{Achernar4a},
	Achernar5,
	Achernar6,
		Achernar6_moons,
	Achernar7,
	Achernar8,
	Achernar9,
	Achernar10
})

-- now add it to the sector etc
sys:add_to_sector(4,-9,-16,v(0.023,0.143,0.883))
