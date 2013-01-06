-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name = 'Constrictor',
	model='conny',
	forward_thrust = 343e5,
	reverse_thrust = 156e5,
	up_thrust = 79e5,
	down_thrust = 79e5,
	left_thrust = 79e5,
	right_thrust = 79e5,
	angular_thrust = 1170e5,
	camera_offset = v(0,3.4,-15),
	gun_mounts =
	{
		{ v(0,-2,-26), v(0,0,-1), 5, 'HORIZONTAL' },
		{ v(0,-2,19), v(0,0,1), 5, 'HORIZONTAL' }
	},
	max_cargo = 90,
	max_laser = 2,
	max_missile = 2,
	max_fuelscoop = 1,
	max_cargoscoop = 1,
	capacity = 90,
	hull_mass = 60,
	fuel_tank_mass = 84,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 57778e3,
	price = 143000,
	hyperdrive_class = 3,
}
