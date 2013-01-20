-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Caribou',
	model='caribou',
	forward_thrust = 724e5,
	reverse_thrust = 217e5,
	up_thrust = 217e5,
	down_thrust = 181e5,
	left_thrust = 181e5,
	right_thrust = 181e5,
	angular_thrust = 4250e5,
	camera_offset = v(0,4,-38),
	gun_mounts =
	{
		{ v(0,-0.5,-62), v(0,0,-1), 5, 'HORIZONTAL' },
		{ v(0,10,62), v(0,0,1), 5, 'HORIZONTAL' },
	},
	max_cargo = 941,
	max_laser = 2,
	max_missile = 20,
	max_cargoscoop = 0,
	capacity = 941,
	hull_mass = 460,
	fuel_tank_mass = 419,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 55357e3,
	price = 2.1e6,
	hyperdrive_class = 7,
}
