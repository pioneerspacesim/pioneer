local s = CustomSystem:new('Sol', { Body.Type.STAR_G })

s:short_desc('The historical birthplace of humankind')
s:long_desc([[Sol is a fine joint]])

local sol = CustomSBody:new('Sol', Body.Type.STAR_G)
sol:radius(f(1,1))
sol:mass(f(1,1))
sol:temp(5700)

local mercury = CustomSBody:new('Mercury', Body.Type.PLANET_TERRESTRIAL)
mercury:radius(f(38,100))
mercury:mass(f(55,1000))
mercury:temp(340)
mercury:semi_major_axis(f(387,1000))
mercury:eccentricity(f(205,1000))
mercury:latitude(math.deg2rad(7.0))
mercury:rotation_period(f(59,1))
mercury:axial_tilt(math.fixed.deg2rad(f(1,100)))

local venus = CustomSBody:new('Venus', Body.Type.PLANET_TERRESTRIAL)
venus:radius(f(95,100))
venus:mass(f(815,1000))
venus:temp(735)
venus:semi_major_axis(f(723,1000))
venus:eccentricity(f(7,1000))
venus:latitude(math.deg2rad(3.39))
venus:rotation_period(f(243,1))
venus:axial_tilt(math.fixed.deg2rad(f(26,10)))

local earth = CustomSBody:new('Earth', Body.Type.PLANET_TERRESTRIAL)
earth:radius(f(1,1))
earth:mass(f(1,1))
earth:temp(288)
earth:semi_major_axis(f(1,1))
earth:eccentricity(f(167,10000))
earth:rotation_period(f(1,1))
earth:axial_tilt(math.fixed.deg2rad(f(2344,100)))
earth:height_map('data/earth.hmap')

local mars = CustomSBody:new('Mars', Body.Type.PLANET_TERRESTRIAL)
mars:radius(f(533,1000))
mars:mass(f(107,1000))
mars:temp(274)
mars:semi_major_axis(f(152,100))
mars:eccentricity(f(933,10000))
mars:latitude(math.deg2rad(1.85))
mars:rotation_period(f(1027,1000))
mars:axial_tilt(math.fixed.deg2rad(f(2519,100)))

local jupiter = CustomSBody:new('Jupiter', Body.Type.PLANET_GAS_GIANT)
jupiter:radius(f(11,1))
jupiter:mass(f(3178,10))
jupiter:temp(165)
jupiter:semi_major_axis(f(5204,1000))
jupiter:eccentricity(f(488,10000))
jupiter:latitude(math.deg2rad(1.305))
jupiter:rotation_period(f(4,10))
jupiter:axial_tilt(math.fixed.deg2rad(f(313,100)))

local saturn = CustomSBody:new('Saturn', Body.Type.PLANET_GAS_GIANT)
saturn:radius(f(9,1))
saturn:mass(f(95152,1000))
saturn:temp(134)
saturn:semi_major_axis(f(9582,1000))
saturn:eccentricity(f(557,10000))
saturn:latitude(math.deg2rad(2.485))
saturn:rotation_period(f(4,10))
saturn:axial_tilt(math.fixed.deg2rad(f(2673,100)))

local uranus = CustomSBody:new('Uranus', Body.Type.PLANET_GAS_GIANT)
uranus:radius(f(4,1))
uranus:mass(f(145,10))
uranus:temp(76)
uranus:semi_major_axis(f(19229,1000))
uranus:eccentricity(f(444,10000))
uranus:latitude(math.deg2rad(0.772))
uranus:rotation_period(f(7,10))
uranus:axial_tilt(math.fixed.deg2rad(f(9777,100)))

local neptune = CustomSBody:new('Neptune', Body.Type.PLANET_GAS_GIANT)
neptune:radius(f(38,10))
neptune:mass(f(17147,100))
neptune:temp(72)
neptune:semi_major_axis(f(30104,1000))
neptune:eccentricity(f(112,10000))
neptune:latitude(math.deg2rad(1.768))
neptune:rotation_period(f(75,100))
neptune:axial_tilt(math.fixed.deg2rad(f(2832,100)))

local pluto = CustomSBody:new('Pluto', Body.Type.PLANET_TERRESTRIAL)
pluto:radius(f(18,100))
pluto:mass(f(21,10000))
pluto:temp(44)
pluto:semi_major_axis(f(394,10))
pluto:eccentricity(f(249,1000))
pluto:latitude(math.deg2rad(11.88))
pluto:rotation_period(f(153,24))
pluto:axial_tilt(math.fixed.deg2rad(f(296,10)))

s:bodies(sol, {
	mercury,
	venus,
	earth,
	mars,
	saturn,
	uranus,
	neptune,
	pluto
})

s:add_to_sector(0,0,v(0.5,0.5,0))

--[[
earth_children = {
	{
		name('Shanghai')
		type(TYPE_STARPORT_SURFACE)
		latitude(math.deg2rad(31))
		longitude(math.deg2rad(-121))
	}, {
		name('Mexico City')
		type(TYPE_STARPORT_SURFACE)
		latitude(math.deg2rad(19))
		longitude(math.deg2rad(99))
	}, {
		name('London')
		type(TYPE_STARPORT_SURFACE)
		latitude(math.deg2rad(51))
		longitude(0)
	}, {
		name('Moscow')
		type(TYPE_STARPORT_SURFACE)
		latitude(math.deg2rad(55))
		longitude(math.deg2rad(-37.5))
	}, {
		name('Brasilia')
		type(TYPE_STARPORT_SURFACE)
		latitude(math.deg2rad(-15.5))
		longitude(math.deg2rad(48))
	}, {
		name('Los Angeles')
		type(TYPE_STARPORT_SURFACE)
		latitude(math.deg2rad(34))
		longitude(math.deg2rad(118))
	}, {
		name('Gates Spaceport')
		type(TYPE_STARPORT_ORBITAL)
		semi_major_axis(f(100,100000))
		rotation_period(f(1)24*60*3)
	}, {
		name('Moon')
		type(TYPE_PLANET_DWARF)
		radius(f(273,1000))
		mass(f(12,1000))
		temp(220)
		semi_major_axis(f(257,100000))
		eccentricity(f(549,10000))
		latitude(math.deg2rad(5.145))
		rotation_period(f(273,10))
		axial_tilt(math.fixed.deg2rad(f(668,100)))
		children = { {
			name('Lunar City')
			type(TYPE_STARPORT_SURFACE)
			latitude(math.deg2rad(19))
			longitude(math.deg2rad(99))
		} },
	},
}

mars_children = {
	{
		name('Cydonia')
		type(TYPE_STARPORT_SURFACE)
		latitude(math.deg2rad(-29))
		longitude(math.deg2rad(124))
	}, {
		name('Olympus Mons')
		type(TYPE_STARPORT_SURFACE)
		latitude(math.deg2rad(30))
		longitude(math.deg2rad(-37))
	}, {
		name('Mars High')
		type(TYPE_STARPORT_ORBITAL)
		semi_major_axis(f(5068,100000000))
		rotation_period(f(11,24))
	}, {
		name('Phobos')
		type(TYPE_PLANET_LARGE_ASTEROID)
		radius(f(21,10000))
		mass(f(18,10000000000))
		temp(233)
		semi_major_axis(f(6268,100000000))
		eccentricity(f(151,10000))
		latitude(math.deg2rad(1.093))
		rotation_period(f(11,24))
		children = { {
			name('Phobos Base')
			type(TYPE_STARPORT_SURFACE)
			latitude(math.deg2rad(5))
			longitude(math.deg2rad(-5))
		} },
	}, {
		name('Deimos')
		type(TYPE_PLANET_ASTEROID)
		radius(f(12,10000))
		mass(f(25,100000000000))
		temp(233)
		semi_major_axis(f(1568,10000000))
		eccentricity(f(2,10000))
		latitude(math.deg2rad(0.93))
		rotation_period(f(30,24))
		children = { {
			name('Tomm\'s Sanctuary')
			type(TYPE_STARPORT_SURFACE)
		} },
	},
}

jupiter_children = {
	{
		name('Io')
		type(TYPE_PLANET_HIGHLY_VOLCANIC)
		radius(f(286,1000))
		mass(f(15,1000))
		temp(130)
		semi_major_axis(f(282,100000))
		eccentricity(f(41,10000))
		latitude(math.deg2rad(2.21))
		rotation_period(f(177,100))
		children = { {
			name('Dante\'s Base')
			type(TYPE_STARPORT_SURFACE)
			latitude(math.deg2rad(-0.5))
			longitude(math.deg2rad(26.2))
		} },
	}, {
		name('Europa')
		type(TYPE_PLANET_WATER)
		radius(f(245,1000))
		mass(f(8,1000))
		temp(102)
		semi_major_axis(f(441,100000))
		eccentricity(f(9,1000))
		rotation_period(f(355,100))
		children = { {
			name('Clarke\'s Station')
			type(TYPE_STARPORT_ORBITAL)
			semi_major_axis(f(12,500000))
			rotation_period(f(1,24*60*3))
		} },
	}, {
		name('Ganymede')
		type(TYPE_PLANET_SMALL)
		radius(f(413,1000))
		mass(f(25,1000))
		temp(110)
		semi_major_axis(f(72,10000))
		eccentricity(f(13,10000))
		latitude(math.deg2rad(0.2))
		rotation_period(f(72,10))
		children = { {
			name('Enki Catena')
			type(TYPE_STARPORT_SURFACE)
			latitude(math.deg2rad(84))
			longitude(math.deg2rad(96))
		} },
	}, {
		name('Callisto')
		type(TYPE_PLANET_DWARF)
		radius(f(378,1000))
		mass(f(18,1000))
		temp(134)
		semi_major_axis(f(126,10000))
		eccentricity(f(74,10000))
		latitude(math.deg2rad(0.192))
		rotation_period(f(167,10))
	}, {
		name('Discovery Base')
		type(TYPE_STARPORT_ORBITAL)
		semi_major_axis(f(7,1000))
		rotation_period(f(11,1))
	},
}

saturn_children = {
	{
		name('Titan')
		type(TYPE_PLANET_METHANE_THICK_ATMOS)
		radius(f(400,1000))
		mass(f(225,10000))
		temp(94)
		semi_major_axis(f(82,10000))
		eccentricity(f(288,10000))
		latitude(math.deg2rad(0.34854))
		rotation_period(f(15945,1000))
		children = { {
			name('Oasis City')
			type(TYPE_STARPORT_SURFACE)
			latitude(math.deg2rad(18.4))
			longitude(math.deg2rad(196))
		}, {
			name('Port Makenzie')
			type(TYPE_STARPORT_SURFACE)
			latitude(math.deg2rad(1))
			longitude(math.deg2rad(14))
		}, {
			name('Daniel\'s Haven')
			type(TYPE_STARPORT_ORBITAL)
			semi_major_axis(f(12,500000))
			eccentricity(f(50,1000))
			rotation_period(f(11,9))
		} },
	}, {
		name('Rhea')
		type(TYPE_PLANET_DWARF)
		radius(f(12,100))
		mass(f(39,10000))
		temp(81)
		semi_major_axis(f(441,100000))
		eccentricity(f(126,100000))
		latitude(math.deg2rad(0.345))
		rotation_period(f(452,100))
	}, {
		name('Iapetus')
		type(TYPE_PLANET_DWARF)
		radius(f(1155,10000))
		mass(f(3,10000))
		temp(115)
		semi_major_axis(f(238,10000))
		eccentricity(f(29,1000))
		latitude(math.deg2rad(15.47))
		rotation_period(f(7932,100))
	}, {
		name('Dione')
		type(TYPE_PLANET_DWARF)
		radius(f(881,10000))
		mass(f(328,1000000))
		temp(87)
		semi_major_axis(f(252,100000))
		eccentricity(f(22,10000))
		latitude(math.deg2rad(0.019))
		rotation_period(f(2737,1000))
	},
}

uranus_children = {
	{
		name('Titania')
		type(TYPE_PLANET_DWARF)
		radius(f(1235,10000))
		mass(f(5908,10000000))
		temp(70)
		semi_major_axis(f(2913,1000000))
		eccentricity(f(11,10000))
		latitude(math.deg2rad(0.34))
		rotation_period(f(87,10))
	}, {
		name('Oberon')
		type(TYPE_PLANET_DWARF)
		radius(f(1194,10000))
		mass(f(5046,10000000))
		temp(75)
		semi_major_axis(f(39,10000))
		eccentricity(f(14,10000))
		latitude(math.deg2rad(0.058))
		rotation_period(f(135,10))
	}, {
		name('Umbriel')
		type(TYPE_PLANET_DWARF)
		radius(f(92,1000))
		mass(f(2,10000))
		temp(75)
		semi_major_axis(f(178,100000))
		eccentricity(f(39,10000))
		latitude(math.deg2rad(0.128))
		rotation_period(f(4144,1000))
	}, {
		name('Ariel')
		type(TYPE_PLANET_DWARF)
		radius(f(908,10000))
		mass(f(226,1000000))
		temp(60)
		semi_major_axis(f(1277,1000000))
		eccentricity(f(12,10000))
		latitude(math.deg2rad(0.26))
		rotation_period(f(252,100))
	},
}

neptune_children = {
	{
		name('Triton')
		type(TYPE_PLANET_WATER)
		radius(f(2122,10000))
		mass(f(359,100000))
		temp(38)
		semi_major_axis(f(2371,100000))
		eccentricity(f(16,1000000))
		latitude(math.deg2rad(156.885))
		rotation_period(f(141,24))
		children = { {
			name('Poseidon Station')
			type(TYPE_STARPORT_ORBITAL)
			semi_major_axis(f(12,500000))
			rotation_period(f(11,7))
		} },
	}, {
		name('Nereid')
		type(TYPE_PLANET_ASTEROID)
		radius(f(267,10000))
		mass(f(519,100000000))
		temp(50)
		semi_major_axis(f(3685,100000))
		eccentricity(f(75,100))
		latitude(math.deg2rad(32.55))
		rotation_period(f(115,240))
	}, {
		name('Proteus')
		type(TYPE_PLANET_LARGE_ASTEROID)
		radius(f(310,10000))
		mass(f(710,100000000))
		temp(51)
		semi_major_axis(f(786,1000000))
		eccentricity(f(53,100000))
		latitude(math.deg2rad(0.524))
		rotation_period(f(1122,1000))
	},
}

pluto_children = {
	{
		name('Pluto Research Base')
		type(TYPE_STARPORT_SURFACE)
		latitude(math.deg2rad(84))
		longitude(math.deg2rad(96))
	},
}
]]--

