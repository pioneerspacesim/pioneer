local s = CustomSystem:new('Tolan Kingdom', { 'STAR_A','STAR_F' }) 
	:govtype('PLUTOCRATIC') 
	:seed(190)
	:short_desc('Tolan Kingdom.  Terraformed world with introduced life.  Isolationist')
	:long_desc([[Little is known about the history of the Tolan Kingdom System. Colonised in 2305 by the Generation ship "Tolan", Oberon Tolan and his descendents have ruled for over six centuries.  
	The Royal Union of Miners is the most efficient mining company in the galaxy and have mined the Tolan System dry.  Despite rumours of illegal mining outside the Tolan Kingdom, the official line continues to be of 'Splendid Isolation'.
	They have a strong police force and most activities are illegal.  Traders from outside are welcome, but only at the designated starports.
	Entering the cities or landing anywhere but the starports without the correct documentation carries the death sentence. ]])



local TolanA = CustomSBody:new('Tolan', 'STAR_A')
	:radius(f(50,10))
	:mass(f(610,1))
	:temp(4584)


local TolanB = CustomSBody:new('Tolan B', 'STAR_F')
	:radius(f(42,10))
	:mass(f(401,1))
	:temp(4300)
	:semi_major_axis(f(95,1000))
	:eccentricity(f(1,5))
	:rotation_period(f(0.2,1))


local Tolan1 = CustomSBody:new('Claim', 'PLANET_TERRESTRIAL')
	:seed(8)
	:radius(f(245,1000))
	:mass(f(20,100))
	:temp(400)
	:semi_major_axis(f(793,1000))
	:eccentricity(f(1,5000000))
	:inclination(math.deg2rad(3.09))
	:rotation_period(f(5,10))
	:axial_tilt(math.fixed.deg2rad(f(26,10)))
	:metallicity(f(1,6))
	:volcanicity(f(6,10))
	:atmos_density(f(1,9))
	:atmos_oxidizing(f(1,10))
	:ocean_cover(f(1,10))
	:ice_cover(f(0,100))
	:life(f(2,35))
	
	local Tolan1ORBITALS ={
		CustomSBody:new('Claim Orbital 1', 'STARPORT_ORBITAL')
		:semi_major_axis(f(100,100000))
		:rotation_period(f(1,24*60*3)),
		CustomSBody:new('Claim Orbital 2', 'STARPORT_ORBITAL')
		:semi_major_axis(f(100,100000))
		:rotation_period(f(1,24*60*3)),
		}
		

local Tolan2 = CustomSBody:new('Home', 'PLANET_TERRESTRIAL')
	:seed(43)
	:radius(f(15,10))
	:mass(f(45,40))
	:temp(293)
	:semi_major_axis(f(95,10))
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
	
local Tolan2_starports = {
	CustomSBody:new('Titania', 'STARPORT_SURFACE')
		:latitude(math.deg2rad(31))
		:longitude(math.deg2rad(-121)),
	CustomSBody:new('Oberon City', 'STARPORT_SURFACE')
		:latitude(math.deg2rad(19))
		:longitude(math.deg2rad(99)),
	CustomSBody:new('Brithish', 'STARPORT_SURFACE')
		:latitude(math.deg2rad(51))
		:longitude(0),
	CustomSBody:new('Trade Post', 'STARPORT_ORBITAL')
		:semi_major_axis(f(100,100000))
		:rotation_period(f(1,24*60*3)),
}
	
	
	local Tolan2a = CustomSBody:new('Tolans Moon', 'PLANET_TERRESTRIAL')
		:seed(191082)
		:radius(f(324,1000))
		:mass(f(121,1000))
		:temp(187)
		:semi_major_axis(f(16,100000))
		:eccentricity(f(1,10))
		:inclination(math.deg2rad(5.145))
		:rotation_period(f(6,10))
		:axial_tilt(math.fixed.deg2rad(f(668,100)))
		:volcanicity(f(9,10))
		:atmos_density(f(1,10))
		
	local Tolan2aSURFACE1 = CustomSBody:new('Palatial', 'STARPORT_SURFACE')
			:latitude(math.deg2rad(31))
			:longitude(math.deg2rad(-121))
			
			
local Tolan3 = CustomSBody:new('Avalon', 'PLANET_TERRESTRIAL')
	:seed(43)
	:radius(f(15,10))
	:mass(f(45,40))
	:temp(280)
	:semi_major_axis(f(1250,100))
	:eccentricity(f(5,100))
	:rotation_period(f(87,100))
	:axial_tilt(math.fixed.deg2rad(f(75,100)))
	:metallicity(f(7,6))
	:volcanicity(f(40,100))
	:atmos_density(f(10,10))
	:atmos_oxidizing(f(7,10))
	:ocean_cover(f(10,100))
	:ice_cover(f(80,100))
	:life(f(10,51))		
			
local Tolan4 = CustomSBody:new('Kent', 'PLANET_GAS_GIANT')
	:radius(f(14,1))
	:mass(f(811,1))
	:temp(174)
	:semi_major_axis(f(2682,100))
	:eccentricity(f(15,100))
	:inclination(math.deg2rad(1.305))
	:rotation_period(f(4,10))
	:axial_tilt(math.fixed.deg2rad(f(313,100)))
	
	local Tolan4a = CustomSBody:new('Kent A', 'PLANET_TERRESTRIAL')
		:seed(12)
		:radius(f(317,1000))
		:mass(f(117,1000))
		:temp(135)
		:semi_major_axis(f(1,100))
		:eccentricity(f(92,100))
		:inclination(math.deg2rad(5.145))
		:rotation_period(f(9,10))
		:axial_tilt(math.fixed.deg2rad(f(100,100)))
		:volcanicity(f(1,10))
		:atmos_density(f(1,15))
		:ocean_cover(f(0,10))
		:ice_cover(f(1,1500))
		:atmos_oxidizing(f(20,1000))
	
local Tolan4b = CustomSBody:new('Kent B', 'PLANET_TERRESTRIAL')
		:seed(12)
		:radius(f(317,1000))
		:mass(f(117,1000))
		:temp(135)
		:semi_major_axis(f(2,100))
		:eccentricity(f(1,1000))
		:inclination(math.deg2rad(5.145))
		:rotation_period(f(9,10))
		:axial_tilt(math.fixed.deg2rad(f(100,100)))
		:volcanicity(f(1,10))
		:atmos_density(f(10,15))
		:ocean_cover(f(0,10))
		:ice_cover(f(1,1))
		:atmos_oxidizing(f(50,10))
		
	local Tolan4c = CustomSBody:new('Kent C', 'PLANET_ASTEROID')
		:seed(12)
		:radius(f(31,1000))
		:mass(f(11,1000))
		:temp(115)
		:semi_major_axis(f(4,100))
		:eccentricity(f(2,1000))
		:inclination(math.deg2rad(5.145))
		:rotation_period(f(9,10))
		:axial_tilt(math.fixed.deg2rad(f(100,100)))
		:volcanicity(f(1,10))
		:atmos_density(f(4,15))
		:ocean_cover(f(0,10))
		:ice_cover(f(1,1500))
		
	local Tolan4d = CustomSBody:new('Kent D', 'PLANET_ASTEROID')
		:seed(12)
		:radius(f(17,1000))
		:mass(f(17,1000))
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

local Tolan5 = CustomSBody:new('Anglia', 'PLANET_GAS_GIANT')
	:radius(f(14,1))
	:mass(f(3000,10))
	:temp(-120)
	:semi_major_axis(f(4802,100))
	:eccentricity(f(48,10000))
	:inclination(math.deg2rad(1.305))
	:rotation_period(f(4,10))
	:axial_tilt(math.fixed.deg2rad(f(313,100)))
	
	local Tolan5a = CustomSBody:new('Scapa Flow', 'PLANET_TERRESTRIAL')
		:seed(1002)
		:radius(f(27,1000))
		:mass(f(30,1000))
		:temp(270)
		:semi_major_axis(f(4,100))
		:eccentricity(f(1,100000))
		:inclination(math.deg2rad(2.145))
		:rotation_period(f(1,10))
		:axial_tilt(math.fixed.deg2rad(f(100,100)))
		:volcanicity(f(5,10))
		:atmos_density(f(4,15))
		:ocean_cover(f(1,100))
		:ice_cover(f(1,1500))
		:atmos_oxidizing(f(10,10))
	
	local Tolan5aSTARPORTS = {
		CustomSBody:new('Naval Central', 'STARPORT_SURFACE')
			:latitude(math.deg2rad(1))
			:longitude(math.deg2rad(1)),
		CustomSBody:new('Depot', 'STARPORT_SURFACE')
			:latitude(math.deg2rad(25))
			:longitude(math.deg2rad(16)),
		}
	 
	local Tolan6 = CustomSBody:new('Wight', 'PLANET_TERRESTRIAL')
	:radius(f(1,100))
	:mass(f(1,10))
	:temp(50)
	:semi_major_axis(f(7602,100))
	:eccentricity(f(488,10000))
	:inclination(math.deg2rad(1.305))
	:rotation_period(f(4,10))
	:axial_tilt(math.fixed.deg2rad(f(313,100)))
	
	local Tolan7 = CustomSBody:new('Guernsey', 'PLANET_TERRESTRIAL')
	:radius(f(5,100))
	:mass(f(1,10))
	:temp(20)
	:semi_major_axis(f(12002,100))
	:eccentricity(f(488,10000))
	:inclination(math.deg2rad(5))
	:rotation_period(f(4,10))
	:axial_tilt(math.fixed.deg2rad(f(313,100)))
	
	local Tolan8 = CustomSBody:new('Miners Haven', 'PLANET_TERRESTRIAL')
	:radius(f(25,100))
	:mass(f(2,10))
	:temp(80)
	:semi_major_axis(f(16002,100))
	:eccentricity(f(4,10000))
	:inclination(math.deg2rad(1.305))
	:rotation_period(f(4,10))
	:axial_tilt(math.fixed.deg2rad(f(313,100)))
	:metallicity(f(26,6))
	:volcanicity(f(0,100))
	:atmos_density(f(0,10))
	:atmos_oxidizing(f(2,20))
	:ocean_cover(f(0,100))
	:ice_cover(f(50,100))
	:life(f(6,501))	
	
	local Tolan8_starports = {
	CustomSBody:new('Ord Central', 'STARPORT_SURFACE')
		:latitude(math.deg2rad(31))
		:longitude(math.deg2rad(-121)),
	CustomSBody:new('Final Base', 'STARPORT_ORBITAL')
		:semi_major_axis(f(100,100000))
		:rotation_period(f(1,24*60*3)),
	}

	s:bodies(TolanA, {
	TolanB,	
	Tolan1,
		Tolan1ORBITALS,
	Tolan2, 
		Tolan2_starports,
		{
		Tolan2a, {
			Tolan2aSURFACE1,
			},
			},
	Tolan3,
	Tolan4, {
		Tolan4a,
		Tolan4b,
		Tolan4c,
		Tolan4d,
		Tolan4e,
		},
	Tolan5, {
		Tolan5a,
			Tolan5aSTARPORTS,
			},
	Tolan6,
	Tolan7,
	Tolan8,
		Tolan8_starports,
	})

s:add_to_sector(1,-4,-1,v(0.037,0.5,0.6))
