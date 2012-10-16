-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Stardust',
	model='stardust',
	forward_thrust = 200e5,
	reverse_thrust = 80e5,
	up_thrust = 40e5,
	down_thrust = 40e5,
	left_thrust = 40e5,
	right_thrust = 40e5,
	angular_thrust = 320e5,
	cockpit_front = v(0,0,-23),
	cockpit_rear = v(0,8,-15),
	front_camera = v(0,0,-23),
	rear_camera = v(0,0,9),
	left_camera = v(-8.5,0,0),
	right_camera = v(8.5,0,0),
	top_camera = v(0,9,-7),
	bottom_camera = v(0,-9,-7),
	gun_mounts =
	{
		{ v(0,0,-26), v(0,0,-1), 5, 'HORIZONTAL' },
		{ v(0,0,5), v(0,0,1), 5, 'HORIZONTAL' },
	},
	max_cargo = 100,
	max_laser = 2,
	max_fuelscoop = 0,
	max_cargoscoop = 0,
	capacity = 100,
	hull_mass = 35,
	fuel_tank_mass = 65,
	thruster_fuel_use = 0.0002,
	price = 150000,
	hyperdrive_class = 4,
}
