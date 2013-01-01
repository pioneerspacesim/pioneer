-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name = 'Imperial Trader',
	model = 'trader',
	forward_thrust = 8e7,
	reverse_thrust = 3e7,
	up_thrust = 3e7,
	down_thrust = 1e7,
	left_thrust = 1e7,
	right_thrust = 1e7,
	angular_thrust = 15e7,
	camera_offset = v(0,3.5,-29),
	gun_mounts =
	{
		{ v(0,0.6,-36), v(0,0,-1), 5, 'HORIZONTAL' },
		{ v(0,0,22), v(0,0,1), 5, 'HORIZONTAL' },
	},
	max_cargo = 450,
	max_laser = 1,
	max_missile = 6,
	max_cargoscoop = 0,
	capacity = 450,
	hull_mass = 300,
	fuel_tank_mass = 150,
	thruster_fuel_use = 0.0002,
	price = 954000,
	hyperdrive_class = 5,
}
