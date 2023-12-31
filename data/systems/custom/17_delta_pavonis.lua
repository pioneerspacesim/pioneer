-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local system = CustomSystem:new('Delta Pavonis', { 'STAR_G_GIANT',  })
	:govtype('EARTHDEMOC')
	:short_desc('Underground industrial colony.')
	:long_desc([["My place, my life, my air": this advertisement for real estate could summarize  the way of life in Delta Pavonis.
Cities here are built deep underground according to the traditional "cellular" architecture, inspired by the human body. People live in individual or familial quarters called "Cells". Those Cells are connected to the rest of the city through "Arteries" where everything transits: people, goods and life support.
Air and water being scarce, settlements are designed to reduce their consumption.  As to avoid waste of volume, the cities are densely built; Cells are usually tiny and so are the workplaces; there are no gardens except in luxury residences; public areas are almost non-existent besides the Arteries.
Some foreigners depict Pavonian cities as overcrowded caves of steel. Yet, the locals are content with their environment.
As they spend their life in enclosed areas (including their vehicles), the Pavonians do not get to see the crowd, but rather their friends, colleagues, partners or family members. Those areas seem spacious, thanks to the dream-windows covering walls and ceilings. There is a broad choice of sceneries to display, including top seller "bivouac on the prairie".
Cause or consequence? The local culture is highly individualistic. Air in particular is a strictly personal, intimate commodity: even at work or at school, people are billed as they open the tap to refill their personal air tank.
"Let us breathe together" is a traditional blessing at marriages, synonymous with a lovely intimate moment. This expression also describes the way of life on an outdoor world: an outrageous promiscuity.]])


local deltapavonis = CustomSystemBody:new("Delta Pavonis", 'STAR_G_GIANT')
	:radius(f(176100,10000))
	:mass(f(21800,10000))
	:seed(1194586800)
	:temp(5113)

local deltapavonisa = CustomSystemBody:new("Sunnan", 'PLANET_GAS_GIANT') --Old Norse for "south"
	:radius(f(138841,10000))
	:mass(f(2444340,10000))
	:seed(868802962)
	:temp(269)
	:semi_major_axis(f(148123,10000))
	:eccentricity(f(902,10000))
	:rotation_period(f(59583,10000))
	:axial_tilt(fixed.deg2rad(f(3632,10000)))

local deltapavonisa1 = CustomSystemBody:new("Alep", 'PLANET_TERRESTRIAL') --Letter in the Hebraic alphabet
	:radius(f(1958,10000))
	:mass(f(75,10000))
	:seed(1908326657)
	:temp(225)
	:semi_major_axis(f(47,10000))
	:eccentricity(f(377,10000))
	:rotation_period(f(43770,10000))
	:metallicity(f(862,10000))
	:volcanicity(f(3,10000))
	:atmos_density(f(0,10000))
	:atmos_oxidizing(f(8193,10000))
	:ocean_cover(f(0,10000))
	:ice_cover(f(0,10000))
	:life(f(0,10000))

local deltapavonisa2 = CustomSystemBody:new("Areklandria", 'PLANET_TERRESTRIAL') --Suggested on the forum
	:radius(f(2796,10000))
	:mass(f(219,10000))
	:seed(1621520543)
	:temp(225)
	:semi_major_axis(f(95,10000))
	:eccentricity(f(11,10000))
	:rotation_period(f(124442,10000))
	:axial_tilt(fixed.deg2rad(f(6,10000)))
	:rotational_phase_at_start(fixed.deg2rad(f(0,10000)))
	:orbital_phase_at_start(fixed.deg2rad(f(0,10000)))
	:orbital_offset(fixed.deg2rad(f(0,10000)))
	:metallicity(f(5147,10000))
	:volcanicity(f(117,10000))
	:atmos_density(f(0,10000))
	:ocean_cover(f(0,10000))
	:ice_cover(f(0,10000))
	:life(f(0,10000))

local deltapavonisa2_starports = {
	CustomSystemBody:new('New Tellus', 'STARPORT_SURFACE') --Suggested on the forum
		:latitude(math.deg2rad(1.050833))
		:longitude(math.deg2rad(51.36694)),
	CustomSystemBody:new('Bandiagara', 'STARPORT_SURFACE') --Cave Town(s) in Mali
		:latitude(math.deg2rad(2.480556))
		:longitude(math.deg2rad(-95.13)),
	CustomSystemBody:new('Hansen', 'STARPORT_ORBITAL') --Original generated name
		:temp(225)
		:semi_major_axis(f(64,1000000))
		:rotation_period(f(3,10000)),
	CustomSystemBody:new("Guyaju", 'STARPORT_ORBITAL') --Cave town in China, Yangqing district
		:temp(225)
		:semi_major_axis(f(55,1000000))
		:rotation_period(f(3,10000)),
}

local deltapavonisa3 = CustomSystemBody:new("Ebal", 'PLANET_TERRESTRIAL') --A mountain
	:radius(f(4588,10000))
	:mass(f(966,10000))
	:seed(3497143830)
	:temp(225)
	:semi_major_axis(f(166,10000))
	:eccentricity(f(801,10000))
	:rotation_period(f(272890,10000))
	:axial_tilt(fixed.deg2rad(f(121,10000)))
	:metallicity(f(1828,10000))
	:volcanicity(f(955,10000))
	:atmos_density(f(0,10000))
	:ocean_cover(f(0,10000))
	:ice_cover(f(0,10000))
	:life(f(0,10000))


local deltapavonisa3_starports = {
	CustomSystemBody:new('Matmata', 'STARPORT_SURFACE') --Cave town in Tunisia
		:latitude(math.deg2rad(35.04694))
		:longitude(math.deg2rad(85.06806)),
	CustomSystemBody:new('Naours', 'STARPORT_SURFACE') --Cave town in France
		:latitude(math.deg2rad(-43.17917))
		:longitude(math.deg2rad(-98.42639))
}	
	
local deltapavonisa4 = CustomSystemBody:new("Bet", 'PLANET_TERRESTRIAL') --Letter in the Hebraic alphabet
	:radius(f(4160,10000))
	:mass(f(720,10000))
	:seed(443969960)
	:temp(225)
	:semi_major_axis(f(263,10000))
	:eccentricity(f(307,10000))
	:rotation_period(f(279685,10000))
	:axial_tilt(fixed.deg2rad(f(318,10000)))
	:metallicity(f(2736,10000))
	:volcanicity(f(139,10000))
	:atmos_density(f(0,10000))
	:ocean_cover(f(0,10000))
	:ice_cover(f(0,10000))
	:life(f(0,10000))

local deltapavonisb = CustomSystemBody:new("Serenia", 'PLANET_GAS_GIANT') --As suggested on the Forum
	:radius(f(122163,10000))
	:mass(f(10362351,10000))
	:seed(1506249889)
	:temp(177)
	:semi_major_axis(f(336886,10000))
	:eccentricity(f(1330,10000))
	:rotation_period(f(47917,10000))
	:axial_tilt(fixed.deg2rad(f(1644,10000)))

local deltapavonisb1 = CustomSystemBody:new("Gimel", 'PLANET_TERRESTRIAL') --Letter in the Hebraic alphabet
	:radius(f(1843,10000))
	:mass(f(63,10000))
	:seed(902582816)
	:temp(113)
	:semi_major_axis(f(111,10000))
	:eccentricity(f(109,10000))
	:rotation_period(f(76259,10000))
	:metallicity(f(1734,10000))
	:volcanicity(f(43,10000))
	:atmos_density(f(0,10000))
	:ocean_cover(f(0,10000))
	:ice_cover(f(0,10000))
	:life(f(0,10000))

local deltapavonisb2 = CustomSystemBody:new("Dalet", 'PLANET_TERRESTRIAL') --Letter in the Hebraic alphabet
	:radius(f(2576,10000))
	:mass(f(171,10000))
	:seed(2337162238)
	:temp(113)
	:semi_major_axis(f(184,10000))
	:eccentricity(f(961,10000))
	:rotation_period(f(161104,10000))
	:axial_tilt(fixed.deg2rad(f(37,10000)))
	:metallicity(f(4873,10000))
	:volcanicity(f(8,10000))
	:atmos_density(f(0,10000))
	:ocean_cover(f(0,10000))
	:ice_cover(f(0,10000))
	:life(f(0,10000))

local deltapavonisb3 = CustomSystemBody:new("Zayin", 'PLANET_TERRESTRIAL') --Letter in the Hebraic Alphabet
	:radius(f(3645,10000))
	:mass(f(484,10000))
	:seed(486652740)
	:temp(113)
	:semi_major_axis(f(343,10000))
	:eccentricity(f(1720,10000))
	:rotation_period(f(299937,10000))
	:axial_tilt(fixed.deg2rad(f(431,10000)))
	:metallicity(f(3241,10000))
	:volcanicity(f(245,10000))
	:atmos_density(f(0,10000))
	:ocean_cover(f(0,10000))
	:ice_cover(f(0,10000))
	:life(f(0,10000))

local deltapavonisb4 = CustomSystemBody:new("Yod", 'PLANET_TERRESTRIAL') --Letter in the Hebraic Alphabet
	:radius(f(6918,10000))
	:mass(f(3311,10000))
	:seed(3500171993)
	:temp(113)
	:semi_major_axis(f(723,10000))
	:eccentricity(f(1353,10000))
	:rotation_period(f(46250,10000))
	:axial_tilt(fixed.deg2rad(f(2440,10000)))
	:rotational_phase_at_start(fixed.deg2rad(f(0,10000)))
	:orbital_phase_at_start(fixed.deg2rad(f(0,10000)))
	:orbital_offset(fixed.deg2rad(f(0,10000)))
	:metallicity(f(705,10000))
	:volcanicity(f(540,10000))
	:atmos_density(f(0,10000))
	:atmos_oxidizing(f(1434,10000))
	:ocean_cover(f(0,10000))
	:ice_cover(f(0,10000))
	:life(f(0,10000))

local deltapavonisb5 = CustomSystemBody:new("Lamed", 'PLANET_TERRESTRIAL') --Letter in the Hebraic Alphabet
	:radius(f(7471,10000))
	:mass(f(4169,10000))
	:seed(1691665209)
	:temp(113)
	:semi_major_axis(f(1359,10000))
	:eccentricity(f(844,10000))
	:rotation_period(f(74167,10000))
	:axial_tilt(fixed.deg2rad(f(3876,10000)))
	:metallicity(f(2065,10000))
	:volcanicity(f(1792,10000))
	:atmos_density(f(0,10000))
	:ocean_cover(f(0,10000))
	:ice_cover(f(0,10000))
	:life(f(0,10000))

local deltapavonisb6 = CustomSystemBody:new("Samek", 'PLANET_TERRESTRIAL') --Letter in the Hebraic Alphabet
	:radius(f(5832,10000))
	:mass(f(1983,10000))
	:seed(2363556055)
	:temp(113)
	:semi_major_axis(f(2659,10000))
	:eccentricity(f(271,10000))
	:rotation_period(f(24583,10000))
	:axial_tilt(fixed.deg2rad(f(584,10000)))
	:metallicity(f(1088,10000))
	:volcanicity(f(386,10000))
	:atmos_density(f(0,10000))
	:ocean_cover(f(0,10000))
	:ice_cover(f(0,10000))
	:life(f(0,10000))

system:bodies(deltapavonis, 
	{
	deltapavonisa, 
	{
	deltapavonisa1, 
	deltapavonisa2,
		deltapavonisa2_starports, 
	deltapavonisa3,
		deltapavonisa3_starports, 
	deltapavonisa4, 
	}, 
	deltapavonisb, 
	{
	deltapavonisb1, 
	deltapavonisb2, 
	deltapavonisb3, 
	deltapavonisb4, 
	deltapavonisb5, 
	deltapavonisb6, 
	}, 
	})

system:add_to_sector(-1,-1,-3,v(0.1490,0.4630,0.7220))
