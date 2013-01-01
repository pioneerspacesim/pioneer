-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Sirius Interdictor',
	model='interdictor',
	forward_thrust = 24e6,
	reverse_thrust = 12e6,
	up_thrust = 6e6,
	down_thrust = 6e6,
	left_thrust = 6e6,
	right_thrust = 6e6,
	angular_thrust = 120e6,
	camera_offset = v(0,4,-17.5),
	gun_mounts =
	{
		{ v(0,-0.5,0), v(0,0,-1), 5, 'HORIZONTAL' },
		{ v(0,-0.5,0), v(0,0,1), 5, 'HORIZONTAL' },
	},
	max_cargo = 90,
	max_laser = 2,
	max_missile = 8,
	max_cargoscoop = 0,
	capacity = 90,
	hull_mass = 66,
	fuel_tank_mass = 37,
	thruster_fuel_use = 0.0002,
	price = 160000,
	hyperdrive_class = 4,
}
