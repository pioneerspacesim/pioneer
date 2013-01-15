-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Sirius Interdictor',
	model='interdictor',
	forward_thrust = 242e5,
	reverse_thrust = 121e5,
	up_thrust = 61e5,
	down_thrust = 61e5,
	left_thrust = 61e5,
	right_thrust = 61e5,
	angular_thrust = 1560e5,
	camera_offset = v(0,4,-17.5),
	gun_mounts =
	{
		{ v(0,-0.5,0), v(0,0,-1), 5, 'HORIZONTAL' },
		{ v(0,-0.5,0), v(0,0,1), 5, 'HORIZONTAL' },
	},
	max_cargo = 90,
	max_laser = 2,
	max_missile = 8,
	max_cargoscoop = 0,
	capacity = 90,
	hull_mass = 66,
	fuel_tank_mass = 95,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 57591e3,
	price = 160000,
	hyperdrive_class = 4,
}
