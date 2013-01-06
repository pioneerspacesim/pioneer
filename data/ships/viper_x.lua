-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Viper X',
	model='viper_x',
	forward_thrust = 155e5,
	reverse_thrust = 52e5,
	up_thrust = 52e5,
	down_thrust = 34e5,
	left_thrust = 34e5,
	right_thrust = 34e5,
	angular_thrust = 260e5,
	camera_offset = v(4,2,-1),
	gun_mounts =
	{
		{ v(0,-1.4,-28), v(0,0,-1), 5, 'HORIZONTAL' },
		{ v(0,0,0), v(0,0,1), 5, 'HORIZONTAL' }
	},
	max_cargo = 55,
	max_laser = 1,
	max_missile = 4,
	max_cargoscoop = 0,
	max_fuelscoop = 1,
	capacity = 55,
	hull_mass = 25,
	fuel_tank_mass = 44,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 60263e3,
	price = 90000,
	hyperdrive_class = 2,
}
