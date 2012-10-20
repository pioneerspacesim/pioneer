-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name = 'Imperial Trader',
	model = 'trader',
	forward_thrust = 542e5,
	reverse_thrust = 203e5,
	up_thrust = 203e5,
	down_thrust = 68e5,
	left_thrust = 68e5,
	right_thrust = 68e5,
	angular_thrust = 1950e5,
	gun_mounts =
	{
		{ v(0,0.6,-36), v(0,0,-1) },
		{ v(0,0,22), v(0,0,1) },
	},
	max_cargo = 600,
	max_laser = 1,
	max_missile = 6,
	max_cargoscoop = 0,
	capacity = 600,
	hull_mass = 300,
	fuel_tank_mass = 270,
	effective_exhaust_velocity = 55556e3,
	price = 954000,
	hyperdrive_class = 5,
}
