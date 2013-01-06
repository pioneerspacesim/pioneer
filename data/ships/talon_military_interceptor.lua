-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Talon Military Interceptor',
	model='fi',
	forward_thrust = 34e5,
	reverse_thrust = 850000,
	up_thrust = 680000,
	down_thrust = 510000,
	left_thrust = 510000,
	right_thrust = 510000,
	angular_thrust = 20e5,
	camera_offset = v(0,.5,-8),
	gun_mounts =
	{
		{ v(0,-2,-46), v(0,0,-1), 5, 'HORIZONTAL' },
		{ v(0,0,0), v(0,0,1), 5, 'HORIZONTAL' },
	},
	max_cargo = 9,
	max_laser = 1,
	max_missile = 6,
	max_fuelscoop = 0,
	max_cargoscoop = 0,
	capacity = 9,
	hull_mass = 5,
	fuel_tank_mass = 12,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 75647e3,
	price = 33000,
	hyperdrive_class = 1,
}
