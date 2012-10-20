-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Cobra Mk I',
	model='cobra1',
	forward_thrust = 226e5,
	reverse_thrust = 94e5,
	up_thrust = 75e5,
	down_thrust = 75e5,
	left_thrust = 75e5,
	right_thrust = 75e5,
	angular_thrust = 364e5,
	cockpit_front = v(0,2.2,-8.2),
	cockpit_rear = v(0,5.5,0),
	front_camera = v(0,.5,-14),
	rear_camera = v(0,0,15),
	left_camera = v(-21.3,0,9),
	right_camera = v(21.3,0,9),
	top_camera = v(0,5.5,5),
	bottom_camera = v(0,-5,6),
	gun_mounts =
	{
		{ v(0,0,-13), v(0,0,-1) },
		{ v(0,0,10), v(0,0,1) }
	},
	max_cargo = 60,
	max_laser = 2,
	max_missile = 2,
	max_cargoscoop = 0,
	capacity = 60,
	hull_mass = 40,
	fuel_tank_mass = 56,
	effective_exhaust_velocity = 59167e3,
	price = 97000,
	hyperdrive_class = 2,
}
