-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local system = CustomSystem:new('Wolf 359', {'STAR_M'}):other_names({"CN Leonis", "CN Leo"})
	:govtype('NONE')
	:short_desc('Deep space survey system of legend')
	:long_desc(
		[[The system is best known for the old 'deep space' research and signal station Hephaestus, that is still orbiting the red dwarf of Wolf 359. It harkens back to when humanity took its first trying steps out from its birth system of Sol.
As more information of its operation emerges, Hephaestus is becoming an epicentre of many legends and myths. It is rumoured that the then comparative seclusion of the system made it ideal for prying human minds with nefarious intentions. Officially, many of the rumours are attributed as being artefacts of the deteriorating mental state of the minimal skeletal crew that manned the station, which are chronicled in their audio logs, some of which have survived to this day. One of the controversies is whether the supposedly alien signal intercepted from Gliese 163 was merely an hallucinogenic effect from listening to too much static.
The low mass and temperature of Wolf 359 puts it right on the lower limit of being able to maintain hydrogen fusion. Coincidentally, it bears many features in common with Gliese 163. ]])

-- Metallicity [Fe/H]	+0.18 ± 0.17 dex
-- Rotational velocity (v sin i)	< 3.0 km/s

local star = CustomSystemBody:new('Wolf 359', 'STAR_M')
	:radius(f(16,100))			-- 0.16 R_sol
	:mass(f(1,11))				-- 0.09 M_sol
	:temp(2800)					-- +/- 100 K

-- Wikipedia:
-- Habitable zone inner limit	0.024 AU
-- Habitable zone outer limit   0.052 AU
local station = {
	CustomSystemBody:new('Hephaestus', 'STARPORT_ORBITAL')
		:semi_major_axis(f(24,1000))
--		:rotation_period(f(1,))
		:orbital_offset(f(1,3))
}

system:bodies(star, station)
system:add_to_sector(0,0,0,v(0.264,0.931,0.120))
