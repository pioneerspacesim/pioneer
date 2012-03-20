local s = CustomSystem:new('Haber Corp Central', {'STAR_F','STAR_G','STAR_K','STAR_M'}) 
	:govtype('CORPORATE') 
	:seed(91152111)
	:short_desc('Centre of Corporate Space. Heavy Mining and Industry')
	:long_desc([[The remnants of Haber Corp that fled Earth arrived here in 2415 and immediately began recreating what they had lost.
	With no environmental laws the Haber corporation quickly rebuilt and spread throughout the surrounding systems, mining planets bare and leaving pollution in its wake.
	Haber attempted to destroy the Solar Federation after the War for Hope was lost, but were quickly routed by highly trained Earth forces.  The Haber Accord of 2789 enforced a maximum of five corporate systems.  
	Today, Haber continues to be a big supplier of raw materials and finished goods.  
	Working conditions are poor and foodstuffs and water are precious commodoties.  Any privateer wishing to land at a Haber Corp starport must verify they have foodstuffs in their cargo bay before landing.
]])

local HaberA = CustomSBody:new('Haber', 'STAR_F')
	:radius(f(50,10))
	:mass(f(610,1))
	:temp(4584)

local HaberB = CustomSBody:new('HaberB', 'STAR_G')
	:radius(f(30,10))
	:mass(f(610,1))
	:temp(4584)
	:semi_major_axis(f(12,100))
	:eccentricity(f(1,5000000))
	:rotation_period(f(0.2,1))
	

	local HaberC = CustomSBody:new('Sanctum','PLANET_TERRESTRIAL')
	:seed(43)
	:radius(f(5,10))
	:mass(f(6,10))
	:temp(500)
	:semi_major_axis(f(50,100))
	:eccentricity(f(1,10000))
	:rotation_period(f(300,100))
	:axial_tilt(math.fixed.deg2rad(f(10,100)))
	:metallicity(f(10,6))
	:volcanicity(f(200,100))
	:atmos_density(f(11,10))
	:atmos_oxidizing(f(10,10))
	:ocean_cover(f(1,10000))
	:ice_cover(f(1,10000))
	:life(f(1,51))
	
	local HaberC_STARPORTS = {
		CustomSBody:new('Sun Skimmer', 'STARPORT_ORBITAL')
		:semi_major_axis(f(100,100000))
		:rotation_period(f(1,24*60*3)),
		}
	
	
	
	local HaberD = CustomSBody:new('New Washington','PLANET_TERRESTRIAL')
	:seed(43)
	:radius(f(9,10))
	:mass(f(32,40))
	:temp(296)
	:semi_major_axis(f(12,10))
	:eccentricity(f(1,1000))
	:rotation_period(f(87,100))
	:axial_tilt(math.fixed.deg2rad(f(1741,100)))
	:metallicity(f(5,6))
	:volcanicity(f(20,100))
	:atmos_density(f(5,10))
	:atmos_oxidizing(f(25,10))
	:ocean_cover(f(70,100))
	:ice_cover(f(10,100))
	:life(f(1,51))
	
	
	
	local HaberE1 = CustomSBody:new('Life', 'STAR_K')
	:radius(f(8,10))
	:mass(f(610,1))
	:temp(4584)
	:semi_major_axis(f(3,1))
	:eccentricity(f(1,5000))
	:rotation_period(f(1,1))
	
	
	
	local HaberE2 = CustomSBody:new('Death', 'STAR_M')
	:radius(f(5,10))
	:mass(f(610,1))
	:temp(3642)
	:semi_major_axis(f(15,1000))
	:eccentricity(f(1,5))
	:rotation_period(f(1,1))
	
	
	local HaberE3 = CustomSBody:new('HaberE3','PLANET_TERRESTRIAL')
	:seed(13)
	:radius(f(65,100))
	:mass(f(35,100))
	:temp(400)
	:semi_major_axis(f(3,10))
	:eccentricity(f(1,1000))
	:rotation_period(f(2,1))
	:axial_tilt(math.fixed.deg2rad(f(41,100)))
	:metallicity(f(6,6))
	:volcanicity(f(40,100))
	:atmos_density(f(2,10))
	:atmos_oxidizing(f(2,10))
	:ocean_cover(f(100,10000))
	:ice_cover(f(1,10000000))
	:life(f(1,51))
	
	local HaberE4 = CustomSBody:new('Haber World','PLANET_TERRESTRIAL')
	:seed(43)
	:radius(f(15,10))
	:mass(f(45,40))
	:temp(300)
	:semi_major_axis(f(7,10))
	:eccentricity(f(1,500))
	:rotation_period(f(131,100))
	:axial_tilt(math.fixed.deg2rad(f(100,100)))
	:metallicity(f(2,6))
	:volcanicity(f(10,100))
	:atmos_density(f(915,100))
	:atmos_oxidizing(f(10,10))
	:ocean_cover(f(60,100))
	:ice_cover(f(25,100))
	:life(f(60,51))
	
	local HaberE4_STARPORTS = {
	CustomSBody:new('New Idaho City', 'STARPORT_SURFACE')
		:latitude(math.deg2rad(31))
		:longitude(math.deg2rad(-121)),
	CustomSBody:new('Industrial', 'STARPORT_SURFACE')
		:latitude(math.deg2rad(19))
		:longitude(math.deg2rad(99)),
	CustomSBody:new('Krugerberg', 'STARPORT_SURFACE')
		:latitude(math.deg2rad(51))
		:longitude(0),
	CustomSBody:new('Haber Orbital', 'STARPORT_ORBITAL')
		:semi_major_axis(f(100,100000))
		:rotation_period(f(1,24*60*3)),
		CustomSBody:new('Denise\'s Citadel', 'STARPORT_ORBITAL')
		:semi_major_axis(f(100,100000))
		:rotation_period(f(1,24*60*3)),
		}
	
	
	local HaberE5 = CustomSBody:new('Habers Second','PLANET_TERRESTRIAL')
	:seed(5974516545132002324165413)
	:radius(f(11,10))
	:mass(f(80,100))
	:temp(270)
	:semi_major_axis(f(13,10))
	:eccentricity(f(1,50))
	:rotation_period(f(90,100))
	:axial_tilt(math.fixed.deg2rad(f(341,100)))
	:metallicity(f(1,600))
	:volcanicity(f(20,100))
	:atmos_density(f(11,10))
	:atmos_oxidizing(f(13,10))
	:ocean_cover(f(70,100))
	:ice_cover(f(10,100))
	:life(f(40,51))
	
	local HaberE5_STARPORTS = {
	CustomSBody:new('Regans Landing', 'STARPORT_SURFACE')
		:latitude(math.deg2rad(31))
		:longitude(math.deg2rad(-121)),
	CustomSBody:new('Fort Knox', 'STARPORT_ORBITAL')
		:semi_major_axis(f(100,100000))
		:rotation_period(f(1,24*60*3)),
		}
	
	local HaberE6 = CustomSBody:new('HaberE6','PLANET_ASTEROID')
	:seed(12)
		:radius(f(17,1000))
		:mass(f(40,1000))
		:temp(120)
		:semi_major_axis(f(20,100))
		:eccentricity(f(1,10))
		:inclination(math.deg2rad(5.145))
		:rotation_period(f(9,10))
		:axial_tilt(math.fixed.deg2rad(f(500,100)))
		:volcanicity(f(1,100000000))
		:atmos_density(f(4,1000005))
		:ocean_cover(f(0,1000000000))
		:ice_cover(f(1500,2))
	
	local HaberE7 = CustomSBody:new('HaberE7','PLANET_ASTEROID')
	:seed(12)
		:radius(f(17,1000))
		:mass(f(170,1000))
		:temp(100)
		:semi_major_axis(f(10,100))
		:eccentricity(f(1,1000))
		:inclination(math.deg2rad(5.145))
		:rotation_period(f(9,10))
		:axial_tilt(math.fixed.deg2rad(f(100,100)))
		:volcanicity(f(1,10))
		:atmos_density(f(4,15))
		:ocean_cover(f(0,10))
		:ice_cover(f(1,1500))
	
	
	local HaberF = CustomSBody:new('HaberF','PLANET_GAS_GIANT')
	:radius(f(14,1))
	:mass(f(25,1))
	:temp(190)
	:semi_major_axis(f(50,10))
	:eccentricity(f(1,10000000))
	:inclination(math.deg2rad(1.305))
	:rotation_period(f(4,10))
	:axial_tilt(math.fixed.deg2rad(f(313,100)))
	
	local HaberG = CustomSBody:new('HaberG','PLANET_GAS_GIANT')
	:radius(f(14,1))
	:mass(f(11,1))
	:temp(144)
	:semi_major_axis(f(65,10))
	:eccentricity(f(1,10000000))
	:inclination(math.deg2rad(1.305))
	:rotation_period(f(4,10))
	:axial_tilt(math.fixed.deg2rad(f(313,100)))
	
	local HaberH = CustomSBody:new('Prison','PLANET_TERRESTRIAL')
	:seed(43)
	:radius(f(77,100))
	:mass(f(77,100))
	:temp(86)
	:semi_major_axis(f(85,10))
	:eccentricity(f(1,100))
	:rotation_period(f(87,100))
	:axial_tilt(math.fixed.deg2rad(f(1741,100)))
	:metallicity(f(2,6))
	:volcanicity(f(20,100))
	:atmos_density(f(11,10))
	:atmos_oxidizing(f(15,10))
	:ocean_cover(f(10,100))
	:ice_cover(f(1,100))
	:life(f(30,51))
	
	local HaberH_STARPORTS = {
	CustomSBody:new('Gulag 1', 'STARPORT_SURFACE')
		:latitude(math.deg2rad(31))
		:longitude(math.deg2rad(-121)),
	CustomSBody:new('Gulag 2', 'STARPORT_SURFACE')
		:latitude(math.deg2rad(19))
		:longitude(math.deg2rad(99)),
	CustomSBody:new('Prisoner Processing Station','STARPORT_ORBITAL')
		:semi_major_axis(f(100,100000))
		:rotation_period(f(1,24*60*3)),
		}
	

	s:bodies(HaberA,{
		HaberB,
		HaberC,
		HaberC_STARPORTS,
		HaberD,
		HaberE1,{
			HaberE2,
			HaberE3,
			HaberE4,
			HaberE4_STARPORTS,
			HaberE5,
			HaberE5_STARPORTS,
			HaberE6,
			},
		HaberF,
		HaberG,
		HaberH,
		HaberH_STARPORTS,
		})
			

s:add_to_sector(-5,0,-1,v(0.2,0.5,0.0))
