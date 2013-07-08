-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Pumpkinseed',
	model='pumpkinseed',
	forward_thrust = 5e6,
	reverse_thrust = 3e6,
	up_thrust = 5e5,
	down_thrust = 34e4,
	left_thrust = 34e4,
	right_thrust = 34e4,
	angular_thrust = 160e5,
	camera_offset = v(0,.6,-6),
	gun_mounts =
	{
		{ v(0,-0.5,-10.7), v(0,0,-1), 5, 'HORIZONTAL' },
		{ v(0,-0.5,0), v(0,0,1), 5, 'HORIZONTAL' },
	},
	max_cargo = 10,
	max_laser = 2,
	max_missile = 4,
	max_cargoscoop = 0,
	max_fuelscoop = 1,
	min_crew = 1,
	max_crew = 1,
	capacity = 17,
	hull_mass = 10,
	fuel_tank_mass = 7,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 8e7,
	price = 182000,
	hyperdrive_class = 1,
}
