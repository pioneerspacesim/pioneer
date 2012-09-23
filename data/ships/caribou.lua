-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Caribou',
	model='caribou',
	forward_thrust = -10e7,
	reverse_thrust = 3e7,
	up_thrust = 3e7,
	down_thrust = -1e7,
	left_thrust = -1e7,
	right_thrust = 1e7,
	angular_thrust = 25e7,
	cockpit_front = v(0,4,-38),
	cockpit_rear = v(0,10,-25),
	front_camera = v(0,-7,-53),
	rear_camera = v(0,1,52),
	left_camera = v(-26,0,-.2),
	right_camera = v(26,0,-.2),
	top_camera = v(0,11,0),
	bottom_camera = v(0,-13,0),
	gun_mounts =
	{
		{ v(0,-0.5,-62), v(0,0,-1) },
		{ v(0,10,62), v(0,0,1) },
	},
	max_cargo = 740,
	max_laser = 2,
	max_missile = 20,
	max_cargoscoop = 0,
	capacity = 740,
	hull_mass = 460,
	fuel_tank_mass = 200,
	thruster_fuel_use = 0.0002,
	price = 2.1e6,
	hyperdrive_class = 7,
}
