-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Meteor',
	model='meteor',
	forward_thrust = 28e5,
	reverse_thrust = 2e5,
	up_thrust = 5e5,
	down_thrust = 5e5,
	left_thrust = 5e5,
	right_thrust = 5e5,
	angular_thrust = 10e5,
	camera_offset = v(0,1.3,-2.3),
	gun_mounts =
	{
		{ v(0,0,-25), v(0,0,-1), 5, 'HORIZONTAL' },
	},
	max_cargo = 5,
	max_missile = 4,
	max_cargoscoop = 0,
	max_ecm = 0,
	hyperdrive_class = 1,
	capacity = 12,
	hull_mass = 5,
	fuel_tank_mass = 7,
	thruster_fuel_use = 0.0005,
	price = 18000,
}
