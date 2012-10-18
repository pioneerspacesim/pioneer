-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Talon Military Interceptor',
	model='fi',
	forward_thrust = 34e5,
	reverse_thrust = 850000,
	up_thrust = 680000,
	down_thrust = 510000,
	left_thrust = 510000,
	right_thrust = 510000,
	angular_thrust = 15e5,
	cockpit_front = v(0,.5,-8),
	cockpit_rear = v(0,1,-3),
	front_camera = v(0,-.3,-10.5),
	rear_camera = v(0,.5,10),
	left_camera = v(-9.5,-2.9,6.2),
	right_camera = v(9.5,-2.9,6.2),
	top_camera = v(0,1,4),
	bottom_camera = v(0,-3,4),
	gun_mounts =
	{
		{ v(0,-2,-46), v(0,0,-1) },
		{ v(0,0,0), v(0,0,1) },
	},
	max_cargo = 9,
	max_laser = 1,
	max_missile = 6,
	max_fuelscoop = 0,
	max_cargoscoop = 0,
	capacity = 9,
	hull_mass = 6,
	fuel_tank_mass = 11,
	effective_exhaust_velocity = 80000e3,
	price = 33000,
	hyperdrive_class = 1,
}
