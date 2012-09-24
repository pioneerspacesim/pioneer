-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Wave Heavy Hypersonic Fighter',
	model='wave',
	forward_thrust = 6e6,
	reverse_thrust = 2e6,
	up_thrust = 1e6,
	down_thrust = 1e6,
	left_thrust = 1e6,
	right_thrust = 1e6,
	angular_thrust = 30e6,
	cockpit_front = v(0,.6,-13),
	cockpit_rear = v(0,1,-10),
	front_camera = v(0,-.2,-15.5),
	rear_camera = v(0,.8,15.2),
	left_camera = v(-15.3,-3,14.5),
	right_camera = v(15.3,-3,14.5),
	top_camera = v(0,1,5),
	bottom_camera = v(0,-3.5,5),
	gun_mounts =
	{
		{ v(0,-0.5,-10.7), v(0,0,-1) },
		{ v(0,-0.5,0), v(0,0,1) },
	},
	max_cargo = 30,
	max_laser = 2,
	max_missile = 4,
	max_cargoscoop = 0,
	max_fuelscoop = 0,
	capacity = 30,
	hull_mass = 13,
	fuel_tank_mass = 7,
	thruster_fuel_use = 0.0002,
	price = 93000,
	hyperdrive_class = 2,
}
