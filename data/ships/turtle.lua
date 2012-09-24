-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Turtle',
	model='turtle',
	forward_thrust = 10e6,
	reverse_thrust = 10e6,
	up_thrust = 10e6,
	down_thrust = 10e6,
	left_thrust = 10e6,
	right_thrust = 10e6,
	angular_thrust = 30e6,
	cockpit_front = v(0,-2,-16),
	cockpit_rear = v(0,6,-14),
	front_camera = v(0,-2,-16),
	rear_camera = v(0,2,16),
	left_camera = v(-16,2,0),
	right_camera = v(16,2,0),
	top_camera = v(0,7.5,-6),
	bottom_camera = v(0,-7.5,-6),
	gun_mounts =
	{
		{ v(0,-4,-10.2), v(0,0,-1) },
		{ v(0,-0.5,0), v(0,0,1) },
	},
	max_cargo = 90,
	max_laser = 2,
	max_missile = 4,
	max_cargoscoop = 0,
	capacity = 90,
	hull_mass = 50,
	fuel_tank_mass = 5,
	thruster_fuel_use = 0.0004,
	price = 250000,
	hyperdrive_class = 3,
}
