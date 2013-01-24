-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Flowerfairy Heavy Trader',
	model='flowerfairy',
	forward_thrust = 587e5,
	reverse_thrust = 196e5,
	up_thrust = 196e5,
	down_thrust = 141e5,
	left_thrust = 141e5,
	right_thrust = 141e5,
	angular_thrust = 2860e5,
	camera_offset = v(0,4,-35),
	gun_mounts =
	{
		{ v(0,-0.5,0), v(0,0,-1), 5, 'HORIZONTAL' },
		{ v(0,0,0), v(0,0,1), 5, 'HORIZONTAL' },
	},
	max_atmoshield = 0,
	max_cargo = 650,
	max_laser = 2,
	max_missile = 4,
	max_cargoscoop = 0,
	capacity = 650,
	hull_mass = 350,
	fuel_tank_mass = 300,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 55500e3,
	price = 550000,
	hyperdrive_class = 6,
}
