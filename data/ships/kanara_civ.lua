-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Kanara Interceptor',
	ship_class='light_fighter',
	manufacturer='mandarava_csepel',
	model='kanara_civ',
	forward_thrust = 6e6,
	reverse_thrust = 2e6,
	up_thrust = 1e6,
	down_thrust = 1e6,
	left_thrust = 1e6,
	right_thrust = 1e6,
	angular_thrust = 25e5,

	hull_mass = 16,
	fuel_tank_mass = 22,
	capacity = 19,
	max_cargo = 17,
	max_laser = 1,
	max_missile = 8,
	max_cargoscoop = 0,
	max_engine = 1,
	min_crew = 1,
	max_crew = 2,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 68e5,
	price = 234e3,
	hyperdrive_class = 0,
}

