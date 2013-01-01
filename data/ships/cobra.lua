-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Cobra Mk I',
	model='cobra1',
	forward_thrust = 12e6,
	reverse_thrust = 5e6,
	up_thrust = 4e6,
	down_thrust = 4e6,
	left_thrust = 4e6,
	right_thrust = 4e6,
	angular_thrust = 28e6,
	camera_offset = v(0,2.2,-8.2),
	gun_mounts =
	{
		{ v(0,0,-13), v(0,0,-1), 5, 'HORIZONTAL' },
		{ v(0,0,10), v(0,0,1), 5, 'HORIZONTAL' }
	},
	max_cargo = 60,
	max_laser = 2,
	max_missile = 2,
	max_cargoscoop = 0,
	capacity = 60,
	hull_mass = 40,
	fuel_tank_mass = 20,
	thruster_fuel_use = 0.0002,
	price = 97000,
	hyperdrive_class = 2,
}
