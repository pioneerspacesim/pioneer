-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Boa Freighter',
	model='boa',
	forward_thrust = 12e7,
	reverse_thrust = 4e7,
	up_thrust = 4e7,
	down_thrust = 2e7,
	left_thrust = 2e7,
	right_thrust = 2e7,
	angular_thrust = 50e7,
	camera_offset = v(0,6,-4),
	gun_mounts =
	{
		{ v(0,-2,-46), v(0,0,-1), 1, 'HORIZONTAL' },
		{ v(0,3,48), v(0,0,1), 6, 'VERTICAL' },
	},
	max_cargo = 600,
	max_laser = 2,
	max_missile = 6,
	max_cargoscoop = 0,
	capacity = 600,
	hull_mass = 300,
	fuel_tank_mass = 280,
	thruster_fuel_use = 0.00025,
	price = 2474000,
	hyperdrive_class = 7,
}
