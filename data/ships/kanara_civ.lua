-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Kanara Interceptor',
	ship_class='light_fighter',
	manufacturer='mandarava_csepel',
	model='kanara_civ',
	forward_thrust = 8980000,
	reverse_thrust = 1800000,
	up_thrust = 18e5,
	down_thrust = 12e5,
	left_thrust = 12e5,
	right_thrust = 12e5,
	angular_thrust = 25e5,

	max_cargo = 17,
	max_laser = 1,
	max_missile = 8,
	max_cargoscoop = 0,
	max_engine = 1,
	min_crew = 1,
	max_crew = 2,
	capacity = 19,
	hull_mass = 16,
	fuel_tank_mass = 22,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 1625000,
	price = 700000,
	hyperdrive_class = 0,
}

