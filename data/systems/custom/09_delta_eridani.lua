-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local s = CustomSystem:new('Delta Eridani',{'STAR_K_GIANT'}):other_names({"Svartalfheim"})
	:faction('Commonwealth of Independent Worlds')
	:govtype('CISSOCDEM')
	:lawlessness(f(15,100)) -- 1/100th from a peaceful eden
	:short_desc('Social democratic mining colony')
	:long_desc([["How will I pay my oxygen bill ?" This question is as old as the colonization of the Moon a thousand years ago. On most airless planets across the Galaxy, people are individually responsaible for their air, and usually live and work in small cubicles as to avoid waste. Only monuments and luxury buildings offer vast, breathable areas. 
Air poverty has been an important problem for ages, and civilizations have come up with many answers : some left their poor die of air deprivation, others encouraged charity, gave ration coupons, or opened public bunkhouses.
But the Commonwealth of Independent Worlds took a radical approach : in most CIW systems, air is a free commodity for everyone. This policy may sound obvious to anyone living on an outdoor world, but it does make a world of difference for airless planets like the ones around Delta Eridani.
The cities here are built according to the "Sub-Terran" architecture : vast underground volumes, usually former quarries and mines, provide a public space surrounded by subterranean buildings. People only have to open their windows to renew their air, enjoy the artificial sunlight or the collectively-set temperature. These public areas also sport colourful parks, fountains, swimming pools etc.
As a result, cities around Delta Eridani often rank among the most pleasant places to live in the Galaxy. However, to enjoy life here, one also has to "enjoy" sharing space with the crowd, abiding to strict anti-pollution regulation, and paying especially high taxes.]])

--Names do not infringe any copyright
--Planets are named after Norse mythology (gas giants are minor deities, terrestrial planets are named after dwarves)
--Cities are named after various utopias
 

local deltaeridani = CustomSystemBody:new('Delta Eridani', 'STAR_K_GIANT') --Alt name will be Svartalfheim
	:radius(f(1781,100))
	:mass(f(283,100))
	:temp(3685)

local deltaeridania = CustomSystemBody:new('Magni', 'PLANET_GAS_GIANT')
	:seed(328034979)
	:radius(f(118816,10000))
	:mass(f(14177945,10000))
	:temp(156)
	:semi_major_axis(f(22841,1000))
	:eccentricity(f(7,100))
	:inclination(math.deg2rad(25))
	:rotation_period(f(28,10))
	:axial_tilt(fixed.deg2rad(f(18,10)))
	:metallicity(f(383,1000))
	:volcanicity(f(708,1000))
	:atmos_density(f(0,1000))
	:atmos_oxidizing(f(0,10))
	:ocean_cover(f(0,1000))
	:ice_cover(f(0,100))
	:life(f(0,1))	

local deltaeridania1 = {
	CustomSystemBody:new('Lofar', 'PLANET_TERRESTRIAL')
		:seed(1823714768)		
		:radius(f(855,10000))
		:mass(f(625,1000000))
		:temp(100)
		:semi_major_axis(f(4587,1000000))
		:eccentricity(f(17,100))
		:inclination(math.deg2rad(0))
		:rotation_period(f(17,10))
		:axial_tilt(fixed.deg2rad(f(0,10)))
		:metallicity(f(498,1000))
		:volcanicity(f(0,1000))
		:atmos_density(f(0,10))
		:atmos_oxidizing(f(0,10))
		:ocean_cover(f(0,1000))
		:ice_cover(f(0,1000))
		:life(f(0,10))
	}

local deltaeridania2 = {
	CustomSystemBody:new('Dvalin', 'PLANET_TERRESTRIAL')
		:seed(971150682)		
		:radius(f(3435,10000))
		:mass(f(405,10000))
		:temp(100)
		:semi_major_axis(f(10,1000))
		:eccentricity(f(28,100))
		:inclination(math.deg2rad(0))
		:rotation_period(f(59,10))
		:axial_tilt(fixed.deg2rad(f(0,10)))
		:metallicity(f(459,1000))
		:volcanicity(f(7,1000))
		:atmos_density(f(0,10))
		:atmos_oxidizing(f(0,10))
		:ocean_cover(f(0,1000))
		:ice_cover(f(0,1000))
		:life(f(0,10))
	}

local deltaeridania3 = {
	CustomSystemBody:new('Brokk', 'PLANET_TERRESTRIAL')
		:seed(1887818494)		
		:radius(f(3680,10000))
		:mass(f(499,10000))
		:temp(100)
		:semi_major_axis(f(27,1000))
		:eccentricity(f(12,100))
		:inclination(math.deg2rad(0.3))
		:rotation_period(f(433,10))
		:axial_tilt(fixed.deg2rad(f(3,10)))
		:metallicity(f(79,1000))
		:volcanicity(f(19,1000))
		:atmos_density(f(0,10))
		:atmos_oxidizing(f(0,10))
		:ocean_cover(f(0,1000))
		:ice_cover(f(0,1000))
		:life(f(0,10))
	}
	
local deltaeridania4 = {
	CustomSystemBody:new('Durin', 'PLANET_TERRESTRIAL')
		:seed(1103174480)		
		:radius(f(6845,10000))
		:mass(f(3207,10000))
		:temp(100)
		:semi_major_axis(f(44,1000))
		:eccentricity(f(9,100))
		:inclination(math.deg2rad(0.1))
		:rotation_period(f(414,10))
		:axial_tilt(fixed.deg2rad(f(1,10)))
		:metallicity(f(701,1000))
		:volcanicity(f(307,1000))
		:atmos_density(f(0,10))
		:atmos_oxidizing(f(0,10))
		:ocean_cover(f(0,1000))
		:ice_cover(f(0,1000))
		:life(f(0,10)),	
	{
		CustomSystemBody:new('Arcadia', 'STARPORT_SURFACE')
			:latitude(math.deg2rad(13.1308))
			:longitude(math.deg2rad(-80.1142)),
		CustomSystemBody:new('Topolobampo', 'STARPORT_SURFACE')
			:latitude(math.deg2rad(77.0921))
			:longitude(math.deg2rad(-57.0348)),
		CustomSystemBody:new('Diggersborough', 'STARPORT_SURFACE')
			:latitude(math.deg2rad(-41.0543))
			:longitude(math.deg2rad(55.0727)),
	},	
	}

local deltaeridania5 = {
	CustomSystemBody:new('Eiti', 'PLANET_TERRESTRIAL')
		:seed(-1290076376)		
		:radius(f(6197,10000))
		:mass(f(2379,10000))
		:temp(100)
		:semi_major_axis(f(69,1000))
		:eccentricity(f(1,100))
		:inclination(math.deg2rad(0.4))
		:rotation_period(f(83,10))
		:axial_tilt(fixed.deg2rad(f(23,10)))
		:metallicity(f(508,1000))
		:volcanicity(f(143,1000))
		:atmos_density(f(0,10))
		:atmos_oxidizing(f(0,10))
		:ocean_cover(f(0,1000))
		:ice_cover(f(0,1000))
		:life(f(0,10))
	}
	
local deltaeridania6 = {
	CustomSystemBody:new('Ivaldi', 'PLANET_TERRESTRIAL')
		:seed(-1051896087)		
		:radius(f(9195,10000))
		:mass(f(7776,10000))
		:temp(100)
		:semi_major_axis(f(146,1000))
		:eccentricity(f(21,100))
		:inclination(math.deg2rad(4.2))
		:rotation_period(f(60,10))
		:axial_tilt(fixed.deg2rad(f(154,10)))
		:metallicity(f(748,1000))
		:volcanicity(f(157,1000))
		:atmos_density(f(391,1000))
		:atmos_oxidizing(f(8,10))
		:ocean_cover(f(260,1000))
		:ice_cover(f(157,1000))
		:life(f(0,10)),	
	{
		CustomSystemBody:new('Civitas Solis', 'STARPORT_SURFACE')
			:latitude(math.deg2rad(-33))
			:longitude(math.deg2rad(108)),
		CustomSystemBody:new('Concordium', 'STARPORT_SURFACE')
			:latitude(math.deg2rad(60))
			:longitude(math.deg2rad(-87)),
		CustomSystemBody:new('Icaria', 'STARPORT_ORBITAL')
			:semi_major_axis(f(861,1000000))
			:rotation_period(f(59,10)),
		CustomSystemBody:new('Datong', 'STARPORT_ORBITAL')
			:semi_major_axis(f(130,1000000))
			:rotation_period(f(5,10)),
	},	
	}
	
local deltaeridanib = CustomSystemBody:new('Modi', 'PLANET_GAS_GIANT')
	:seed(225608433)
	:radius(f(111039,10000))
	:mass(f(30436102,10000))
	:temp(122)
	:semi_major_axis(f(37234,1000))
	:eccentricity(f(8,100))
	:inclination(math.deg2rad(0))
	:rotation_period(f(25,10))
	:axial_tilt(fixed.deg2rad(f(4,10)))
	:metallicity(f(464,1000))
	:volcanicity(f(750,1000))
	:atmos_density(f(0,1000))
	:atmos_oxidizing(f(0,10))
	:ocean_cover(f(0,1000))
	:ice_cover(f(0,100))
	:life(f(0,1))		

local deltaeridanib1 = {
	CustomSystemBody:new('Sindri', 'PLANET_TERRESTRIAL')
		:seed(7407138)		
		:radius(f(443,10000))
		:mass(f(87,1000000))
		:temp(78)
		:semi_major_axis(f(3974,1000000))
		:eccentricity(f(3,100))
		:inclination(math.deg2rad(0))
		:rotation_period(f(10,10))
		:axial_tilt(fixed.deg2rad(f(0,10)))
		:metallicity(f(213,1000))
		:volcanicity(f(0,1000))
		:atmos_density(f(0,10))
		:atmos_oxidizing(f(0,10))
		:ocean_cover(f(0,1000))
		:ice_cover(f(0,1000))
		:life(f(0,10))
	}

local deltaeridanib2 = {
	CustomSystemBody:new('Berling', 'PLANET_TERRESTRIAL')
		:seed(-1920036867)		
		:radius(f(513,10000))
		:mass(f(135,1000000))
		:temp(78)
		:semi_major_axis(f(5919,1000000))
		:eccentricity(f(2,100))
		:inclination(math.deg2rad(0))
		:rotation_period(f(17,10))
		:axial_tilt(fixed.deg2rad(f(0,10)))
		:metallicity(f(485,1000))
		:volcanicity(f(0,1000))
		:atmos_density(f(0,10))
		:atmos_oxidizing(f(0,10))
		:ocean_cover(f(0,1000))
		:ice_cover(f(0,1000))
		:life(f(0,10))
	}

local deltaeridanib3 = {
	CustomSystemBody:new('Grer', 'PLANET_TERRESTRIAL')
		:seed(1442301875)		
		:radius(f(738,10000))
		:mass(f(4,10000))
		:temp(78)
		:semi_major_axis(f(10,1000))
		:eccentricity(f(10,100))
		:inclination(math.deg2rad(0))
		:rotation_period(f(39,10))
		:axial_tilt(fixed.deg2rad(f(0,10)))
		:metallicity(f(502,1000))
		:volcanicity(f(0,1000))
		:atmos_density(f(0,10))
		:atmos_oxidizing(f(0,10))
		:ocean_cover(f(0,1000))
		:ice_cover(f(0,1000))
		:life(f(0,10))
	}

local deltaeridanib4 = {
	CustomSystemBody:new('Andvari', 'PLANET_TERRESTRIAL')
		:seed(1041123534)		
		:radius(f(2403,10000))
		:mass(f(139,10000))
		:temp(78)
		:semi_major_axis(f(25,1000))
		:eccentricity(f(14,100))
		:inclination(math.deg2rad(0))
		:rotation_period(f(147,10))
		:axial_tilt(fixed.deg2rad(f(1,10)))
		:metallicity(f(611,1000))
		:volcanicity(f(12,1000))
		:atmos_density(f(0,10))
		:atmos_oxidizing(f(0,10))
		:ocean_cover(f(0,1000))
		:ice_cover(f(0,1000))
		:life(f(0,10))
	}

local deltaeridanib5 = {
	CustomSystemBody:new('Fjalar', 'PLANET_TERRESTRIAL')
		:seed(-2014924513)		
		:radius(f(2919,10000))
		:mass(f(249,10000))
		:temp(78)
		:semi_major_axis(f(42,1000))
		:eccentricity(f(7,100))
		:inclination(math.deg2rad(0))
		:rotation_period(f(266,10))
		:axial_tilt(fixed.deg2rad(f(25,10)))
		:metallicity(f(262,1000))
		:volcanicity(f(1,1000))
		:atmos_density(f(0,10))
		:atmos_oxidizing(f(0,10))
		:ocean_cover(f(0,1000))
		:ice_cover(f(0,1000))
		:life(f(0,10))
	}

local deltaeridanib6 = {
	CustomSystemBody:new('Galar', 'PLANET_TERRESTRIAL')
		:seed(-574202901)		
		:radius(f(4012,10000))
		:mass(f(646,10000))
		:temp(78)
		:semi_major_axis(f(73,1000))
		:eccentricity(f(4,100))
		:inclination(math.deg2rad(0))
		:rotation_period(f(206,10))
		:axial_tilt(fixed.deg2rad(f(109,10)))
		:metallicity(f(711,1000))
		:volcanicity(f(41,1000))
		:atmos_density(f(0,10))
		:atmos_oxidizing(f(0,10))
		:ocean_cover(f(0,1000))
		:ice_cover(f(0,1000))
		:life(f(0,10))
	}

local deltaeridanib7 = {
	CustomSystemBody:new('Alberich', 'PLANET_TERRESTRIAL')
		:seed(180730376)		
		:radius(f(3744,10000))
		:mass(f(525,10000))
		:temp(78)
		:semi_major_axis(f(108,1000))
		:eccentricity(f(0,100))
		:inclination(math.deg2rad(0))
		:rotation_period(f(33,10))
		:axial_tilt(fixed.deg2rad(f(42,10)))
		:metallicity(f(811,1000))
		:volcanicity(f(11,1000))
		:atmos_density(f(0,10))
		:atmos_oxidizing(f(0,10))
		:ocean_cover(f(0,1000))
		:ice_cover(f(0,1000))
		:life(f(0,10)),
	{
		CustomSystemBody:new('Politeia', 'STARPORT_SURFACE')
			:latitude(math.deg2rad(40.1744))
			:longitude(math.deg2rad(-113.3403))
	},	
	}

local deltaeridanib8 = {
	CustomSystemBody:new('Alfrigg', 'PLANET_TERRESTRIAL')
		:seed(-1386329689)		
		:radius(f(4522,10000))
		:mass(f(925,10000))
		:temp(78)
		:semi_major_axis(f(154,1000))
		:eccentricity(f(5,100))
		:inclination(math.deg2rad(0))
		:rotation_period(f(38,10))
		:axial_tilt(fixed.deg2rad(f(4,10)))
		:metallicity(f(717,1000))
		:volcanicity(f(14,1000))
		:atmos_density(f(0,10))
		:atmos_oxidizing(f(0,10))
		:ocean_cover(f(0,1000))
		:ice_cover(f(0,1000))
		:life(f(0,10)),
	{
		CustomSystemBody:new('New Lanark', 'STARPORT_SURFACE')
			:latitude(math.deg2rad(-61.0722))
			:longitude(math.deg2rad(54.2447)),
		CustomSystemBody:new('Phalanstery', 'STARPORT_ORBITAL')
			:semi_major_axis(f(461,1000000))
			:rotation_period(f(30,10)),
	},	
	}

local deltaeridanib9 = {
	CustomSystemBody:new('Modsognir', 'PLANET_TERRESTRIAL')
		:seed(-1625009381)		
		:radius(f(6117,10000))
		:mass(f(2289,10000))
		:temp(78)
		:semi_major_axis(f(284,1000))
		:eccentricity(f(4,100))
		:inclination(math.deg2rad(0))
		:rotation_period(f(8,10))
		:axial_tilt(fixed.deg2rad(f(35,10)))
		:metallicity(f(238,1000))
		:volcanicity(f(82,1000))
		:atmos_density(f(275,1000))
		:atmos_oxidizing(f(1,10))
		:ocean_cover(f(215,1000))
		:ice_cover(f(138,1000))
		:life(f(0,10))
	}
	
s:bodies(deltaeridani, {

	deltaeridania,
		deltaeridania1,
		deltaeridania2,
		deltaeridania3,
		deltaeridania4,
		deltaeridania5,
		deltaeridania6,
	deltaeridanib,
		deltaeridanib1,
		deltaeridanib2,
		deltaeridanib3,
		deltaeridanib4,
		deltaeridanib5,
		deltaeridanib6,
		deltaeridanib7,
		deltaeridanib8,
		deltaeridanib9,

	})

s:add_to_sector(3,-3,-1,v(0.006,0.958,0.375))
