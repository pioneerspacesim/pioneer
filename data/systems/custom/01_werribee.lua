-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local s = CustomSystem:new("Werribee",{'STAR_G'})
	:govtype('EARTHMILDICT')
	:short_desc('Protected Nature Reserve')
	:long_desc([[Founded as a reserve for species indiginous to Australia on Earth,
Werribee enjoys protection from the Earth military against poachers.

Since 3140, this system has been the only place where some of the most
beautiful and dangerous life forms from Earth live outside of captivity.]])

local werribee = CustomSystemBody:new("Werribee",'STAR_G')
   :radius(f(11,10))
   :mass(f(11,10))
   :temp(5750)

local tasmania = CustomSystemBody:new('Tasmania', 'PLANET_TERRESTRIAL')
   :seed(1)
   :radius(f(3,100))
   :mass(f(20,1000))
   :temp(530)
   :semi_major_axis(f(12,100))
   :eccentricity(f(2,10))
   :rotation_period(f(14,1))

local newaustralia = CustomSystemBody:new('New Australia', 'PLANET_TERRESTRIAL')
   :radius(f(104,100))
   :mass(f(108,100))
   :temp(290)
   :semi_major_axis(f(102,100))
   :eccentricity(f(2,100))
   :rotation_period(f(9,10))
   :axial_tilt(fixed.deg2rad(f(2344,100)))
	:metallicity(f(1,2))
	:volcanicity(f(1,10))
	:atmos_density(f(1,1))
	:atmos_oxidizing(f(8,10))
	:ocean_cover(f(6,10))
	:ice_cover(f(7,10))
	:life(f(8,10))

local ozzy_ports = {
   CustomSystemBody:new('Norris','STARPORT_SURFACE')
      :latitude(math.deg2rad(0))
		:longitude(math.deg2rad(0)),
   CustomSystemBody:new('Cook', 'STARPORT_SURFACE')
      :latitude(math.deg2rad(5))
		:longitude(math.deg2rad(170)),
   CustomSystemBody:new('Immigration', 'STARPORT_ORBITAL')
      :semi_major_axis(f(102,100000))
	   :rotation_period(f(1,24*60*3)),
}

local yanada = {
	CustomSystemBody:new('Yanada', 'PLANET_TERRESTRIAL')
		:seed(191080)
		:radius(f(283,1000))
		:mass(f(11,1000))
		:temp(210)
		:semi_major_axis(f(268,100000))
		:eccentricity(f(500,10000))
		:inclination(math.deg2rad(5.0))
		:rotation_period(f(199,10))
		:volcanicity(f(0,1)),
}

local uluru = CustomSystemBody:new('Uluru', 'PLANET_GAS_GIANT')
   :seed(4)
   :radius(f(24,1))
   :mass(f(1008,1))
   :semi_major_axis(f(854,100))
   :rotation_period(f(14,1))

local ayers_rocks = {
   CustomSystemBody:new('Francesca', 'PLANET_TERRESTRIAL')
      :mass(f(9,1000))
      :radius(f(125,1000))
      :semi_major_axis(f(217,100000))
      :rotation_period(f(68,10))
      :inclination(math.deg2rad(48.3))
      :volcanicity(f(1,1)),
   CustomSystemBody:new('Penny', 'PLANET_TERRESTRIAL')
      :mass(f(17,1000))
      :radius(f(185,1000))
      :semi_major_axis(f(314,100000))
      :rotation_period(f(19,10)),
   CustomSystemBody:new('Beth', 'PLANET_TERRESTRIAL')
      :mass(f(22,1000))
      :radius(f(225,1000))
      :semi_major_axis(f(481,100000))
      :rotation_period(f(8,10))
      :atmos_density(f(10,6)),
}

local olga = CustomSystemBody:new('Kata Tjuta', 'PLANET_GAS_GIANT')
   :seed(4)
   :radius(f(39,10))
   :mass(f(18,1))
   :semi_major_axis(f(1811,100))
   :eccentricity(f(3,10))
   :inclination(math.deg2rad(3.4))
   :rotation_period(f(26,1))

s:bodies(werribee, {
   tasmania, -- If we forget it, they'll be so pissed...
   newaustralia, ozzy_ports, yanada,
   uluru, ayers_rocks,
   olga,
})

s:add_to_sector(22,4,11,v(0.179,0.526,0.285))
