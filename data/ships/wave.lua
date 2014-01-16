-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Wave',
	ship_class='medium_fighter',
	manufacturer='auronox',
	model='wave',
	forward_thrust = 68e5,
	reverse_thrust = 14e5,
	up_thrust = 8e5,
	down_thrust = 8e5,
	left_thrust = 8e5,
	right_thrust = 8e5,
	angular_thrust = 70e5,
	
	hull_mass = 13,
	fuel_tank_mass = 22,
	capacity = 30,
	max_cargo = 30,
	max_laser = 2,
	max_missile = 4,
	max_cargoscoop = 0,
	max_fuelscoop = 0,
	min_crew = 1,
	max_crew = 1,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 13e6,
	price = 269e3,
	hyperdrive_class = 2,
}
