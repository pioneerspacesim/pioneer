-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Eagle MK-IV "Bomber"',
	model='eagle_mk4',
	forward_thrust = 98e5,
	reverse_thrust = 49e5,
	up_thrust = 27e5,
	down_thrust = 24e5,
	left_thrust = 24e5,
	right_thrust = 24e5,
	angular_thrust = 117e5,
	camera_offset = v(0,1,-12.8),
	gun_mounts =
	{
		{ v(0,-.7,-40), v(0,0,-1), 5, 'HORIZONTAL' },
		{ v(0,-.7,25), v(0,0,1), 5, 'HORIZONTAL' },
	},
	max_cargo = 34,
	max_laser = 2,
	max_missile = 6,
	max_fuelscoop = 1,
	max_cargoscoop = 1,
	capacity = 34,
	hull_mass = 15,
	fuel_tank_mass = 24,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 63929e3,
	price = 56000,
	hyperdrive_class = 2,
}
