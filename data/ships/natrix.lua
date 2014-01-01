-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Natrix',
	ship_class='light_freighter',
	manufacturer='opli',
	model='natrix',
	forward_thrust = 124e5,
	reverse_thrust = 21e5,
	up_thrust = 21e5,
	down_thrust = 21e5,
	left_thrust = 21e5,
	right_thrust = 21e5,
	angular_thrust = 195e5,
	max_atmoshield = 0,
	max_cargo = 40,
	max_laser = 1,
	max_missile = 0,
	max_cargoscoop = 0,
	max_fuelscoop = 0,
	min_crew = 1,
	max_crew = 1,
	capacity = 40,
	hull_mass = 15,
	fuel_tank_mass = 36,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 62143e3,
	price = 50000,
	hyperdrive_class = 2,
}
