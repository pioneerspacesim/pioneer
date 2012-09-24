-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Sidewinder',
	model='rattlesnake',
	forward_thrust = 4e6,
	reverse_thrust = 3e6,
	up_thrust = 2e6,
	down_thrust = 2e6,
	left_thrust = 2e6,
	right_thrust = 2e6,
	angular_thrust = 10e6,
	cockpit_front = v(0,2.5,0),
	cockpit_rear = v(0,3.5,2),
	front_camera = v(0,0,-13),
	rear_camera = v(0,-3,10),
	left_camera = v(-20,0,9.5),
	right_camera = v(20,0,9.5),
	top_camera = v(0,4,6),
	bottom_camera = v(0,-4,6),
	gun_mounts =
	{
		{ v(0,0,-16), v(0,0,-1) },
		{ v(0,0,15), v(0,0,1) },
	},
	max_cargo = 30,
	max_laser = 2,
	max_missile = 0,
	max_fuelscoop = 1,
	max_cargoscoop = 1,
	capacity = 30,
	hull_mass = 20,
	fuel_tank_mass = 10,
	thruster_fuel_use = 0.00035,
	price = 44000,
	hyperdrive_class = 2,
}
