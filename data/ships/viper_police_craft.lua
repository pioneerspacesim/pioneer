-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Viper Police Craft',
	model='viperpol',
	forward_thrust = 10e6,
	reverse_thrust = 4e6,
	up_thrust = 4e6,
	down_thrust = 3e6,
	left_thrust = 3e6,
	right_thrust = 3e6,
	angular_thrust = 30e6,
	camera_offset = v(0,4.5,-12.5),
	gun_mounts =
	{
		{ v(0,-2,-46), v(0,0,-1), 5, 'HORIZONTAL' },
		{ v(0,0,0), v(0,0,1), 5, 'HORIZONTAL' },
	},
	max_cargo = 60,
	max_laser = 1,
	max_missile = 4,
	max_cargoscoop = 0,
	capacity = 60,
	hull_mass = 40,
	fuel_tank_mass = 20,
	thruster_fuel_use = 0.0003,
	price = 70000,
	hyperdrive_class = 3,
}
