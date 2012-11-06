-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Boa Freighter',
	model='boa',
	forward_thrust = 693e5,
	reverse_thrust = 231e5,
	up_thrust = 231e5,
	down_thrust = 152e5,
	left_thrust = 152e5,
	right_thrust = 152e5,
	angular_thrust = 6500e5,
	camera_offset = v(0,6,-4),
	gun_mounts =
	{
		{ v(0,-2,-46), v(0,0,-1), 1, 'HORIZONTAL' },
		{ v(0,3,48), v(0,0,1), 6, 'VERTICAL' },
	},
	max_cargo = 774,
	max_laser = 2,
	max_missile = 6,
	max_cargoscoop = 0,
	capacity = 774,
	hull_mass = 300,
	fuel_tank_mass = 460,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 55424e3,
	price = 2474000,
	hyperdrive_class = 7,
}
