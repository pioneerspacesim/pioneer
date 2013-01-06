-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Stardust',
	model='stardust',
	forward_thrust = 320e5,
	reverse_thrust = 128e5,
	up_thrust = 64e5,
	down_thrust = 64e5,
	left_thrust = 64e5,
	right_thrust = 64e5,
	angular_thrust = 416e5,
	camera_offset = v(0,0,-23),
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
	fuel_tank_mass = 125,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 57500e3,
	price = 150000,
	hyperdrive_class = 4,
}
