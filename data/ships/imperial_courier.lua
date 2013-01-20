-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name = 'Imperial Courier',
	model = 'courier',
	forward_thrust = 399e5,
	reverse_thrust = 120e5,
	up_thrust = 120e5,
	down_thrust = 84e5,
	left_thrust = 84e5,
	right_thrust = 84e5,
	angular_thrust = 1430e5,
	camera_offset = v(0,3.5,-25),
	gun_mounts =
	{
		{ v(0,0.6,-25), v(0,0,-1), 5, 'HORIZONTAL' },
		{ v(0,0,16), v(0,0,1), 5, 'HORIZONTAL' },
	},
	max_cargo = 400,
	max_laser = 1,
	max_missile = 6,
	max_cargoscoop = 0,
	capacity = 400,
	hull_mass = 200,
	fuel_tank_mass = 180,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 55833e3,
	price = 611000,
	hyperdrive_class = 4,
}
