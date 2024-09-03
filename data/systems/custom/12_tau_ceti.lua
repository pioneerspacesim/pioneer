-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local s = CustomSystem:new('Tau Ceti',{'STAR_G'})
	:faction('Commonwealth of Independent Worlds')
	:govtype('CISSOCDEM')
	:lawlessness(f(15,100)) -- 1/100th from a peaceful eden
	:short_desc('Historic system with interplanetary rivalry')
	:long_desc([[One of the "old systems" founded before hyperspace travel, Tau Ceti is the stage of a centuries old rivalry between the heirs of the two original colony ships.
As the "Inamorata" first reached the system in 2331 -despite being launched later- its colonists claimed lush planet Terranova for themselves. When the "Harapan" arrived 5 years later, its passengers unawara they would encounter fellow humans, it had no choice but to land on Planet Pontianak where the colonists would live a harsh life.
Revenge should have no bounds. For four centuries, both factions waged a bloody interplanetary vendetta. While diplomats avoided an all-out war, both governments unofficially maintained strong fleets of privateers, which wrought pillage and destruction on their enemy, further fueling calls for revenge.
During the 27th century, when hyperspace travel connected Tau Ceti to its neighbours, foreigners avoided this dangerous system. The Federation did establish embassies, but took lightly to enforce order in these warlike parts. Pontianak took advantage of this situation, with its agile ships and cunning businessmen making profit from interstellar trade, while their privateers barred Terranova from doing the same.
During the War of Hope (2723-2725), Pontianak sided with the Solar Federation to defend its trade interests and Terranova sided with the Commonwealth, becoming one of its founding members. As both factions neutralized each other, Tau Ceti did not contribute significantly to the outcome of the war at large. 
Ever since the end of the war, Tau Ceti has been a member of the CIW, which made efforts to pacify the system by disarming the privateers and treating both planets equally: the system is now safe and prosperous. 
As to the calls for blood, they have been replaced by bad taste jokes and faint racism. Citizens of Tau Ceti commonly explain to foreigners: "Our two planets are now in eternal friendship: us and them are worst friends forever".]])

--Names do not infringe any copyright
--Bodies' names are inspired either by Italian-themed theatre and opera, either by Indonesian folklore
--Starports are named after actual villages or cities in Italy or on the island of Java.
 

local tauceti = CustomSystemBody:new('Tau Ceti', 'STAR_G')
	:radius(f(99,100))
	:mass(f(102,100))
	:temp(5642)

local taucetia = CustomSystemBody:new('Columbinna', 'PLANET_TERRESTRIAL') --Character in comedia dell'arte
	:seed(-560421356)
	:radius(f(4849,10000))
	:mass(f(1139,10000))
	:temp(766)
	:semi_major_axis(f(125,1000))
	:eccentricity(f(17,100))
	:inclination(math.deg2rad(0))
	:rotation_period(f(160,10))
	:axial_tilt(fixed.deg2rad(f(0,10)))
	:metallicity(f(120,1000))
	:volcanicity(f(82,1000))
	:atmos_density(f(0,1000))
	:atmos_oxidizing(f(0,100))
	:ocean_cover(f(0,1000))
	:ice_cover(f(0,1000))
	:life(f(0,1))

local taucetib = CustomSystemBody:new('Sekartaji', 'PLANET_TERRESTRIAL') --Character in the tale of Prince Panji
	:seed(-660830055)
	:radius(f(6822,10000))
	:mass(f(3175,10000))
	:temp(579)
	:semi_major_axis(f(223,1000))
	:eccentricity(f(7,100))
	:inclination(math.deg2rad(0))
	:rotation_period(f(362,10))
	:axial_tilt(fixed.deg2rad(f(12,10)))
	:metallicity(f(352,1000))
	:volcanicity(f(224,1000))
	:atmos_density(f(84,1000))
	:atmos_oxidizing(f(85,100))
	:ocean_cover(f(13,1000))
	:ice_cover(f(1,1000))
	:life(f(0,1))

local taucetic = CustomSystemBody:new('Gunung Sari', 'PLANET_TERRESTRIAL') --Character in the tale of Prince Panji
	:seed(-2077146049)
	:radius(f(6181,10000))
	:mass(f(2361,10000))
	:temp(370)
	:semi_major_axis(f(695,1000))
	:eccentricity(f(6,100))
	:inclination(math.deg2rad(5.2))
	:rotation_period(f(82,10))
	:axial_tilt(fixed.deg2rad(f(189,10)))
	:metallicity(f(205,1000))
	:volcanicity(f(206,1000))
	:atmos_density(f(144,1000))
	:atmos_oxidizing(f(2,100))
	:ocean_cover(f(30,1000))
	:ice_cover(f(3,1000))
	:life(f(0,1))

local taucetic_starports =	{
	CustomSystemBody:new('Jangkar', 'STARPORT_SURFACE') --Javanese town
		:latitude(math.deg2rad(43.2777))
		:longitude(math.deg2rad(104.4289))
	}

local taucetid = CustomSystemBody:new('Fiordiligi', 'PLANET_TERRESTRIAL') --Character in Cosi Fan Tutte
	:seed(-1097654929)
	:radius(f(14990,10000))
	:mass(f(22470,10000))
	:temp(876)
	:semi_major_axis(f(827,1000))
	:eccentricity(f(8,100))
	:inclination(math.deg2rad(5.7))
	:rotation_period(f(42,10))
	:axial_tilt(fixed.deg2rad(f(189,10)))
	:metallicity(f(400,1000))
	:volcanicity(f(31,1000))
	:atmos_density(f(2794,1000))
	:atmos_oxidizing(f(25,100))
	:ocean_cover(f(302,1000))
	:ice_cover(f(17,1000))
	:life(f(0,1))


local taucetie = CustomSystemBody:new('Terranova', 'PLANET_TERRESTRIAL') --Italian town
	:seed(7)
	:radius(f(23370,10000))
	:mass(f(54616,10000))
	:temp(291)
	:semi_major_axis(f(1106,1000))
	:eccentricity(f(2,100))
	:inclination(math.deg2rad(0))
	:rotation_period(f(18,10))
	:axial_tilt(fixed.deg2rad(f(189,10)))
	:metallicity(f(216,1000))
	:volcanicity(f(395,1000))
	:atmos_density(f(897,1000))
	:atmos_oxidizing(f(98,100))
	:ocean_cover(f(756,1000))
	:ice_cover(f(103,1000))
	:life(f(1,1))	

local taucetie_starports =	{
	CustomSystemBody:new('Verona', 'STARPORT_SURFACE') --Italian city, and a not so subtle hint to Romeo and Juliet
		:latitude(math.deg2rad(-26.13111))
		:longitude(math.deg2rad(-116.13111)),
	CustomSystemBody:new('Savelli', 'STARPORT_SURFACE') --Italian town
		:latitude(math.deg2rad(32.066))
		:longitude(math.deg2rad(-139.36593)),
	CustomSystemBody:new('Mongrassano', 'STARPORT_SURFACE') --Italian town
		:latitude(math.deg2rad(49.305))
		:longitude(math.deg2rad(21.04194)),
	CustomSystemBody:new('Porto Doria', 'STARPORT_ORBITAL') --Doria is an Italian town
			:semi_major_axis(f(350,1000000))
			:rotation_period(f(48,24)),
	}

local taucetie_moon = {
	CustomSystemBody:new('Pontianak', 'PLANET_TERRESTRIAL') --Spirit of women who died in childbirth in Indonesian folklore
		:seed(14)		
		:radius(f(5330,10000))
		:mass(f(1070,10000))
		:temp(275)
		:semi_major_axis(f(393,100000))
		:eccentricity(f(4,100))
		:inclination(math.deg2rad(5.2))
		:rotation_period(f(222,10))
		:axial_tilt(fixed.deg2rad(f(112,10)))
		:metallicity(f(581,1000))
		:volcanicity(f(16,100))
		:atmos_density(f(6,10))
		:atmos_oxidizing(f(97,100))
		:ocean_cover(f(239,1000))
		:ice_cover(f(352,1000))
		:life(f(3,10)),
	{
		CustomSystemBody:new('Sampang', 'STARPORT_SURFACE') --Javanese town
			:latitude(math.deg2rad(-2.04361))
			:longitude(math.deg2rad(124.35778)),
		CustomSystemBody:new('Kaliurip', 'STARPORT_SURFACE') --Javanese town
			:latitude(math.deg2rad(-21.11861))
			:longitude(math.deg2rad(-29.395)),
		CustomSystemBody:new('Binangun Station', 'STARPORT_ORBITAL') --Binangun is a Javanese town
			:semi_major_axis(f(15068,100000000))
			:rotation_period(f(11,24)),
	},
	}

local taucetif = CustomSystemBody:new('Damarwulan', 'PLANET_TERRESTRIAL') --Character in Javanese legends
	:seed(924201188)
	:radius(f(10894,10000))
	:mass(f(11869,10000))
	:temp(118)
	:semi_major_axis(f(2112,1000))
	:eccentricity(f(0.4,100))
	:inclination(math.deg2rad(0))
	:rotation_period(f(34,10))
	:axial_tilt(fixed.deg2rad(f(264,10)))
	:metallicity(f(216,1000))
	:volcanicity(f(395,1000))
	:atmos_density(f(1978,1000))
	:atmos_oxidizing(f(25,100))
	:ocean_cover(f(1437,1000))
	:ice_cover(f(609,1000))
	:life(f(0,1))


local taucetig = CustomSystemBody:new('Panji', 'PLANET_GAS_GIANT') --Main character in the tale of Prince Panji
	:seed(-158309594)
	:radius(f(124102,10000))
	:mass(f(1540139,10000))
	:temp(119)
	:semi_major_axis(f(5026,1000))
	:eccentricity(f(32,100))
	:inclination(math.deg2rad(0))
	:rotation_period(f(35,10))
	:axial_tilt(fixed.deg2rad(f(91,10)))
	:metallicity(f(459,1000))
	:volcanicity(f(525,1000))
	:atmos_density(f(0,1000))
	:atmos_oxidizing(f(0,100))
	:ocean_cover(f(0,1000))
	:ice_cover(f(0,1000))
	:life(f(0,1))

local taucetig1 = {
	CustomSystemBody:new('Vespone', 'PLANET_TERRESTRIAL') --Character in opera La Serva Padrona
		:seed(872974218)		
		:radius(f(4426,10000))
		:mass(f(866,10000))
		:temp(76)
		:semi_major_axis(f(569,100000))
		:eccentricity(f(7,100))
		:inclination(math.deg2rad(0))
		:rotation_period(f(26,10))
		:axial_tilt(fixed.deg2rad(f(0,10)))
		:metallicity(f(69,1000))
		:volcanicity(f(40,100))
		:atmos_density(f(0,10))
		:atmos_oxidizing(f(0,10))
		:ocean_cover(f(0,1000))
		:ice_cover(f(0,1000))
		:life(f(0,10)),
	{
		CustomSystemBody:new('Plataci', 'STARPORT_SURFACE') --Italian town
			:latitude(math.deg2rad(26.24444))
			:longitude(math.deg2rad(91.14583)),
		CustomSystemBody:new('Strongoli', 'STARPORT_SURFACE') --Italian town
			:latitude(math.deg2rad(-35.29472))
			:longitude(math.deg2rad(-72.30583)),
	},
	}

local taucetih = CustomSystemBody:new('Romeo', 'PLANET_GAS_GIANT') --A definitely not subtle hint to Romeo and Juliet
	:seed(1310712298)
	:radius(f(136153,10000))
	:mass(f(1853770,10000))
	:temp(81)
	:semi_major_axis(f(10805,1000))
	:eccentricity(f(7,100))
	:inclination(math.deg2rad(1))
	:rotation_period(f(80,10))
	:axial_tilt(fixed.deg2rad(f(42,10)))
	:metallicity(f(51,1000))
	:volcanicity(f(83,1000))
	:atmos_density(f(0,1000))
	:atmos_oxidizing(f(0,100))
	:ocean_cover(f(0,1000))
	:ice_cover(f(0,1000))
	:life(f(0,1))

local taucetih1 = {
	CustomSystemBody:new('Leporello', 'PLANET_TERRESTRIAL') --Character in opera Don Giovanni
		:seed(-886618851)		
		:radius(f(1480,10000))
		:mass(f(32,10000))
		:temp(52)
		:semi_major_axis(f(545,100000))
		:eccentricity(f(1,100))
		:inclination(math.deg2rad(0))
		:rotation_period(f(22,10))
		:axial_tilt(fixed.deg2rad(f(0,10)))
		:metallicity(f(248,1000))
		:volcanicity(f(1,100))
		:atmos_density(f(0,10))
		:atmos_oxidizing(f(0,10))
		:ocean_cover(f(0,1000))
		:ice_cover(f(0,1000))
		:life(f(0,10)),
	}

local taucetih2 = {
	CustomSystemBody:new('Menak Jingga', 'PLANET_TERRESTRIAL') --Character in Javanese legends
		:seed(-832691832)		
		:radius(f(3231,10000))
		:mass(f(337,10000))
		:temp(52)
		:semi_major_axis(f(752,100000))
		:eccentricity(f(32,100))
		:inclination(math.deg2rad(0))
		:rotation_period(f(103,10))
		:axial_tilt(fixed.deg2rad(f(0,10)))
		:metallicity(f(45,1000))
		:volcanicity(f(7,100))
		:atmos_density(f(0,10))
		:atmos_oxidizing(f(0,10))
		:ocean_cover(f(0,1000))
		:ice_cover(f(0,1000))
		:life(f(0,10)),
	}

local taucetih3 = {
	CustomSystemBody:new('Tumang', 'PLANET_TERRESTRIAL') --Character in Javanese legends
		:seed(-225327750)		
		:radius(f(3356,10000))
		:mass(f(378,10000))
		:temp(52)
		:semi_major_axis(f(15,1000))
		:eccentricity(f(7,100))
		:inclination(math.deg2rad(0))
		:rotation_period(f(284,10))
		:axial_tilt(fixed.deg2rad(f(11,10)))
		:metallicity(f(492,1000))
		:volcanicity(f(14,100))
		:atmos_density(f(0,10))
		:atmos_oxidizing(f(0,10))
		:ocean_cover(f(0,1000))
		:ice_cover(f(0,1000))
		:life(f(0,10)),
	{
		CustomSystemBody:new('Mengok', 'STARPORT_SURFACE') --Javanese town
			:latitude(math.deg2rad(-24.34889))
			:longitude(math.deg2rad(-9.107778)),
		CustomSystemBody:new('Pelabuhan', 'STARPORT_ORBITAL') --Pelabuhan should mean "harbor" in Javanese
			:semi_major_axis(f(700,1000000))
			:rotation_period(f(48,24)),
	},
	}

local taucetih4 = {
	CustomSystemBody:new('Giuditta', 'PLANET_TERRESTRIAL') --Character in Italian opera
		:seed(789773883)		
		:radius(f(3178,10000))
		:mass(f(321,10000))
		:temp(52)
		:semi_major_axis(f(22,1000))
		:eccentricity(f(1,100))
		:inclination(math.deg2rad(0))
		:rotation_period(f(236,10))
		:axial_tilt(fixed.deg2rad(f(70,10)))
		:metallicity(f(84,1000))
		:volcanicity(f(7,100))
		:atmos_density(f(0,10))
		:atmos_oxidizing(f(0,10))
		:ocean_cover(f(0,1000))
		:ice_cover(f(0,1000))
		:life(f(0,10)),
	{
		CustomSystemBody:new('Moccone', 'STARPORT_SURFACE') --Italian town
			:latitude(math.deg2rad(-4.5895))
			:longitude(math.deg2rad(-137.4417)),
	},
	}

local taucetii = CustomSystemBody:new('Giulietta', 'PLANET_GAS_GIANT') --Another not subtle hint to Romeo and Juliet
	:seed(1823486736)
	:radius(f(140894,10000))
	:mass(f(2071073,10000))
	:temp(63)
	:semi_major_axis(f(17911,1000))
	:eccentricity(f(1,100))
	:inclination(math.deg2rad(0))
	:rotation_period(f(79,10))
	:axial_tilt(fixed.deg2rad(f(20,10)))
	:metallicity(f(72,1000))
	:volcanicity(f(286,1000))
	:atmos_density(f(0,1000))
	:atmos_oxidizing(f(0,100))
	:ocean_cover(f(0,1000))
	:ice_cover(f(0,1000))
	:life(f(0,1))

local taucetii1 = {
	CustomSystemBody:new('Sang Kancil', 'PLANET_TERRESTRIAL') --Character in Javanese folklore
		:seed(546063033)		
		:radius(f(1162,10000))
		:mass(f(16,10000))
		:temp(40)
		:semi_major_axis(f(375,100000))
		:eccentricity(f(4,100))
		:inclination(math.deg2rad(0))
		:rotation_period(f(34,10))
		:axial_tilt(fixed.deg2rad(f(0,10)))
		:metallicity(f(460,1000))
		:volcanicity(f(1,100))
		:atmos_density(f(0,10))
		:atmos_oxidizing(f(0,10))
		:ocean_cover(f(0,1000))
		:ice_cover(f(0,1000))
		:life(f(0,10)),
	}

local taucetii2 = {
	CustomSystemBody:new('Ruggiero', 'PLANET_TERRESTRIAL') --Character in Italian opera
		:seed(-795351675)		
		:radius(f(2182,10000))
		:mass(f(104,10000))
		:temp(40)
		:semi_major_axis(f(719,100000))
		:eccentricity(f(25,100))
		:inclination(math.deg2rad(0))
		:rotation_period(f(89,10))
		:axial_tilt(fixed.deg2rad(f(0,10)))
		:metallicity(f(292,1000))
		:volcanicity(f(8,100))
		:atmos_density(f(0,10))
		:atmos_oxidizing(f(0,10))
		:ocean_cover(f(0,1000))
		:ice_cover(f(0,1000))
		:life(f(0,10)),
	}

local taucetii3 = {
	CustomSystemBody:new('Dorabella', 'PLANET_TERRESTRIAL') --Character in Cosi Fan Tutte
		:seed(2085489986)		
		:radius(f(4141,10000))
		:mass(f(710,10000))
		:temp(40)
		:semi_major_axis(f(20,1000))
		:eccentricity(f(37,100))
		:inclination(math.deg2rad(0))
		:rotation_period(f(339,10))
		:axial_tilt(fixed.deg2rad(f(62,10)))
		:metallicity(f(422,1000))
		:volcanicity(f(60,100))
		:atmos_density(f(0,10))
		:atmos_oxidizing(f(0,10))
		:ocean_cover(f(0,1000))
		:ice_cover(f(0,1000))
		:life(f(0,10)),
	}

local taucetii4 = {
	CustomSystemBody:new('Celeng', 'PLANET_TERRESTRIAL') --Celeng Wayungyang is a character in Javanese folklore
		:seed(681108061)		
		:radius(f(1842,10000))
		:mass(f(62,10000))
		:temp(40)
		:semi_major_axis(f(48,1000))
		:eccentricity(f(7,100))
		:inclination(math.deg2rad(0))
		:rotation_period(f(30,10))
		:axial_tilt(fixed.deg2rad(f(47,10)))
		:metallicity(f(341,1000))
		:volcanicity(f(4,100))
		:atmos_density(f(0,10))
		:atmos_oxidizing(f(0,10))
		:ocean_cover(f(0,1000))
		:ice_cover(f(0,1000))
		:life(f(0,10)),
	}
	
s:bodies(tauceti, {

	taucetia,
	taucetib,
	taucetic,
		taucetic_starports,
	taucetid,
	taucetie,
		taucetie_starports,
		taucetie_moon,
	taucetif,
	taucetig,
		taucetig1,
	taucetih,
		taucetih1,
		taucetih2,
		taucetih3,
		taucetih4,
	taucetii,
		taucetii1,
		taucetii2,
		taucetii3,
		taucetii4,

	})

s:add_to_sector(0,-2,-1,v(0.627,0.715,0.592))
