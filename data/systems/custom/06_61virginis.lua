-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local s = CustomSystem:new('61 Virginis',{ 'STAR_G' })
	:faction('Solar Federation')
	:govtype('EARTHCOLONIAL')
	:lawlessness(f(5,100)) -- 1/100th from a peaceful eden
	:short_desc('Solar Federation farming colony')
	:long_desc([[As Earth was facing more frequent food shortages, the Solar Federation founded several farming colonies, of which 61 Virginis was the most successful.
The SolFed government divided fertile lands between twelve companies, in order to massively export Terran crops. The results exceeded Sol's expectancies during the first decades. However, the drawbacks became clear over time.
The farms being continent-sized, and managed from the capitol city of Colheita, entire biomes were destroyed and many indigenous species are now extinct. The Terran crops are poorly adapted to Abundancia's climate, and are increasignly dependant on fertilizer and machine power.
While the planet still manages to produce food for many SolFed citizens, its yield is decreasing steadily. As such, several farming companies are openly considering a relocation of their activity on another planet.
The future remains to be written yet : the University of Colheita has become a leader in the field of environmental engineering, and many inhabitants believe the planet's ecosystem can be revived with careful engineering and farming of local crops. Some farming companies have allowed these experiments on their lands.]])

local virginis = CustomSystemBody:new('61 Virginis', 'STAR_G')
	:radius(f(11,10))
	:mass(f(1,1))
	:temp(5361)

local virginisa = CustomSystemBody:new('61 Virginis a', 'PLANET_GAS_GIANT')
	:radius(f(4579,1000))
	:mass(f(20966,1000))
	:temp(568)
	:semi_major_axis(f(207,1000))
	:eccentricity(f(6,100))
	:inclination(math.deg2rad(5.64))
	:rotation_period(f(327,10))
	:axial_tilt(fixed.deg2rad(f(0,10)))

local virginisb = CustomSystemBody:new('61 Virginis b', 'PLANET_GAS_GIANT')
	:radius(f(10402,1000))
	:mass(f(108204,1000))
	:temp(342)
	:semi_major_axis(f(571,1000))
	:eccentricity(f(4,100))
	:inclination(math.deg2rad(5.64))
	:rotation_period(f(833,10))
	:axial_tilt(fixed.deg2rad(f(124,10)))

local abundancia = CustomSystemBody:new('Abundância', 'PLANET_TERRESTRIAL')
	:seed(216)
	:radius(f(2078,1000))
	:mass(f(4319,1000))
	:temp(292)
	:semi_major_axis(f(856,1000))
	:eccentricity(f(02,100))
	:rotation_period(f(39,10))
	:axial_tilt(fixed.deg2rad(f(43,10)))
	:metallicity(f(1,100))
	:volcanicity(f(3,100))
	:atmos_density(f(10,10))
	:atmos_oxidizing(f(14,10))
	:ocean_cover(f(4,10))
	:ice_cover(f(5,100))
	:life(f(9,10))

local abundancia_starports =	{
		CustomSystemBody:new('Colheita', 'STARPORT_SURFACE') --Means "harvest" in portuguese
			:latitude(math.deg2rad(5.7541))
			:longitude(math.deg2rad(4.14)),

		CustomSystemBody:new('Gergelim', 'STARPORT_SURFACE') --Means "sesame" in portuguese
			:latitude(math.deg2rad(38.1))
			:longitude(math.deg2rad(159.1)),

		CustomSystemBody:new('Viagem', 'STARPORT_ORBITAL') --Means "travel" in portuguese
			:seed(13)
			:semi_major_axis(f(50,100000))
			:rotation_period(f(1,24*60*3)),
	}

local virginisd = CustomSystemBody:new('61 Virginis d', 'PLANET_GAS_GIANT')
	:radius(f(12732,1000))
	:mass(f(64985,100))
	:temp(243)
	:semi_major_axis(f(1134,1000))
	:eccentricity(f(15,100))
	:inclination(math.deg2rad(14.5))
	:rotation_period(f(33,10))
	:axial_tilt(fixed.deg2rad(f(21,10)))

local virginise = CustomSystemBody:new('61 Virginis e', 'PLANET_GAS_GIANT')
	:radius(f(12252,1000))
	:mass(f(1002516,100))
	:temp(174)
	:semi_major_axis(f(2195,1000))
	:eccentricity(f(9,100))
	:inclination(math.deg2rad(7.6))
	:rotation_period(f(28,10))
	:axial_tilt(fixed.deg2rad(f(67,10)))

s:bodies(virginis, {

	virginisa,
	virginisb,
	abundancia,
		abundancia_starports,
	virginisd,
	virginise,
	})
s:add_to_sector(-2,3,-2,v(0.893,0.108,0.908))
