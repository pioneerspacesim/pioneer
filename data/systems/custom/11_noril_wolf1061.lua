-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local s = CustomSystem:new('Noril',{'STAR_M'}):other_names({"Wolf 1061"})
	:faction('Solar Federation')
	:govtype('EARTHDEMOC')
	:lawlessness(f(15,100)) -- 1/100th from a peaceful eden
	:short_desc('Federal Industrial Colony')
	:long_desc([[Founded in 2786,  Noril is an industrial system, and one of the oldest "Mill worlds" under Federal Rule.
After the War of Hope, the Solar Federation raced against the Commonwealth of Independent Worlds to colonize new systems faster than its competitor. To that effect, corporations were incented to settle new worlds under Federal rule. 
As per the "Colonization act 2778-1266" a company could claim large parts of a planet, and would get various benefits : long-term tax cuts and subsidies, strong police protection against piracy, as well as lax labor or environment regulation. In exchange, that company would have to build all initial infrastructure -the factories, the living quarters, the government and military buildings- with its private funds.
Noril became known as the "child of Mrs Bureaucracy and Mr Corporate Greed". The founding corporations built their Mill towns for cheap. Really cheap. Settlers quickly discovered the buildings' shoddiness and the near absence of amenities. Meanwhile, the Federation had to raise taxes in its core systems to fund its stretched out forces, and to repair the subpar infrastructure. 
With hindsight however, the "Colonization act 2778-1266" was a success. It claimed many systems which remain in the Federation until this day. It also prevented the creation of quite a few independant corporate worlds.
Furthermore, several Mill towns have turned into successful cities. Travellers are advised to visit Octoscent, "City of Eternal Crimson"; not only for its museums located in the (now bankrupt) Octoscent Corp headquarters, but also to feel the impetus of its small businesses, art galleries and pubs.]])

--Names do not infringe any copyright
--Bodies' names are inspired by USSR factory-cities : Norilsk, Kisselevsk and Seversk.
--Uninhabited planets keep their generic names, as the Federal bureaucracy didn't bother to name them
--Cities are named after fictional companies. Their names are generated with http://www.businessnamegenerators.com/. A quick search lets think there is no actual company  with those names.


local noril = CustomSystemBody:new('Noril', 'STAR_M')
	:radius(f(48,100))
	:mass(f(44,100))
	:temp(2863)

local norila = CustomSystemBody:new('Kissel', 'PLANET_TERRESTRIAL')
	:seed(-1225274654)
	:radius(f(12230,10000))
	:mass(f(14958,10000))
	:temp(240)
	:semi_major_axis(f(192,1000))
	:eccentricity(f(4,100))
	:inclination(math.deg2rad(0))
	:rotation_period(f(463,10))
	:axial_tilt(fixed.deg2rad(f(0,10)))
	:metallicity(f(520,1000))
	:volcanicity(f(886,1000))
	:atmos_density(f(1407,1000))
	:atmos_oxidizing(f(17,100))
	:ocean_cover(f(485,1000))
	:ice_cover(f(101,1000))
	:life(f(0,1))

local norila_starports =	{
	CustomSystemBody:new('Octoscent', 'STARPORT_SURFACE')
		:latitude(math.deg2rad(43.19417))
		:longitude(math.deg2rad(23.30194)),
	CustomSystemBody:new('Intralium', 'STARPORT_SURFACE')
		:latitude(math.deg2rad(-30.16861))
		:longitude(math.deg2rad(135.4575)),
	CustomSystemBody:new('Sugill', 'STARPORT_SURFACE')
		:latitude(math.deg2rad(-58.27889))
		:longitude(math.deg2rad(62.11417)),
	CustomSystemBody:new('Podity Orbital', 'STARPORT_ORBITAL')
		:semi_major_axis(f(535,1000000))
		:rotation_period(f(48,24)),
	}	

local norilb = CustomSystemBody:new('Seversk', 'PLANET_TERRESTRIAL')
	:seed(-921350797)
	:radius(f(20733,10000))
	:mass(f(42986,10000))
	:temp(114)
	:semi_major_axis(f(283,1000))
	:eccentricity(f(3,100))
	:inclination(math.deg2rad(0))
	:rotation_period(f(61,10))
	:axial_tilt(fixed.deg2rad(f(49,10)))
	:metallicity(f(493,1000))
	:volcanicity(f(480,1000))
	:atmos_density(f(344,1000))
	:atmos_oxidizing(f(33,100))
	:ocean_cover(f(262,1000))
	:ice_cover(f(115,1000))
	:life(f(0,1))

local norilb_starports =	{
	CustomSystemBody:new('Sucize', 'STARPORT_SURFACE')
		:latitude(math.deg2rad(5.7541))
		:longitude(math.deg2rad(4.14)),
	CustomSystemBody:new('Hyperill', 'STARPORT_SURFACE')
		:latitude(math.deg2rad(-38.25722))
		:longitude(math.deg2rad(179.2514)),
	CustomSystemBody:new('Hemivu Station', 'STARPORT_ORBITAL')
			:semi_major_axis(f(700,1000000))
			:rotation_period(f(48,24)),
	}

local norilc = CustomSystemBody:new('Noril c', 'PLANET_GAS_GIANT')
	:seed(-984766483)
	:radius(f(46151,10000))
	:mass(f(212996,10000))
	:temp(136)
	:semi_major_axis(f(492,1000))
	:eccentricity(f(9,100))
	:inclination(math.deg2rad(0))
	:rotation_period(f(366,10))
	:axial_tilt(fixed.deg2rad(f(30,10)))
	:metallicity(f(116,1000))
	:volcanicity(f(458,1000))
	:atmos_density(f(0,1000))
	:atmos_oxidizing(f(0,100))
	:ocean_cover(f(0,1000))
	:ice_cover(f(0,1000))
	:life(f(0,1))

local norild = CustomSystemBody:new('Noril d', 'PLANET_GAS_GIANT')
	:seed(-1080033261)
	:radius(f(113178,10000))
	:mass(f(1279564,10000))
	:temp(98)
	:semi_major_axis(f(944,1000))
	:eccentricity(f(11,100))
	:inclination(math.deg2rad(0))
	:rotation_period(f(6,10))
	:axial_tilt(fixed.deg2rad(f(72,10)))
	:metallicity(f(45,1000))
	:volcanicity(f(589,1000))
	:atmos_density(f(0,1000))
	:atmos_oxidizing(f(0,100))
	:ocean_cover(f(0,1000))
	:ice_cover(f(0,1000))
	:life(f(0,1))

local norile = CustomSystemBody:new('Noril e', 'PLANET_GAS_GIANT')
	:seed(490690684)
	:radius(f(136108,10000))
	:mass(f(3059155,10000))
	:temp(76)
	:semi_major_axis(f(1566,1000))
	:eccentricity(f(3,100))
	:inclination(math.deg2rad(0))
	:rotation_period(f(5,10))
	:axial_tilt(fixed.deg2rad(f(159,10)))
	:metallicity(f(273,1000))
	:volcanicity(f(416,1000))
	:atmos_density(f(0,1000))
	:atmos_oxidizing(f(0,100))
	:ocean_cover(f(0,1000))
	:ice_cover(f(0,1000))
	:life(f(0,1))

local norilf = CustomSystemBody:new('Noril f', 'PLANET_GAS_GIANT')
	:seed(-1244234417)
	:radius(f(134060,10000))
	:mass(f(3620204,10000))
	:temp(51)
	:semi_major_axis(f(2413,1000))
	:eccentricity(f(4,100))
	:inclination(math.deg2rad(0))
	:rotation_period(f(57,10))
	:axial_tilt(fixed.deg2rad(f(43,10)))
	:metallicity(f(367,1000))
	:volcanicity(f(320,1000))
	:atmos_density(f(0,1000))
	:atmos_oxidizing(f(0,100))
	:ocean_cover(f(0,1000))
	:ice_cover(f(0,1000))
	:life(f(0,1))

local norilg = CustomSystemBody:new('Noril g', 'PLANET_GAS_GIANT')
	:seed(-1365712623)
	:radius(f(136498,10000))
	:mass(f(2962057,10000))
	:temp(49)
	:semi_major_axis(f(3691,1000))
	:eccentricity(f(1,100))
	:inclination(math.deg2rad(0))
	:rotation_period(f(47,10))
	:axial_tilt(fixed.deg2rad(f(1,10)))
	:metallicity(f(294,1000))
	:volcanicity(f(187,1000))
	:atmos_density(f(0,1000))
	:atmos_oxidizing(f(0,100))
	:ocean_cover(f(0,1000))
	:ice_cover(f(0,1000))
	:life(f(0,1))
	
s:bodies(noril, {

	norila,
		norila_starports,
	norilb,
		norilb_starports,
	norilc,
	norild,
	norile,
	norilf,
	norilg,

	})

s:add_to_sector(-2,0,-1,v(0.432,0.647,0.619))
