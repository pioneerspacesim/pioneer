local s = CustomSystem:new('Epsilon Eridani', { 'STAR_K' })
	:govtype('CISLIBDEM')
	:short_desc('First off-earth colony. Industrial world with indigenous life.')
	:long_desc([[Epsilon Eridani was the first star system beyond Sol to be colonised by humanity. The New Hope colony on the life-bearing planet of the same name was founded in 2279. Its 1520 initial inhabitants completed their pre-hyperspace voyage of 10.7 lightyears from Sol in just under 25 years.
Mass emigration from Earth in the 27th century drove a population explosion and today Epsilon Eridani counts itself among the most populous of inhabited systems.
The system's history has been marked by political friction between Epsilon Eridani and the Earth government. This began with the advent of hyperspace around the end of the 26th century. While previously the communications lag of 20 years had prevented exertion of Earth's power, suddenly the rulers of Epsilon Eridani found themselves constantly subject to the interference of Earth.
This conflict flared up in 2714 when the pro-Earth president of Epsilon Eridani was toppled amid strikes and civil disorder over the unfair tax and trade conditions imposed by Earth. The 'Free Republic' then established survived nine months until Earth rule was re-imposed by force, including the notorious use of orbital lasers on population centres.
Independence was not finally won until the wars of the 30th century, and the formation of the Confederation of Independent Worlds, of which Epsilon Eridani was a founding member.
Epsilon Eridani is today a thriving centre of industry, cutting-edge technology and tourism.
Reproduced with the kind permission of Enrique Watson, New Hope University, 2992]])


local epserid = CustomSBody:new('Epsilon Eridani', 'STAR_K')
	:radius(f(7,10))
	:mass(f(61,110))
	:temp(4584)

local icarus = CustomSBody:new('Icarus', 'PLANET_TERRESTRIAL')
	:seed(13)
	:radius(f(42,100))
	:mass(f(41,100))
	:temp(687)
	:semi_major_axis(f(17,1000))
	:eccentricity(f(205,1000))
	:inclination(math.deg2rad(7.0))
	:rotation_period(f(12,1))
	:axial_tilt(math.fixed.deg2rad(f(1,100)))
	:metallicity(f(98,100))
	:volcanicity(f(52,100))
	:atmos_density(f(21,100))
	:atmos_oxidizing(f(2,10))
	:ocean_cover(f(0,1))
	:ice_cover(f(0,100))
	:life(f(0,1))

local atlantica = CustomSBody:new('Atlantica', 'PLANET_TERRESTRIAL')
	:seed(8)
	:radius(f(245,100))
	:mass(f(315,100))
	:temp(328)
	:semi_major_axis(f(793,1000))
	:eccentricity(f(487,1000))
	:inclination(math.deg2rad(3.09))
	:rotation_period(f(243,1))
	:axial_tilt(math.fixed.deg2rad(f(26,10)))
	:metallicity(f(5,6))
	:volcanicity(f(6,10))
	:atmos_density(f(9,1))
	:atmos_oxidizing(f(1,1))
	:ocean_cover(f(8,10))
	:ice_cover(f(0,1))
	:life(f(11,100))


local newhope = CustomSBody:new('New Hope', 'PLANET_TERRESTRIAL')
	:seed(43)
	:radius(f(4,3))
	:mass(f(5,4))
	:temp(287)
	:semi_major_axis(f(9,10))
	:eccentricity(f(367,10000))
	:rotation_period(f(4,6))
	:axial_tilt(math.fixed.deg2rad(f(1741,100)))
	:metallicity(f(5,6))
	:volcanicity(f(68,100))
	:atmos_density(f(15,10))
	:atmos_oxidizing(f(7,10))
	:ocean_cover(f(45,100))
	:ice_cover(f(6,10))
	:life(f(9,10))
	
	local newhope_starports = {
	CustomSBody:new('New Hope', 'STARPORT_SURFACE')
		:latitude(math.deg2rad(31))
		:longitude(math.deg2rad(-121)),
	CustomSBody:new("Gandhi's Revenge", 'STARPORT_SURFACE')
		:latitude(math.deg2rad(19))
		:longitude(math.deg2rad(99)),
	CustomSBody:new('Epsilon Cove', 'STARPORT_SURFACE')
		:latitude(math.deg2rad(51))
		:longitude(0),
	CustomSBody:new('Eridani Commercial Center', 'STARPORT_ORBITAL')
		:semi_major_axis(f(9068,100000000))
		:rotation_period(f(11,24)),
	}
	
	local hades = {
	CustomSBody:new('Hades', 'PLANET_TERRESTRIAL')
		:seed(191082)
		:radius(f(484,1000))
		:mass(f(121,1000))
		:temp(280)
		:semi_major_axis(f(27,100000))
		:eccentricity(f(349,1000))
		:inclination(math.deg2rad(5.145))
		:rotation_period(f(4,10))
		:axial_tilt(math.fixed.deg2rad(f(668,100)))
		:volcanicity(f(9,10))
		:atmos_density(f(1,10)),
	{
		CustomSBody:new('Eagles Nest', 'STARPORT_ORBITAL')
		:semi_major_axis(f(5068,100000000))
		:rotation_period(f(11,24)),
	},
	}
	
local hercules = CustomSBody:new('Hercules', 'PLANET_GAS_GIANT')
	:radius(f(14,1))
	:mass(f(8115,10))
	:temp(134)
	:semi_major_axis(f(4582,1000))
	:eccentricity(f(488,10000))
	:inclination(math.deg2rad(1.305))
	:rotation_period(f(4,10))
	:axial_tilt(math.fixed.deg2rad(f(313,100)))
	
	local hale = {
	CustomSBody:new('Halee', 'PLANET_TERRESTRIAL')
		:seed(14782)
		:radius(f(317,1000))
		:mass(f(117,1000))
		:temp(135)
		:semi_major_axis(f(457,100000))
		:eccentricity(f(92,1000))
		:inclination(math.deg2rad(5.145))
		:rotation_period(f(9,10))
		:axial_tilt(math.fixed.deg2rad(f(668,100)))
		:volcanicity(f(6,10))
		:atmos_density(f(1,15))
		:ocean_cover(f(4,10))
		:ice_cover(f(9,10))
	}
	
	s:bodies(epserid, {
	icarus,
	atlantica,
	newhope,
		newhope_starports,
		hades,
	hercules,
		hale,
	})

s:add_to_sector(1,-1,-1,v(0.037,0.325,0.784))
