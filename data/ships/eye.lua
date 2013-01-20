-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='E.Y.E',
	model='peye',
	forward_thrust = 320e5,
	reverse_thrust = 160e5,
	up_thrust = 80e5,
	down_thrust = 64e5,
	left_thrust = 64e5,
	right_thrust = 64e5,
	angular_thrust = 260e5,
	camera_offset = v(0,.2,-10.2),
	gun_mounts =
	{
		{ v(0,-5.8,-11), v(0,0,-1), 5, 'HORIZONTAL' },
		{ v(0,0,10), v(0,0,1), 5, 'HORIZONTAL' },
	},
	max_atmoshield = 0,
	max_cargo = 80,
	max_laser = 2,
	max_missile = 0,
	max_cargoscoop = 0,
	capacity = 80,
	hull_mass = 25,
	fuel_tank_mass = 103,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 58125e3,
	price = 100000,
	hyperdrive_class = 3,
}
