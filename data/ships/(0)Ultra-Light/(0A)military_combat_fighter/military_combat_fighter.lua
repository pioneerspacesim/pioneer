-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Military Combat Fighter',
	model = 'military_combat_fighter',
	forward_thrust = 63e5,
	reverse_thrust = 32e5,
	up_thrust = 0.1e8,
	down_thrust = 0.1e8,
	left_thrust = 0.1e8,
	right_thrust = 0.1e8,
	angular_thrust = 0.1e8,
	camera_offset = v(0,1,-12.8),
	gun_mounts =
	{
		{ v(0,-.7,-40), v(0,0,-1), 5, 'HORIZONTAL' },
		{ v(0,-.7,25), v(0,0,1), 5, 'HORIZONTAL' },
	},
	max_cargo = 18,
	max_missile = 2,
	max_fuelscoop = 0,
	max_cargoscoop = 0,
	min_crew = 1,
	max_crew = 1,
	capacity = 18,
	hull_mass = 10,
	fuel_tank_mass = 18,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 69286e3,
	price = 38000,
	hyperdrive_class = 1,
}
