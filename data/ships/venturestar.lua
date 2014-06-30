-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Venturestar',
	ship_class='medium_freighter',
	manufacturer='ALBR',
	model='venturestar',
	forward_thrust = 35e6,
	reverse_thrust = 115e5,
	up_thrust = 15e6,
	down_thrust = 8e6,
	left_thrust = 8e6,
	right_thrust = 8e6,
	angular_thrust = 50e6,
	camera_offset = v(0,0,-18),
	gun_mounts =
	{
		{ v(0,-0.5,-10.7), v(0,0,-1), 5, 'HORIZONTAL' },
	},
	hull_mass = 200,
	fuel_tank_mass = 200,
	capacity = 500,
	slots = {
		cargo = 500,
		laser_front = 1,
		missile = 16,
		cargo_scoop = 1,
		fuel_scoop = 1,
		atmo_shield = 1,
	},
	min_crew = 3,
	max_crew = 5,

	effective_exhaust_velocity = 2548e4,
	price = 1346e3,
	hyperdrive_class = 4,
}
