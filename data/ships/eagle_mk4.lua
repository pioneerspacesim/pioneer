-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Eagle MK-IV "Bomber"',
	model='eagle_mk4',
	forward_thrust = 98e5,
	reverse_thrust = 49e5,
	up_thrust = 27e5,
	down_thrust = 24e5,
	left_thrust = 24e5,
	right_thrust = 24e5,
	angular_thrust = 90e5,
	cockpit_front = v(0,1,-12.8),
	cockpit_rear = v(0,2,-8),
	front_camera = v(0,.1,-18.2),
	rear_camera = v(0,1,11.5),
	left_camera = v(-18,0,-.2),
	right_camera = v(18,0,-.2),
	top_camera = v(0,2.5,3),
	bottom_camera = v(0,-2.5,3),
	gun_mounts =
	{
		{ v(0,-.7,-40), v(0,0,-1) },
		{ v(0,-.7,25), v(0,0,1) },
	},
	max_cargo = 34,
	max_laser = 2,
	max_missile = 6,
	max_fuelscoop = 1,
	max_cargoscoop = 1,
	capacity = 34,
	hull_mass = 15,
	fuel_tank_mass = 24,
	effective_exhaust_velocity = 63929e3,
	price = 56000,
	hyperdrive_class = 2,
}
