-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='E.Y.E',
	model='peye',
	forward_thrust = 20e6,
	reverse_thrust = 10e6,
	up_thrust = 5e6,
	down_thrust = 4e6,
	left_thrust = 4e6,
	right_thrust = 4e6,
	angular_thrust = 20e6,
	camera_offset = v(0,.2,-10.2),
	gun_mounts =
	{
		{ v(0,-5.8,-11), v(0,0,-1), 5, 'HORIZONTAL' },
		{ v(0,0,10), v(0,0,1), 5, 'HORIZONTAL' },
	},
	max_atmoshield = 0,
	max_cargo = 80,
	max_laser = 2,
	max_missile = 0,
	max_cargoscoop = 0,
	capacity = 80,
	hull_mass = 25,
	fuel_tank_mass = 55,
	thruster_fuel_use = 0.00025,
	price = 100000,
	hyperdrive_class = 3,
}
