-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Cobra Mk III',
	model='cobra_mk3',
	forward_thrust = 16e6,
	reverse_thrust = 8e6,
	up_thrust = 4e6,
	down_thrust = 4e6,
	left_thrust = 4e6,
	right_thrust = 4e6,
	angular_thrust = 22e6,
	cockpit_front = v(0,2.2,-7),
	cockpit_rear = v(0,5.5,0),
	front_camera = v(0,0,-11.5),
	rear_camera = v(0,0,15),
	left_camera = v(-25.5,-1,10),
	right_camera = v(25.5,-1,10),
	top_camera = v(0,5.5,5),
	bottom_camera = v(0,-5,6),
	gun_mounts =
	{
		{ v(0,-0.5,0), v(0,0,-1), 0, 0 },
		{ v(0,-0.5,0), v(0,0,1), 0, 0 },
	},
	max_cargo = 80,
	max_laser = 2,
	max_missile = 4,
	max_cargoscoop = 0,
	capacity = 80,
	hull_mass = 40,
	fuel_tank_mass = 20,
	thruster_fuel_use = 0.0002,
	price = 124000,
	hyperdrive_class = 3,
}
