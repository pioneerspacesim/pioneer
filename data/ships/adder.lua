-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Adder',
	model='adder',
	forward_thrust = 168e5,
	reverse_thrust = 86e5,
	up_thrust = 50e5,
	down_thrust = 34e5,
	left_thrust = 34e5,
	right_thrust = 34e5,
	angular_thrust = 286e5,
	camera_offset = v(0,4,-22),
	gun_mounts =
	{
		{ v(0,0,-26), v(0,0,-1), 5, 'HORIZONTAL' },
		{ v(0,-2,9), v(0,0,1), 5, 'HORIZONTAL' },
	},
	max_cargo = 50,
	max_laser = 1,
	max_missile = 2,
	max_cargoscoop = 1,
	max_fuelscoop = 1,
	capacity = 50,
	hull_mass = 30,
	fuel_tank_mass = 37,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 60556e3,
	price = 60000,
	hyperdrive_class = 2,
}
