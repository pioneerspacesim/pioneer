-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Amphiesma',
	ship_class='medium_courier',
	manufacturer='opli',
	model='amphiesma',
	forward_thrust = 8e6,
	reverse_thrust = 2e6,
	up_thrust = 1e6,
	down_thrust = 5e5,
	left_thrust = 5e5,
	right_thrust = 5e5,
	angular_thrust = 4e6,
	max_cargo = 38,
	max_laser = 1,
	max_missile = 4,
	max_cargoscoop = 1,
	max_fuelscoop = 1,
	min_crew = 1,
	max_crew = 2,
	capacity = 38,
	hull_mass = 18,
	fuel_tank_mass = 24,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 68e5,
	price = 262000,
	hyperdrive_class = 1,
}
