-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Interplanetary Shuttle',
	model='ip_shuttle',
	forward_thrust = 29e5,
	reverse_thrust = 15e5,
	up_thrust = 15e5,
	down_thrust = 8e5,
	left_thrust = 8e5,
	right_thrust = 8e5,
	angular_thrust = 36e5,
	camera_offset = v(0,2,-12),
	gun_mounts =
	{
		{ v(0,-0.3,-7.9) , v(0,0,-1), 5, 'HORIZONTAL' },
		{ v(0,-0.3,7.5), v(0,0,1), 5, 'HORIZONTAL' },
	},
	max_cargo = 12,
	max_laser = 1,
	max_missile = 0,
	max_fuelscoop = 0,
	max_cargoscoop = 0,
	max_ecm = 0,
	max_engine = 0,
	hyperdrive_class = 0,
	capacity = 12,
	hull_mass = 11,
	fuel_tank_mass = 8,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 75000e3,
	price = 14000,
}
