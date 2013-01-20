-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Asp Explorer',
	model='asp_sparks',
	forward_thrust = 280e5,
	reverse_thrust = 127e5,
	up_thrust = 76e5,
	down_thrust = 76e5,
	left_thrust = 76e5,
	right_thrust = 76e5,
	angular_thrust = 780e5,
	camera_offset = v(0,5,.5),
	gun_mounts =
	{
		{ v(0,2.57,-21.35), v(0,0,-1), 5, 'HORIZONTAL' },
		{ v(0,-4.42,26.04), v(0,0,1), 5, 'HORIZONTAL' },
	},
	max_cargo = 120,
	max_missile = 1,
	max_laser = 2,
	max_cargoscoop = 0,
	max_fuelscoop = 1,
	capacity = 120,
	hull_mass = 60,
	fuel_tank_mass = 106,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 57273e3,
	price = 187000,
	hyperdrive_class = 3,
}
