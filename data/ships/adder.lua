-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Adder',
	model='adder',
	forward_thrust = 100e5,
	reverse_thrust = 51e5,
	up_thrust = 3e6,
	down_thrust = 2e6,
	left_thrust = 2e6,
	right_thrust = 2e6,
	angular_thrust = 22e6,
	cockpit_front = v(0,4,-22),
	cockpit_rear = v(0,5,-7),
	front_camera = v(0,.3,-33.5),
	rear_camera = v(-0.1,2,13),
	left_camera = v(-12,0,0),
	right_camera = v(12,0,0),
	top_camera = v(0,6,-8),
	bottom_camera = v(0,-5,4.5),
	gun_mounts =
	{
		{ v(0,0,-26), v(0,0,-1) },
		{ v(0,-2,9), v(0,0,1) },
	},
	max_cargo = 50,
	max_laser = 1,
	max_missile = 2,
	max_cargoscoop = 1,
	max_fuelscoop = 1,
	capacity = 50,
	hull_mass = 30,
	fuel_tank_mass = 10,
	thruster_fuel_use = 0.0003,
	price = 60000,
	hyperdrive_class = 2,
}
