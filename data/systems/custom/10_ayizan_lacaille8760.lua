-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local s = CustomSystem:new('Ayizan',{'STAR_M'}):other_names({"Lacaille 8760"})
	:faction('Solar Federation')
	:govtype('EARTHCOLONIAL')
	:lawlessness(f(40,100)) -- More than the usual Earth Colonial system, because fo the guerilla
	:short_desc('Impoverished outdoor world')
	:long_desc([[Founded in 2312 by colony ship "Sunflower", Ayizan hosts one of the oldest human settlements of the space age. Gifted with planet Damballa’s lush vegetation, plentiful air and generous oceans, colonists thrived and established a successful civilization.
Following the teachings of philosopher Zephirin "Peace with our world, peace with ourselves", the citizens of Ayizan avoided strife and environmental damage often seen in other pre-hyperdrive colonies. They even kept all the planets in their system united behind their unique consitution.
When Sol used the new hyperspace technology to claim control over its former colonies in the 27th century, the Ayizans resisted bitterly what they saw as a leap back to the Dark Ages. Vastly overpowered, its government surrendered. However the armed forces turned into a guerilla, which succesfully struck even on Earth.
Ever since, the Solar Federation aimed to undermine economic and popular support for the Ayizan "terrorists". To that effect, the Federation enforced a brutal colonial rule and deterred economic development. 
This strict policy did result in turning Ayizan into the impoverished system it is now, providing the Federation with cheap food, consumer goods and workforce.
While Sol loosened its rule a few decades ago as a side effect of the detente between the Federation and the Commonwealth, the secessionist mood among the population and the diaspora remains especially high, the guerilla is active to this day.]])

--Names do not infringe any copyright
--Bodies are named after Haitian Voodoo Loas (spirits)
--Cities and orbital stations are named after Haitian personalities or locations


local ayizan = CustomSystemBody:new('Ayizan', 'STAR_M')
	:radius(f(51,100))
	:mass(f(41,100))
	:temp(3445)

local ayizana = CustomSystemBody:new('Ezili Danto', 'PLANET_TERRESTRIAL')
	:seed(-1225274654)
	:radius(f(859,10000))
	:mass(f(635,1000000))
	:temp(1146)
	:semi_major_axis(f(105,10000))
	:eccentricity(f(3,100))
	:inclination(math.deg2rad(0))
	:rotation_period(f(6,10))
	:axial_tilt(fixed.deg2rad(f(0,10)))
	:metallicity(f(249,1000))
	:volcanicity(f(0,1000))
	:atmos_density(f(0,1000))
	:atmos_oxidizing(f(0,10))
	:ocean_cover(f(0,1000))
	:ice_cover(f(0,1000))
	:life(f(0,1))	

local ayizanb = CustomSystemBody:new('Azaka', 'PLANET_TERRESTRIAL')
	:seed(-921350797)
	:radius(f(1791,10000))
	:mass(f(57,10000))
	:temp(848)
	:semi_major_axis(f(195,10000))
	:eccentricity(f(4,100))
	:inclination(math.deg2rad(0))
	:rotation_period(f(16,10))
	:axial_tilt(fixed.deg2rad(f(0,10)))
	:metallicity(f(105,1000))
	:volcanicity(f(6,1000))
	:atmos_density(f(0,1000))
	:atmos_oxidizing(f(0,10))
	:ocean_cover(f(0,1000))
	:ice_cover(f(0,1000))
	:life(f(0,1))

local ayizanc = CustomSystemBody:new('Agwe', 'PLANET_TERRESTRIAL')
	:seed(54227136)
	:radius(f(2648,10000))
	:mass(f(186,10000))
	:temp(661)
	:semi_major_axis(f(32,1000))
	:eccentricity(f(1,100))
	:inclination(math.deg2rad(0))
	:rotation_period(f(33,10))
	:axial_tilt(fixed.deg2rad(f(0,10)))
	:metallicity(f(105,1000))
	:volcanicity(f(208,1000))
	:atmos_density(f(0,1000))
	:atmos_oxidizing(f(0,10))
	:ocean_cover(f(0,1000))
	:ice_cover(f(0,1000))
	:life(f(0,1))
	
local ayizand = CustomSystemBody:new('Filomez', 'PLANET_TERRESTRIAL')
	:seed(-2093049459)
	:radius(f(3866,10000))
	:mass(f(578,10000))
	:temp(513)
	:semi_major_axis(f(53,1000))
	:eccentricity(f(7,100))
	:inclination(math.deg2rad(0))
	:rotation_period(f(70,10))
	:axial_tilt(fixed.deg2rad(f(0,10)))
	:metallicity(f(239,1000))
	:volcanicity(f(25,1000))
	:atmos_density(f(0,1000))
	:atmos_oxidizing(f(0,10))
	:ocean_cover(f(0,1000))
	:ice_cover(f(0,1000))
	:life(f(0,1))
	
local ayizane = CustomSystemBody:new('Kalfu', 'PLANET_TERRESTRIAL')
	:seed(-1261051275)
	:radius(f(8082,10000))
	:mass(f(5278,10000))
	:temp(409)
	:semi_major_axis(f(116,1000))
	:eccentricity(f(23,100))
	:inclination(math.deg2rad(0))
	:rotation_period(f(225,10))
	:axial_tilt(fixed.deg2rad(f(0,10)))
	:metallicity(f(4,1000))
	:volcanicity(f(276,1000))
	:atmos_density(f(723,1000))
	:atmos_oxidizing(f(12,100))
	:ocean_cover(f(158,1000))
	:ice_cover(f(19,1000))
	:life(f(0,1))

local ayizanf = CustomSystemBody:new('Marassa', 'PLANET_TERRESTRIAL')
	:seed(469775670)
	:radius(f(8318,10000))
	:mass(f(5756,10000))
	:temp(212)
	:semi_major_axis(f(220,1000))
	:eccentricity(f(2,100))
	:inclination(math.deg2rad(0))
	:rotation_period(f(454,10))
	:axial_tilt(fixed.deg2rad(f(0,10)))
	:metallicity(f(520,1000))
	:volcanicity(f(360,1000))
	:atmos_density(f(0,1000))
	:atmos_oxidizing(f(0,100))
	:ocean_cover(f(0,1000))
	:ice_cover(f(0,1000))
	:life(f(0,1))

local ayizanf_starports = {
	CustomSystemBody:new('Antonin', 'STARPORT_SURFACE')
		:latitude(math.deg2rad(31))
		:longitude(math.deg2rad(-121)),
	CustomSystemBody:new('Polynice', 'STARPORT_SURFACE')
		:latitude(math.deg2rad(19))
		:longitude(math.deg2rad(99)),
	CustomSystemBody:new('Port Thelyson', 'STARPORT_ORBITAL')
		:semi_major_axis(f(3,10000))
		:rotation_period(f(10,10))

}

local ayizang = CustomSystemBody:new('Damballa', 'PLANET_TERRESTRIAL')
	:seed(7)
	:radius(f(17301,10000))
	:mass(f(29931,10000))
	:temp(297)
	:semi_major_axis(f(352,1000))
	:eccentricity(f(13,100))
	:inclination(math.deg2rad(0.1))
	:rotation_period(f(42,10))
	:axial_tilt(fixed.deg2rad(f(2,10)))
	:metallicity(f(537,1000))
	:volcanicity(f(397,1000))
	:atmos_density(f(1844,1000))
	:atmos_oxidizing(f(96,100))
	:ocean_cover(f(638,1000))
	:ice_cover(f(106,1000))
	:life(f(1634,1000))

local ayizang_starports = {
	CustomSystemBody:new('Zephirin', 'STARPORT_SURFACE')
		:latitude(math.deg2rad(9.15667))
		:longitude(math.deg2rad(-13.27028)),
	CustomSystemBody:new('Ti Faustin', 'STARPORT_SURFACE')
		:latitude(math.deg2rad(38.38083))
		:longitude(math.deg2rad(9.18167)),
	CustomSystemBody:new('Nan Garcelle', 'STARPORT_SURFACE')
		:latitude(math.deg2rad(-52.46639))
		:longitude(math.deg2rad(89.04972))
		:space_station_type("ground_station"),
	CustomSystemBody:new('Ermoza', 'STARPORT_ORBITAL')
		:semi_major_axis(f(795,1000000))
		:rotation_period(f(27,10))

}

local ayizanh = CustomSystemBody:new('Ayida Wedo', 'PLANET_TERRESTRIAL')
	:seed(-1886250688)
	:radius(f(13607,10000))
	:mass(f(18514,10000))
	:temp(102)
	:semi_major_axis(f(540,1000))
	:eccentricity(f(0,100))
	:inclination(math.deg2rad(0))
	:rotation_period(f(32,10))
	:axial_tilt(fixed.deg2rad(f(10,10)))
	:metallicity(f(629,1000))
	:volcanicity(f(327,1000))
	:atmos_density(f(354,1000))
	:atmos_oxidizing(f(12,100))
	:ocean_cover(f(233,1000))
	:ice_cover(f(114,1000))
	:life(f(0,1))

local ayizanh_starports = {
	CustomSystemBody:new('Sarazin', 'STARPORT_SURFACE')
		:latitude(math.deg2rad(31))
		:longitude(math.deg2rad(-121)),
	CustomSystemBody:new('Alcine', 'STARPORT_ORBITAL')
		:semi_major_axis(f(3,10000))
		:rotation_period(f(10,10))

}	

local ayizani = CustomSystemBody:new('Ogoun', 'PLANET_TERRESTRIAL')
	:seed(-1521407529)
	:radius(f(3390,10000))
	:mass(f(390,10000))
	:temp(83)
	:semi_major_axis(f(812,1000))
	:eccentricity(f(6,100))
	:inclination(math.deg2rad(0))
	:rotation_period(f(80,10))
	:axial_tilt(fixed.deg2rad(f(10,10)))
	:metallicity(f(688,1000))
	:volcanicity(f(4,1000))
	:atmos_density(f(0,1000))
	:atmos_oxidizing(f(0,100))
	:ocean_cover(f(0,1000))
	:ice_cover(f(0,1000))
	:life(f(0,1))

local ayizanj = CustomSystemBody:new('Papa Legba', 'PLANET_GAS_GIANT')
	:seed(1576812685)
	:radius(f(66950,10000))
	:mass(f(448270,10000))
	:temp(88)
	:semi_major_axis(f(1783,1000))
	:eccentricity(f(35,100))
	:inclination(math.deg2rad(0))
	:rotation_period(f(83,10))
	:axial_tilt(fixed.deg2rad(f(32,10)))
	:metallicity(f(56,1000))
	:volcanicity(f(13,1000))
	:atmos_density(f(0,1000))
	:atmos_oxidizing(f(0,100))
	:ocean_cover(f(0,1000))
	:ice_cover(f(0,1000))
	:life(f(0,1))
	
s:bodies(ayizan, {

	ayizana,
	ayizanb,
	ayizanc,
	ayizand,
	ayizane,
	ayizanf,
		ayizanf_starports,
	ayizang,
		ayizang_starports,
	ayizanh,
		ayizanh_starports,
	ayizani,
	ayizanj,
	})

s:add_to_sector(-1,-1,-2,v(0.183,0.05,0.990))
