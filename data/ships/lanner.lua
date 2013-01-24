-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Lanner',
	model='lanner',
	forward_thrust = 478e5,
	reverse_thrust = 159e5,
	up_thrust = 159e5,
	down_thrust = 80e5,
	left_thrust = 80e5,
	right_thrust = 80e5,
	angular_thrust = 1170e5,
	camera_offset = v(0,3,-28.5),
	gun_mounts =
	{
		{ v(0,-1.9,-38), v(0,0,-1), 5, 'HORIZONTAL' },
		{ v(0,1,38), v(0,0,1), 5, 'HORIZONTAL' },
	},
	max_cargo = 191,
	max_laser = 2,
	max_missile = 4,
	max_cargoscoop = 0,
	capacity = 191,
	hull_mass = 130,
	fuel_tank_mass = 173,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 56316e3,
	price = 280000,
	hyperdrive_class = 3,
}
