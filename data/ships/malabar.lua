-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Malabar',
	ship_class='medium_passenger_transport',
	manufacturer='mandarava_csepel',
	model='malabar',
	forward_thrust = 140e6,
	reverse_thrust = 50e6,
	up_thrust = 50e6,
	down_thrust = 30e6,
	left_thrust = 30e6,
	right_thrust = 30e6,
	angular_thrust = 120e6,

	max_cargo = 2000,
	max_laser = 2,
	max_missile = 0,
	max_cargoscoop = 1,
	max_engine = 1,
	min_crew = 1,
	max_crew = 8,
	capacity = 4600,
	hull_mass = 350,
	fuel_tank_mass = 2000,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 21e6,
	price = 700000,
	hyperdrive_class = 0,
}

