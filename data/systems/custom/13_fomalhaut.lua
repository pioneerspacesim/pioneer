-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local system = CustomSystem:new('Fomalhaut', { 'STAR_A', 'BROWN_DWARF',  })
	:faction('Solar Federation')
	:govtype('EARTHCOLONIAL')
	:lawlessness(f(5,100)) -- 1/100th from a peaceful eden
	:short_desc('Federal military outpost')
	:long_desc([[To the average citizen in the Federation, war either belongs to ancient times, or to the barbaric systems on the rim of civilization. Five centuries have passed since the last major conflict, and the war between the Federation and the Commonwealth has since remained cold, even during the climax of the Pell incident in 3012.
The SolFed Military, however, considers war as a serious possibility. Its fleets are frequently deployed for peacekeeping operations in independent systems waging civil wars. And the hypothesis of an open conflict with another major power -be it CIW, Haber, or a younger faction- is not ruled out.
Located on the border between the SolFed and CIW, Fomalhaut is an important military outpost. Form here, the staff headquarters could quickly dispatch the 5th, 6th and 7th fleet, either to reinforce a Federal system under attack, or to strike deeply in enemy territory.
In order to hamper foreign intelligence activity, the Federation strongly restricts and monitors any civilian activity.
Only two civilian stations have yet been authorized in the system, as they offer recreation for the troops on leave. Both are being operated by corporations: the "Red Cloud" aims to attract rank-and-file soldiers, while the "Ambrosia Chateau" appeals to officers. But no matter low cost or premium, the services are unsurprising: gambling, red light districts, restaurants and performances.
These stations -being open to travellers- are the only structures appearing on public maps. But they suffer from a bad reputation: many visitors comment about how troops are being ripped off of their pay with overpriced and tacky entertainment. Moreover, despite the faux mood of freedom and even wild abandon, some visitors cannot shake off a feeling of being constantly watched.]])

--Names do not infringe any copyright
--Bodies' names are generic, as the federal bureaucracy did not care to name them
--Stations' names are made up. "Red Cloud" happens to be a Sioux chef (1822-1909). An internet search doesn't retun any touristic place by the name of "Ambrosia Chateau" (sadly, "Ambrosia Resorts" has already been claimed by a hotel in Pune, India)

local fomalhaut = CustomSystemBody:new("Fomalhaut", 'STAR_A')
	:radius(f(21800,10000))
	:mass(f(29100,10000))
	:seed(1917676571)
	:temp(8481)

local fomalhauta = CustomSystemBody:new("Fomalhaut a", 'BROWN_DWARF')
	:radius(f(1500,10000))
	:mass(f(159,10000))
	:seed(3896260983)
	:temp(3861)
	:semi_major_axis(f(620,10000))
	:eccentricity(f(3080,10000))
	:rotation_period(f(33038,10000))

-- TODO: Fomalhaut B exploded some time before 2000AD, is no longer visible.
-- Possible solution: asteroid cluster or gas cloud for mining?
local fomalhautb = CustomSystemBody:new("Fomalhaut b", 'PLANET_GAS_GIANT')
	:radius(f(114243,10000))
	:mass(f(1305142,10000))
	:seed(2436307742)
	:temp(258)
	:semi_major_axis(f(47333,10000))
	:eccentricity(f(174,10000))
	:rotation_period(f(55000,10000))
	:axial_tilt(fixed.deg2rad(f(2264,10000)))
	:rotational_phase_at_start(fixed.deg2rad(f(0,10000)))
	:orbital_phase_at_start(fixed.deg2rad(f(0,10000)))
	:orbital_offset(fixed.deg2rad(f(0,10000)))

local fomalhautc = CustomSystemBody:new("Fomalhaut c", 'PLANET_GAS_GIANT')
	:radius(f(132844,10000))
	:mass(f(4023227,10000))
	:seed(58130093)
	:temp(192)
	:semi_major_axis(f(84256,10000))
	:eccentricity(f(2173,10000))
	:rotation_period(f(1250,10000))
	:axial_tilt(fixed.deg2rad(f(2054,10000)))

local fomalhautc1 = CustomSystemBody:new("Fomalhaut c 1", 'PLANET_TERRESTRIAL')
	:radius(f(1363,10000))
	:mass(f(25,10000))
	:seed(3746299662)
	:temp(123)
	:semi_major_axis(f(34,10000))
	:eccentricity(f(304,10000))
	:rotation_period(f(21182,10000))
	:axial_tilt(fixed.deg2rad(f(0,10000)))
	:metallicity(f(32,10000))
	:volcanicity(f(1,10000))
	:atmos_density(f(0,10000))
	:atmos_oxidizing(f(918,10000))
	:ocean_cover(f(0,10000))
	:ice_cover(f(0,10000))
	:life(f(0,10000))

local fomalhautc2 = CustomSystemBody:new("Fomalhaut c 2", 'PLANET_TERRESTRIAL')
	:radius(f(5216,10000))
	:mass(f(1419,10000))
	:seed(478589399)
	:temp(123)
	:semi_major_axis(f(56,10000))
	:eccentricity(f(157,10000))
	:rotation_period(f(43544,10000))
	:axial_tilt(fixed.deg2rad(f(0,10000)))
	:metallicity(f(2968,10000))
	:volcanicity(f(411,10000))
	:atmos_density(f(0,10000))
	:atmos_oxidizing(f(087,10000))
	:ocean_cover(f(0,10000))
	:ice_cover(f(0,10000))
	:life(f(0,10000))

local fomalhautc3 = CustomSystemBody:new("Fomalhaut c 3", 'PLANET_TERRESTRIAL')
	:radius(f(1983,10000))
	:mass(f(78,10000))
	:seed(308588721)
	:temp(123)
	:semi_major_axis(f(714,10000))
	:eccentricity(f(714,10000))
	:rotation_period(f(229951,10000))
	:axial_tilt(fixed.deg2rad(f(683,10000)))
	:rotational_phase_at_start(fixed.deg2rad(f(0,10000)))
	:orbital_phase_at_start(fixed.deg2rad(f(0,10000)))
	:orbital_offset(fixed.deg2rad(f(0,10000)))
	:metallicity(f(836,10000))
	:volcanicity(f(42,10000))
	:atmos_density(f(0,10000))
	:atmos_oxidizing(f(7193,10000))
	:ocean_cover(f(0,10000))
	:ice_cover(f(0,10000))
	:life(f(0,10000))

local red_cloud = CustomSystemBody:new("Red Cloud", 'STARPORT_ORBITAL')
	:seed(530867019)
	:temp(123)
	:semi_major_axis(f(55,1000000))

local ambrosia_chateau = CustomSystemBody:new("Ambrosia Chateau", 'STARPORT_ORBITAL')
	:mass(f(0,10000))
	:seed(510441276)
	:temp(123)
	:semi_major_axis(f(71,1000000))
	


system:bodies(fomalhaut, 
	{
	fomalhauta,
	fomalhautb,
	fomalhautc,
		{
		fomalhautc1,
		fomalhautc2,
		fomalhautc3,
			{
			red_cloud,
			ambrosia_chateau,
			}
		} 
	})

system:add_to_sector(-1,-3,-2,v(0.2680,0.3760,0.4510))
