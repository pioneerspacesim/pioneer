-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local s = CustomSystem:new('Star\'s End', { 'STAR_K' })
	:govtype('CORPORATE')
	:lawlessness(f(1,100)) -- 1/100th from a peaceful eden
	:short_desc('Human enclave at the opposite end of the galaxy.')
	:long_desc([[A special team of ambitious colonists sent out during the fall of Haber Corporation in secrecy has picked this small system as their new home.
	With intentions to expand the civilization to the remotest reach of Milky Way, they are safekeeping perhaps one of the most detailed records of
	human history and culture.

	The colonists are currently working on increasing the total population and terraforming the two planets in the system before permanently heading off to colonize
	neighboring star systems.]])

local starsend = CustomSystemBody:new('Star\'s End', 'STAR_K')
	:radius(f(7,10))
	:mass(f(61,110))
	:temp(4400)

local kobol = CustomSystemBody:new('Cobol', 'PLANET_TERRESTRIAL')
	:radius(f(92,100))
	:mass(f(9,10))
	:temp(260)
	:semi_major_axis(f(82,100))
	:eccentricity(f(183,10000))
	:rotation_period(f(86,100))
	:axial_tilt(fixed.deg2rad(f(3145,100)))
	:rotational_phase_at_start(fixed.deg2rad(f(170,1)))
	:metallicity(f(4,5))
	:volcanicity(f(3,10))
	:atmos_density(f(9,10))
	:atmos_oxidizing(f(72,100))
	:ocean_cover(f(0,10))
	:ice_cover(f(1,10))
	:life(f(2,10))
	:orbital_phase_at_start(fixed.deg2rad(f(336,1)))
	:rings(false)

local kobol_starports = {
	CustomSystemBody:new('Exodus', 'STARPORT_SURFACE')
		:latitude(math.deg2rad(55))
		:longitude(math.deg2rad(-37.5)),
}

local dagobah = CustomSystemBody:new('Yodagobah', 'PLANET_TERRESTRIAL')
	:seed(-1317315059)
	:radius(f(412,1000))
	:mass(f(189,1000))
	:temp(213)
	:semi_major_axis(f(160,100))
	:eccentricity(f(1079,10000))
	:inclination(math.deg2rad(0.92))
	:rotation_period(f(712,1000))
	:axial_tilt(fixed.deg2rad(f(1045,100)))
	:metallicity(f(44,50))
	:volcanicity(f(8,10))
	:atmos_density(f(212,1000))
	:atmos_oxidizing(f(2,100))
	:ocean_cover(f(0,100))
	:ice_cover(f(20,1000))
	:life(f(3,100))
	:orbital_phase_at_start(fixed.deg2rad(f(12,1)))
	:rings(false)

s:bodies(starsend, {
	kobol,
		kobol_starports,
	dagobah,
})

s:add_to_sector(-8615,0,0,v(0.001,0.001,0.001))
