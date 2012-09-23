-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Viper Defence Craft',
	model='viper',
	forward_thrust = -10e6,
	reverse_thrust = 4e6,
	up_thrust = 4e6,
	down_thrust = -3e6,
	left_thrust = -3e6,
	right_thrust = 3e6,
	angular_thrust = 30e6,
	cockpit_front = v(0,4.5,-12.5),
	cockpit_rear = v(0,6,-10),
	front_camera = v(0,1.6,-19.7),
	rear_camera = v(0,4.5,19),
	left_camera = v(-15,2.2,17),
	right_camera = v(15,2.2,17),
	top_camera = v(0,8,3),
	bottom_camera = v(0,-3,3),
	gun_mounts =
	{
		{ v(0,-2,-46), v(0,0,-1) },
		{ v(0,0,0), v(0,0,1) },
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
