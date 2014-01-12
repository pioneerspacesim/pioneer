-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local s = CustomSystem:new('Black Hole', { 'STAR_SM_BH' })
	:govtype('CISLIBDEM')
	:short_desc('The End of the line.')
	:long_desc([[This is what holds it all together baby, Yeah!]])


local agamemnon = CustomSystemBody:new('Agamemnon', 'STAR_SM_BH')
	:radius(f(10000,1))
	:mass(f(999999999,1))


local terminus = CustomSystemBody:new('Terminus', 'PLANET_TERRESTRIAL')
	:seed(142)
	:radius(f(14,3))
	:mass(f(24,4))
	:semi_major_axis(f(1000,1))
	:eccentricity(f(67,100))
	:rotation_period(f(1,10))
	:axial_tilt(fixed.deg2rad(f(1741,100)))
	:metallicity(f(1,1))
	:volcanicity(f(9,10))
	:atmos_density(f(3,10))
	:atmos_oxidizing(f(7,10))
	:ocean_cover(f(3,10))
	:ice_cover(f(0,10))
	:life(f(0,10))

	local terminus_starports = {
	CustomSystemBody:new('End of the Line', 'STARPORT_ORBITAL')
		:semi_major_axis(f(5,100))
		:rotation_period(f(11,24)),
	CustomSystemBody:new('Tom Morton Research Institute', 'STARPORT_ORBITAL')
		:semi_major_axis(f(6,100))
		:rotation_period(f(11,24)),
	}

	local melon = {
	CustomSystemBody:new('Melon', 'PLANET_TERRESTRIAL')
		:seed(191082)
		:radius(f(12,10))
		:mass(f(24,10))
		:temp(278)
		:semi_major_axis(f(5,1000))
		:eccentricity(f(97,1000))
		:inclination(math.deg2rad(5.145))
		:rotation_period(f(4,10))
		:axial_tilt(fixed.deg2rad(f(668,100)))
		:volcanicity(f(3,10))
		:atmos_density(f(11,10))
		:ocean_cover(f(5,10))
		:ice_cover(f(9,10))
		:life(f(4,10)),
	{
		CustomSystemBody:new('Strawberry Base', 'STARPORT_ORBITAL')
		:semi_major_axis(f(1,10000))
		:rotation_period(f(11,24)),
	},
	}


	s:bodies(agamemnon, {
	terminus,
		terminus_starports,
		melon,
	})

s:add_to_sector(-3125,0,0,v(0.5,0.5,0.0))
