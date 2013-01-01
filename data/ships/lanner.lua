-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Lanner',
	model='lanner_ub',
	forward_thrust = 30e6,
	reverse_thrust = 10e6,
	up_thrust = 10e6,
	down_thrust = 5e6,
	left_thrust = 5e6,
	right_thrust = 5e6,
	angular_thrust = 90e6,
	camera_offset = v(0,3,-28.5),
	gun_mounts =
	{
		{ v(0,-1.9,-38), v(0,0,-1), 5, 'HORIZONTAL' },
		{ v(0,1,38), v(0,0,1), 5, 'HORIZONTAL' },
	},
	max_cargo = 190,
	max_laser = 2,
	max_missile = 4,
	max_cargoscoop = 0,
	capacity = 190,
	hull_mass = 130,
	fuel_tank_mass = 60,
	thruster_fuel_use = 0.00025,
	price = 280000,
	hyperdrive_class = 3,
}
