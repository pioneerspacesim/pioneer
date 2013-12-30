-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Kanara Police Interceptor',
	ship_class='light_fighter',
	manufacturer='mandarava_csepel',
	model='kanara',
	forward_thrust = 9980000,
	reverse_thrust = 2100000,
	up_thrust = 18e5,
	down_thrust = 12e5,
	left_thrust = 12e5,
	right_thrust = 12e5,
	angular_thrust = 25e5,

	max_cargo = 9,
	max_laser = 1,
	max_missile = 8,
	max_cargoscoop = 0,
	max_engine = 0,
	min_crew = 1,
	max_crew = 2,
	capacity = 15,
	hull_mass = 17,
	fuel_tank_mass = 22,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 1625000,
	price = 0,
	hyperdrive_class = 0,
}

