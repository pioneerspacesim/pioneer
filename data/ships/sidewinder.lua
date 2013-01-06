-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Sidewinder',
	model='rattlesnake',
	forward_thrust = 82e5,
	reverse_thrust = 62e5,
	up_thrust = 41e5,
	down_thrust = 41e5,
	left_thrust = 41e5,
	right_thrust = 41e5,
	angular_thrust = 130e5,
	camera_offset = v(0,2.5,0),
	gun_mounts =
	{
		{ v(0,0,-16), v(0,0,-1), 5, 'HORIZONTAL' },
		{ v(0,0,15), v(0,0,1), 5, 'HORIZONTAL' },
	},
	max_cargo = 30,
	max_laser = 2,
	max_missile = 0,
	max_fuelscoop = 1,
	max_cargoscoop = 1,
	capacity = 30,
	hull_mass = 20,
	fuel_tank_mass = 28,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 63333e3,
	price = 44000,
	hyperdrive_class = 2,
}
