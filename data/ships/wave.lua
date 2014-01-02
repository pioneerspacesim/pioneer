-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Wave',
	ship_class='medium_fighter',
	manufacturer='auronox',
	model='wave',
	forward_thrust = 88e5,
	reverse_thrust = 29e5,
	up_thrust = 15e5,
	down_thrust = 15e5,
	left_thrust = 15e5,
	right_thrust = 15e5,
	angular_thrust = 140e5,
	max_cargo = 30,
	max_laser = 2,
	max_missile = 4,
	max_cargoscoop = 0,
	max_fuelscoop = 0,
	min_crew = 1,
	max_crew = 1,
	capacity = 30,
	hull_mass = 13,
	fuel_tank_mass = 22,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 65000e3,
	price = 93000,
	hyperdrive_class = 2,
}
