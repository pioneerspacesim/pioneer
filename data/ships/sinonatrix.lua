-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Sinonatrix Courier',
	model='sinonatrix',
	forward_thrust = 25e6,
	reverse_thrust = 106e5,
	up_thrust = 106e5,
	down_thrust = 106e5,
	left_thrust = 106e5,
	right_thrust = 106e5,
	angular_thrust = 25e6,
	camera_offset = v(0,0,15),
	gun_mounts = {
		{ v(0.000, 0.000, -9.342), v(0.000, 0.000, -1.000), 5, 'HORIZONTAL' },
	},
	max_cargo = 35,
	max_laser = 1,
	max_missile = 2,
	max_cargoscoop = 1,
	max_fuelscoop = 0,
	min_crew = 1,
	max_crew = 1,
	capacity = 35,
	hull_mass = 185,
	fuel_tank_mass = 30,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 95e6,
	price = 310000,
	hyperdrive_class = 3,
} 