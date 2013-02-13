-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Kanara Interceptor',
	model='kanara',
	forward_thrust = 65e5,
	reverse_thrust = 12e5,
	up_thrust = 18e5,
	down_thrust = 18e5,
	left_thrust = 18e5,
	right_thrust = 18e5,
	angular_thrust = 25e5,
--	camera_offset = v(0,4.5,-12.5),
	gun_mounts =
	{
		{ v(0,-2,-46), v(0,0,-1), 5, 'HORIZONTAL' },
		{ v(0,0,0), v(0,0,1), 5, 'HORIZONTAL' },
	},
	max_cargo = 15,
	max_laser = 1,
	max_missile = 4,
	max_cargoscoop = 0,
	max_engine = 0,
	min_crew = 1,
	max_crew = 2,
	capacity = 15,
	hull_mass = 10,
	fuel_tank_mass = 15,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 59167e3,
	price = 70000,
	hyperdrive_class = 0,
}

