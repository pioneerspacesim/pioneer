-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local s = CustomSystem:new('Barnard\'s star',{ 'STAR_M' })
	:faction('Solar Federation')
	:govtype('EARTHCOLONIAL')
	:lawlessness(f(0,100)) -- 1/100th from a peaceful eden
	:short_desc('Earth Federation prison colony')
	:long_desc([[Barnard's Star is a very low-mass red dwarf star.  Somewhere between 7 and 12 billion years old, it is probably one of the most ancient stars in the galaxy.  Despite that, it is still fairly active.  Pilots entering the system are warned that there might be considerable stellar activity, including flares and massive coronal ejections.

One of the first stars to be visited after the introduction of interstellar travel, Barnard's Star was found to be a much smaller system compared to Sol, with only twelve bodies. Named after the Twelve Causes of Suffering from Buddhist suttas.  Habitats were built here to serve as Federal prison colonies.

A permit is normally required in order to enter this system whilst carrying weapons.]])

local barnard = CustomSystemBody:new('Barnard\'s Star', 'STAR_M')
	:radius(f(17,100))
	:mass(f(16,100))
	:temp(3134)

local formations = CustomSystemBody:new('Formations', 'PLANET_TERRESTRIAL')
	:seed(42)
	:radius(f(77,100))
	:mass(f(4,10))
	:temp(279)
	:semi_major_axis(f(108,1000))
	:eccentricity(f(71,100))
	:inclination(math.deg2rad(5.64))
	:rotation_period(f(9,10))
	:axial_tilt(fixed.deg2rad(f(311,10)))
	:metallicity(f(7,10))
	:volcanicity(f(3,10))
	:atmos_density(f(82,100))
	:atmos_oxidizing(f(2,10))
	:ocean_cover(f(3,10))
	:ice_cover(f(2,100))
	:life(f(1,100))
	:orbital_phase_at_start(fixed.deg2rad(f(138,1)))
local formations_starport =	{
		CustomSystemBody:new('OPLI Pax', 'STARPORT_SURFACE')
			:latitude(math.deg2rad(5.7541))
			:longitude(math.deg2rad(4.14))
	}

local ignorance = CustomSystemBody:new('Ignorance', 'PLANET_TERRESTRIAL')
	:seed(42312)
	:radius(f(566,1000))
	:mass(f(13,100))
	:temp(226)
	:semi_major_axis(f(278,1000))
	:eccentricity(f(66,100))
	:inclination(math.deg2rad(1.65))
	:rotation_period(f(66,100))
	:axial_tilt(fixed.deg2rad(f(193,10)))
	:metallicity(f(53,100))
	:volcanicity(f(1,10))
	:atmos_density(f(393,1000))
	:atmos_oxidizing(f(1,10))
	:ocean_cover(f(3,10))
	:ice_cover(f(93,100))
	:life(f(43,1000))
	:orbital_phase_at_start(fixed.deg2rad(f(261,1)))
	:orbital_offset(fixed.deg2rad(f(144,1)))

	local death = CustomSystemBody:new('Death', 'PLANET_TERRESTRIAL')
	:seed(-1322064465)
	:radius(f(273,10000))
	:mass(f(145,1000000))
	:temp(87)
	:semi_major_axis(f(101,100))
	:eccentricity(f(43,100))
	:inclination(math.deg2rad(187.77))
	:rotation_period(f(234,100))
	:axial_tilt(fixed.deg2rad(f(345,10)))
	:metallicity(f(53,100))
	:volcanicity(f(1,10))
	:atmos_density(f(0,1000))
	:atmos_oxidizing(f(0,10))
	:ocean_cover(f(0,10))
	:ice_cover(f(93,1))
	:life(f(13,10000))
	:orbital_phase_at_start(fixed.deg2rad(f(161,1)))
	:orbital_offset(fixed.deg2rad(f(32,1)))

local consciousness = {
	CustomSystemBody:new('Consciousness', 'PLANET_TERRESTRIAL')
		:seed(1913859659)
		:radius(f(273,10000))
		:mass(f(145,10000000))
		:temp(87)
		:semi_major_axis(f(823,1000000))
		:eccentricity(f(5,10))
		:inclination(math.deg2rad(24.9))
		:rotation_period(f(202,1))
		:axial_tilt(fixed.deg2rad(f(321,100)))
		:orbital_phase_at_start(fixed.deg2rad(f(102,1)))
		:rotational_phase_at_start(fixed.deg2rad(f(93,1)))
		:volcanicity(f(0,1)),
	}

local impression = CustomSystemBody:new('Impression', 'PLANET_GAS_GIANT')
	:seed(786424627)
	:radius(f(934,100))
	:mass(f(137,10))
	:temp(416)
	:atmos_density(f(114,100))
	:atmos_oxidizing(f(9,10))
	:semi_major_axis(f(32,10))
	:rotation_period(f(42,10))
	:axial_tilt(fixed.deg2rad(f(21,100)))
	:rings(f(11176,10000), f(11769,10000), {0.61, 0.48, 0.384, 0.8})
	:orbital_offset(fixed.deg2rad(f(60,1)))
	:eccentricity(f(120,10000))

local impression_moons = {
		CustomSystemBody:new('Name and Form', 'PLANET_ASTEROID')
		:seed(-9812342)
		:radius(f(137,10000))
		:mass(f(81,1000000))
		:temp(121)
		:semi_major_axis(f(801,1000000))
		:eccentricity(f(212,1000000))
		:inclination(math.deg2rad(7.17))
		:rotation_period(f(9701,10000))
		:metallicity(f(51,10000))
		:volcanicity(f(0,1))
		:ice_cover(f(100,1)),
		{
		CustomSystemBody:new('OPLI Contemplation', 'STARPORT_SURFACE')
			:latitude(math.deg2rad(12.1))
			:longitude(math.deg2rad(67.53))
	},

	CustomSystemBody:new('Six sense spheres', 'PLANET_ASTEROID')
		:seed(-16313981)
		:radius(f(395,10000))
		:mass(f(91,1000000))
		:temp(122)
		:semi_major_axis(f(714,1000000))
		:eccentricity(f(19,10000))
		:inclination(math.deg2rad(1.03))
		:rotation_period(f(977,1000))
		:metallicity(f(7,10))
		:volcanicity(f(1,1)),
		{
		CustomSystemBody:new('OPLI Awakening', 'STARPORT_SURFACE')
			:latitude(math.deg2rad(5.7541))
			:longitude(math.deg2rad(4.14))
	},

	CustomSystemBody:new('Sensation', 'PLANET_ASTEROID')
		:seed(-9981242)
		:radius(f(134,10000))
		:mass(f(3,1000000))
		:temp(112)
		:semi_major_axis(f(112,100000))
		:eccentricity(f(3,1000))
		:inclination(math.deg2rad(3.14))
		:rotation_period(f(498179,1000000))
		:metallicity(f(6,10))
		:volcanicity(f(1,1)),

	CustomSystemBody:new('Longings', 'PLANET_ASTEROID')
		:seed(-1146497004)
		:radius(f(2331,1000000))
		:mass(f(42,1000000000))
		:temp(124)
		:semi_major_axis(f(131,100000))
		:eccentricity(f(4,10000))
		:inclination(math.deg2rad(4.076))
		:rotation_period(f(674536,1000000))
		:metallicity(f(7,10))
		:volcanicity(f(1,1)),

		CustomSystemBody:new('Clinging', 'PLANET_ASTEROID')
		:seed(515932)
		:radius(f(173,100000))
		:mass(f(22,1000000000))
		:temp(124)
		:semi_major_axis(f(119,100000))
		:eccentricity(f(12,10000))
		:inclination(math.deg2rad(7.076))
		:rotation_period(f(674536,1000000))
		:metallicity(f(9,10))
		:volcanicity(f(1,1)),
		{
		CustomSystemBody:new('OPLI Wisdom', 'STARPORT_SURFACE')
			:latitude(math.deg2rad(35.71))
			:longitude(math.deg2rad(41.4))
		},

		CustomSystemBody:new('Emotion', 'PLANET_ASTEROID')
		:seed(-989983562342)
		:radius(f(133,100000))
		:mass(f(28,1000000000))
		:temp(124)
		:semi_major_axis(f(125,100000))
		:eccentricity(f(62,10000))
		:inclination(math.deg2rad(7.076))
		:rotation_period(f(674536,1000000))
		:metallicity(f(7,10))
		:volcanicity(f(1,1)),

	}
	local impression_station = {
		CustomSystemBody:new('System Administration Resting', 'STARPORT_ORBITAL')
		:semi_major_axis(f(233,100000))
		:rotation_period(f(1,24*60*3))
		:orbital_phase_at_start(fixed.deg2rad(f(0,1)))
		:axial_tilt(fixed.deg2rad(f(668,100))),
	}


	local tranquility = CustomSystemBody:new('High Security Prison Tranquility', 'STARPORT_ORBITAL')
	:semi_major_axis(f(32,10))
	:rotation_period(f(1,24*60*3))
	:orbital_offset(fixed.deg2rad(f(0,1)))
	:eccentricity(f(120,10000))

	local serenity = CustomSystemBody:new('High Security Prison Serenity', 'STARPORT_ORBITAL')
	:semi_major_axis(f(32,10))
	:orbital_offset(fixed.deg2rad(f(120,1)))
	:rotation_period(f(1,24*120*4))
	:eccentricity(f(120,10000))

	local birth = CustomSystemBody:new('Birth', 'PLANET_TERRESTRIAL')
	:seed(22312521)
	:radius(f(229,1000))
	:mass(f(9,1000))
	:temp(20)
	:semi_major_axis(f(953,100))
	:eccentricity(f(56,100))
	:inclination(math.deg2rad(14.5))
	:rotation_period(f(42,100))
	:axial_tilt(fixed.deg2rad(f(76,1)))
	:metallicity(f(53,100))
	:volcanicity(f(3,100))
	:atmos_density(f(0,1))
	:atmos_oxidizing(f(0,10))
	:ocean_cover(f(0,10))
	:ice_cover(f(12,100))
	:life(f(0,1000))
	:orbital_phase_at_start(fixed.deg2rad(f(21,1)))
	:orbital_offset(fixed.deg2rad(f(351,10)))

s:bodies(barnard, {

	formations,
		formations_starport,
	ignorance,
		consciousness,
	death,
	impression,
		impression_moons,
		impression_station,
	tranquility,
	serenity,
	birth,
	})
s:add_to_sector(-1,0,0,v(0.260,0.007,0.060))
