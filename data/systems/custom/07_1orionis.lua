-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

--Names don't infringe any copyright (see comments after each name)


local s = CustomSystem:new('1 Orionis',{ 'STAR_F' }) --Alt name will be "Cluaran", means "thisle" in scottish
	:faction('Commonwealth of Independent Worlds')
	:govtype('CISLIBDEM')
	:lawlessness(f(5,100)) -- 1/100th from a peaceful eden
	:short_desc('A proudly independent hisoric colony')
	:long_desc([["Proud World for Proud People".
1 Orionis' motto can be traced back to year 2325, as the "Evergreen" -one of the legacy colony ships launched before the era of hyperspace travel- landed on planet Saorsa. Its founding fathers vowed to never live again under Terran oppression,
This heritage still shapes the system’s values to this day : independence, pride, and freedom.
1 Orionis acively took part in the War of Hope (2723-2725), as its volunteer forces fought alongside Epsilon Eridani against Sol’s attempt of hegemony. The system is a founding member of the Commonwealth of Independent Worlds and remains politically influent to this day.
In present day, the systems boasts a strong industry and is the birthplace of many an elite pilot of the Confederation Fleet.
Travellers will also find the citizens are a proud lot : be it of their tradition, their chilly climate, their language, their gas giants...]])

local orionis = CustomSystemBody:new('1 Orionis', 'STAR_F') --Alt name will be "Cluaran", means "thisle" in scottish
	:radius(f(113,100))
	:mass(f(161,100))
	:temp(6387)

local orionisa = CustomSystemBody:new('Raineach', 'PLANET_TERRESTRIAL') --Real scottish town
	:radius(f(2676,10000))
	:mass(f(1918,100000))
	:temp(1495)
	:semi_major_axis(f(48,1000))
	:eccentricity(f(22,100))
	:inclination(math.deg2rad(0.36))
	:rotation_period(f(5,1))
	:axial_tilt(fixed.deg2rad(f(25,10)))
	:metallicity(f(39,100))
	:volcanicity(f(17,1000))
	:atmos_density(f(0,1))
	:atmos_oxidizing(f(0,1))
	:ocean_cover(f(0,1))
	:ice_cover(f(0,1))
	:life(f(0,1))

local orionisb = CustomSystemBody:new('Gearrloch', 'PLANET_TERRESTRIAL') --Real scottish town
	:radius(f(5403,10000))
	:mass(f(1578,10000))
	:temp(929)
	:semi_major_axis(f(1245,10000))
	:eccentricity(f(14,100))
	:inclination(math.deg2rad(3.7))
	:rotation_period(f(126,10))
	:axial_tilt(fixed.deg2rad(f(0,1)))
	:metallicity(f(84,1000))
	:volcanicity(f(49,1000))
	:atmos_density(f(0,1))
	:atmos_oxidizing(f(0,1))
	:ocean_cover(f(0,1))
	:ice_cover(f(0,1))
	:life(f(0,1))

local orionisc = CustomSystemBody:new('Dòrnach', 'PLANET_TERRESTRIAL') --Real scottish town
	:radius(f(7154,10000))
	:mass(f(3661,10000))
	:temp(639)
	:semi_major_axis(f(2625,10000))
	:eccentricity(f(3,100))
	:inclination(math.deg2rad(1.5))
	:rotation_period(f(36,1))
	:axial_tilt(fixed.deg2rad(f(14,10)))
	:metallicity(f(190,1000))
	:volcanicity(f(281,1000))
	:atmos_density(f(0,1))
	:atmos_oxidizing(f(0,1))
	:ocean_cover(f(0,1))
	:ice_cover(f(0,1))
	:life(f(0,1))

local orionisc_starport =	{
		CustomSystemBody:new('Àird Fheàrna', 'STARPORT_ORBITAL') -- Real scottish town
			:semi_major_axis(f(8068,100000000))
			:rotation_period(f(11,24))
	}

local orionisd = CustomSystemBody:new('Neart', 'PLANET_TERRESTRIAL') --Means "strenght" in scottish
	:radius(f(11139,10000))
	:mass(f(12408,10000))
	:temp(1284)
	:semi_major_axis(f(4415,10000))
	:eccentricity(f(6,100))
	:inclination(math.deg2rad(5.6))
	:rotation_period(f(503,10))
	:axial_tilt(fixed.deg2rad(f(112,10)))
	:metallicity(f(399,1000))
	:volcanicity(f(847,1000))
	:atmos_density(f(1571,1000))
	:atmos_oxidizing(f(8,10))
	:ocean_cover(f(118,1000))
	:ice_cover(f(5,1000))
	:life(f(0,1))

local orionisf = CustomSystemBody:new('Ì', 'PLANET_GAS_GIANT') --Real scottish island
	:radius(f(33959,10000))
	:mass(f(115321,10000))
	:temp(249)
	:semi_major_axis(f(1572,1000))
	:eccentricity(f(27,100))
	:inclination(math.deg2rad(0.3))
	:rotation_period(f(60,10))
	:axial_tilt(fixed.deg2rad(f(197,10)))
	:metallicity(f(295,1000))
	:volcanicity(f(457,1000))
	:atmos_density(f(0,1))
	:atmos_oxidizing(f(0,1))
	:ocean_cover(f(0,1))
	:ice_cover(f(0,1))
	:life(f(0,1))

local saorsa = {
	CustomSystemBody:new('Saorsa', 'PLANET_TERRESTRIAL') --means "freedom" in Scottish
		:seed(65)
		:radius(f(9589,10000))
		:mass(f(8818,10000))
		:temp(278)
		:semi_major_axis(f(7,1000))
		:eccentricity(f(9,1000))
		:inclination(math.deg2rad(0.2))
		:rotation_period(f(2,1))
		:axial_tilt(fixed.deg2rad(f(112,10)))
		:metallicity(f(768,1000))
		:volcanicity(f(16,100))
		:atmos_density(f(8,10))
		:atmos_oxidizing(f(10,10))
		:ocean_cover(f(239,1000))
		:ice_cover(f(467,1000))
		:life(f(45,100)),
	{
		CustomSystemBody:new('Dùn Éideann Ùr', 'STARPORT_SURFACE')
			:latitude(math.deg2rad(13.1308))
			:longitude(math.deg2rad(-80.1142)),
		CustomSystemBody:new('Talmine', 'STARPORT_SURFACE') --Real Scottish town
			:latitude(math.deg2rad(77.0921))
			:longitude(math.deg2rad(-57.0348)),
		CustomSystemBody:new('Dun Adhar', 'STARPORT_ORBITAL') -- Made up, should mean "Sky city"
			:semi_major_axis(f(15068,100000000))
			:rotation_period(f(11,24)),
	},
	}


local orionisg = CustomSystemBody:new('Pròis', 'PLANET_TERRESTRIAL') -- Pròis means "pride" in scottish
	:seed(-419185643)
	:radius(f(13314,10000))
	:mass(f(17727,10000))
	:temp(119)
	:semi_major_axis(f(30155,10000))
	:eccentricity(f(7,100))
	:inclination(math.deg2rad(2.3))
	:rotation_period(f(73,10))
	:axial_tilt(fixed.deg2rad(f(1,10)))
	:metallicity(f(284,1000))
	:volcanicity(f(277,1000))
	:atmos_density(f(855,1000))
	:atmos_oxidizing(f(8,10))
	:ocean_cover(f(506,1000))
	:ice_cover(f(213,1000))
	:life(f(0,1))

local orionisg_moon = {
	CustomSystemBody:new('Carraig Dhubh', 'PLANET_TERRESTRIAL') --Carraig Dhubh : made up, should mean "black rock"
		:seed(-668520084)
		:radius(f(1023,10000))
		:mass(f(11,10000))
		:temp(119)
		:semi_major_axis(f(276,1000000))
		:eccentricity(f(1,10))
		:inclination(math.deg2rad(0.2))
		:rotation_period(f(2,1))
		:axial_tilt(fixed.deg2rad(f(112,10)))
		:metallicity(f(258,1000))
		:volcanicity(f(0,100))
		:atmos_density(f(0,10))
		:atmos_oxidizing(f(0,10))
		:ocean_cover(f(0,1000))
		:ice_cover(f(0,1000))
		:life(f(0,10))
	}

local orionish = CustomSystemBody:new('Mac an t-Sealgair', 'PLANET_TERRESTRIAL') --An Sealgair Mór (the big hunter) is Scottish for "Orion", so I went with "son of the hunter" ;)
	:seed(-193118230)
	:radius(f(17469,10000))
	:mass(f(30518,10000))
	:temp(52)
	:semi_major_axis(f(38990,10000))
	:eccentricity(f(9,100))
	:inclination(math.deg2rad(-3.7))
	:rotation_period(f(2,1))
	:axial_tilt(fixed.deg2rad(f(289,10)))
	:metallicity(f(237,1000))
	:volcanicity(f(184,1000))
	:atmos_density(f(2731,1000))
	:atmos_oxidizing(f(8,10))
	:ocean_cover(f(2053,1000))
	:ice_cover(f(1237,1000))
	:life(f(0,1))

local orionish_moons = {
	CustomSystemBody:new('An Càrn Dubh', 'PLANET_ASTEROID') --"Cairndow", real scottish town
		:seed(599155018)
		:radius(f(348,10000))
		:mass(f(42,1000000))
		:temp(131)
		:semi_major_axis(f(450,1000000))
		:eccentricity(f(1,10))
		:inclination(math.deg2rad(0.1))
		:rotation_period(f(12,10))
		:axial_tilt(fixed.deg2rad(f(0,10)))
		:metallicity(f(49,1000))
		:volcanicity(f(0,100))
		:atmos_density(f(0,10))
		:atmos_oxidizing(f(0,10))
		:ocean_cover(f(0,1000))
		:ice_cover(f(0,1000))
		:life(f(0,10)),

	CustomSystemBody:new('Eige', 'PLANET_TERRESTRIAL') --"Eigg", real scottish island
		:seed(-499886056)
		:radius(f(517,10000))
		:mass(f(13,100000))
		:temp(83)
		:semi_major_axis(f(1207,1000000))
		:eccentricity(f(35,100))
		:inclination(math.deg2rad(0))
		:rotation_period(f(51,10))
		:axial_tilt(fixed.deg2rad(f(0,10)))
		:metallicity(f(18,1000))
		:volcanicity(f(0,100))
		:atmos_density(f(0,10))
		:atmos_oxidizing(f(0,10))
		:ocean_cover(f(0,1000))
		:ice_cover(f(0,1000))
		:life(f(0,10))
	}

local orionisi = CustomSystemBody:new('Beàrnaraigh Mòr', 'PLANET_GAS_GIANT') --Real scottish island
	:seed(-1200031656)
	:radius(f(83255,10000))
	:mass(f(693151,10000))
	:temp(106)
	:semi_major_axis(f(9332,1000))
	:eccentricity(f(2,100))
	:inclination(math.deg2rad(1.7))
	:rotation_period(f(29,10))
	:axial_tilt(fixed.deg2rad(f(34,10)))
	:metallicity(f(130,1000))
	:volcanicity(f(459,1000))
	:atmos_density(f(0,1))
	:atmos_oxidizing(f(0,1))
	:ocean_cover(f(0,1))
	:ice_cover(f(0,1))
	:life(f(0,1))

local orionisi_moons = {
	CustomSystemBody:new('Luirg', 'PLANET_TERRESTRIAL') --Real scottish town
		:seed(247130418)
		:radius(f(1231,10000))
		:mass(f(1866,1000000))
		:temp(67)
		:semi_major_axis(f(1757,1000000))
		:eccentricity(f(9,100))
		:inclination(math.deg2rad(0.1))
		:rotation_period(f(19,10))
		:axial_tilt(fixed.deg2rad(f(0,10)))
		:metallicity(f(202,1000))
		:volcanicity(f(1,1000))
		:atmos_density(f(0,10))
		:atmos_oxidizing(f(0,10))
		:ocean_cover(f(0,1000))
		:ice_cover(f(0,1000))
		:life(f(0,10)),

	CustomSystemBody:new('An Càrn Mòr', 'PLANET_TERRESTRIAL') --Real scottish hill
		:seed(-679328091)
		:radius(f(2116,10000))
		:mass(f(948,100000))
		:temp(67)
		:semi_major_axis(f(3683,1000000))
		:eccentricity(f(25,100))
		:inclination(math.deg2rad(-0.3))
		:rotation_period(f(54,10))
		:axial_tilt(fixed.deg2rad(f(0,10)))
		:metallicity(f(68,1000))
		:volcanicity(f(4,1000))
		:atmos_density(f(0,10))
		:atmos_oxidizing(f(0,10))
		:ocean_cover(f(0,1000))
		:ice_cover(f(0,1000))
		:life(f(0,10)),

	CustomSystemBody:new('Errol', 'PLANET_TERRESTRIAL') --Real scottish hill
		:seed(-1350635343)
		:radius(f(2947,10000))
		:mass(f(2559,100000))
		:temp(67)
		:semi_major_axis(f(7262,1000000))
		:eccentricity(f(11,100))
		:inclination(math.deg2rad(0))
		:rotation_period(f(15.5,10))
		:axial_tilt(fixed.deg2rad(f(0,10)))
		:metallicity(f(308,1000))
		:volcanicity(f(22,1000))
		:atmos_density(f(0,10))
		:atmos_oxidizing(f(0,10))
		:ocean_cover(f(0,1000))
		:ice_cover(f(0,1000))
		:life(f(0,10))
	}

local orionisj = CustomSystemBody:new('Arainn', 'PLANET_GAS_GIANT') --Real scottish island
	:seed(-1412603713)
	:radius(f(116996,10000))
	:mass(f(16876247,10000))
	:temp(74)
	:semi_major_axis(f(19195,1000))
	:eccentricity(f(33,100))
	:inclination(math.deg2rad(0.9))
	:rotation_period(f(22,10))
	:axial_tilt(fixed.deg2rad(f(3,10)))
	:metallicity(f(239,1000))
	:volcanicity(f(105,1000))
	:atmos_density(f(0,1))
	:atmos_oxidizing(f(0,1))
	:ocean_cover(f(0,1))
	:ice_cover(f(0,1))
	:life(f(0,1))

local orionisj_moons = {
	CustomSystemBody:new('Dùn Omhain', 'PLANET_TERRESTRIAL') --Real scottish town
		:seed(504618464)
		:radius(f(4094,10000))
		:mass(f(686,10000))
		:temp(46)
		:semi_major_axis(f(12,1000))
		:eccentricity(f(64,100))
		:inclination(math.deg2rad(0))
		:rotation_period(f(25,10))
		:axial_tilt(fixed.deg2rad(f(0,10)))
		:metallicity(f(316,1000))
		:volcanicity(f(25,1000))
		:atmos_density(f(0,10))
		:atmos_oxidizing(f(0,10))
		:ocean_cover(f(0,1000))
		:ice_cover(f(0,1000))
		:life(f(0,10)),

	CustomSystemBody:new('Dùn Bheagan', 'PLANET_TERRESTRIAL') --Real scottish town
		:seed(1601618231)
		:radius(f(4003,10000))
		:mass(f(641,10000))
		:temp(46)
		:semi_major_axis(f(15,1000))
		:eccentricity(f(2,100))
		:inclination(math.deg2rad(0))
		:rotation_period(f(93,10))
		:axial_tilt(fixed.deg2rad(f(0,10)))
		:metallicity(f(31,1000))
		:volcanicity(f(27,1000))
		:atmos_density(f(0,10))
		:atmos_oxidizing(f(0,10))
		:ocean_cover(f(0,1000))
		:ice_cover(f(0,1000))
		:life(f(0,10)),

	CustomSystemBody:new('Ìle', 'PLANET_TERRESTRIAL') --Real scottish island
		:seed(-1495856331)
		:radius(f(8739,10000))
		:mass(f(6675,10000))
		:temp(46)
		:semi_major_axis(f(295,10000))
		:eccentricity(f(22,100))
		:inclination(math.deg2rad(0.1))
		:rotation_period(f(29.4,10))
		:axial_tilt(fixed.deg2rad(f(3,10)))
		:metallicity(f(712,1000))
		:volcanicity(f(287,1000))
		:atmos_density(f(0,10))
		:atmos_oxidizing(f(0,10))
		:ocean_cover(f(0,1000))
		:ice_cover(f(0,1000))
		:life(f(0,10)),

		{
		CustomSystemBody:new('Laphroaig', 'STARPORT_SURFACE') --Real scottish town
			:latitude(math.deg2rad(-4.5895))
			:longitude(math.deg2rad(-137.4417)),
		CustomSystemBody:new('Port Ilein', 'STARPORT_ORBITAL') -- Real scottish town
			:semi_major_axis(f(15068,100000000))
			:rotation_period(f(11,24)),
		},

	CustomSystemBody:new('An Ceathramh Mór', 'PLANET_TERRESTRIAL') --Real scottish town
		:seed(4268442)
		:radius(f(5151,10000))
		:mass(f(1367,10000))
		:temp(46)
		:semi_major_axis(f(74,1000))
		:eccentricity(f(15,100))
		:inclination(math.deg2rad(2.7))
		:rotation_period(f(18,10))
		:axial_tilt(fixed.deg2rad(f(34,10)))
		:metallicity(f(44,1000))
		:volcanicity(f(3,1000))
		:atmos_density(f(0,10))
		:atmos_oxidizing(f(0,10))
		:ocean_cover(f(0,1000))
		:ice_cover(f(0,1000))
		:life(f(0,10))
	}

local orionisk = CustomSystemBody:new('Am Blàr Dubh', 'PLANET_GAS_GIANT') --Real scottish town
	:seed(-976168363)
	:radius(f(121175,10000))
	:mass(f(11356529,10000))
	:temp(52)
	:semi_major_axis(f(3899,100))
	:eccentricity(f(9,100))
	:inclination(math.deg2rad(5.5))
	:rotation_period(f(20,10))
	:axial_tilt(fixed.deg2rad(f(289,10)))
	:metallicity(f(228,1000))
	:volcanicity(f(251,1000))
	:atmos_density(f(0,1))
	:atmos_oxidizing(f(0,1))
	:ocean_cover(f(0,1))
	:ice_cover(f(0,1))
	:life(f(0,1))

local orionisk_moons = {
	CustomSystemBody:new('Brùra', 'PLANET_TERRESTRIAL') --Real scottish town
		:seed(-2061656912)
		:radius(f(4780,10000))
		:mass(f(1092,10000))
		:temp(32)
		:semi_major_axis(f(75,1000))
		:eccentricity(f(36,100))
		:inclination(math.deg2rad(7.6))
		:rotation_period(f(9,10))
		:axial_tilt(fixed.deg2rad(f(86,10)))
		:metallicity(f(78,1000))
		:volcanicity(f(74,1000))
		:atmos_density(f(0,10))
		:atmos_oxidizing(f(0,10))
		:ocean_cover(f(0,1000))
		:ice_cover(f(0,1000))
		:life(f(0,10)),

	CustomSystemBody:new('Iùbh', 'PLANET_TERRESTRIAL') --Real scottish town
		:seed(-2118775515)
		:radius(f(7746,10000))
		:mass(f(4647,10000))
		:temp(32)
		:semi_major_axis(f(186,1000))
		:eccentricity(f(19,100))
		:inclination(math.deg2rad(0.1))
		:rotation_period(f(81,10))
		:axial_tilt(fixed.deg2rad(f(156,10)))
		:metallicity(f(153,1000))
		:volcanicity(f(325,1000))
		:atmos_density(f(58,1000))
		:atmos_oxidizing(f(10,10))
		:ocean_cover(f(71,1000))
		:ice_cover(f(110,1000))
		:life(f(0,10)),

	CustomSystemBody:new('Càrn nan Cat', 'PLANET_TERRESTRIAL') --Real scottish town, and who doesn't like cats ;)
		:seed(-1679030052)
		:radius(f(6769,10000))
		:mass(f(3103,10000))
		:temp(32)
		:semi_major_axis(f(421,1000))
		:eccentricity(f(22,100))
		:inclination(math.deg2rad(0))
		:rotation_period(f(50,10))
		:axial_tilt(fixed.deg2rad(f(25,10)))
		:metallicity(f(376,1000))
		:volcanicity(f(81,1000))
		:atmos_density(f(0,10))
		:atmos_oxidizing(f(0,10))
		:ocean_cover(f(0,1000))
		:ice_cover(f(0,1000))
		:life(f(0,10))
	}


s:bodies(orionis, {

	orionisa,
	orionisb,
	orionisc,
		orionisc_starport,
	orionisd,
	orionisf,
		saorsa,
	orionisg,
		orionisg_moon,
	orionish,
		orionish_moons,
	orionisi,
		orionisi_moons,
	orionisj,
		orionisj_moons,
	orionisk,
		orionisk_moons,

	})

s:add_to_sector(3,-1,0,v(0.097,0.021,0.397))
