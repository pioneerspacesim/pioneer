-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Eagle MK-III',
	model = 'eagle_mk3',
	forward_thrust = 36e5,
	reverse_thrust = 25e5,
	up_thrust = 8e5,
	down_thrust = 8e5,
	left_thrust = 8e5,
	right_thrust = 8e5,
	angular_thrust = 64e5,
	cockpit_front = v(0,1,-12.8),
	cockpit_rear = v(0,2,-8),
	front_camera = v(0,.1,-18.2),
	rear_camera = v(0,1,11.5),
	left_camera = v(-16.3,0,-.2),
	right_camera = v(16.3,0,-.2),
	top_camera = v(0,2.5,3),
	bottom_camera = v(0,-2.5,3),
	gun_mounts =
	{
		{ v(0,-.7,-40), v(0,0,-1), 0, 0 },
		{ v(0,-.7,25), v(0,0,1), 0, 0 },
	},
	max_cargo = 22,
	max_missile = 2,
	max_fuelscoop = 0,
	max_cargoscoop = 0,
	capacity = 22,
	hull_mass = 10,
	fuel_tank_mass = 5,
	thruster_fuel_use = 0.00015,
	price = 43000,
	hyperdrive_class = 1,
}
