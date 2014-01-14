-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Nerodia',
	ship_class='medium_freighter',
	manufacturer='opli',
	model='nerodia',
	forward_thrust = 25e7,
	reverse_thrust = 14e7,
	up_thrust = 14e7,
	down_thrust = 8e7,
	left_thrust = 8e7,
	right_thrust = 8e7,
	angular_thrust = 8e7,
	
	max_cargo = 2500,
	max_laser = 2,
	max_missile = 4,
	max_cargoscoop = 2,
	max_fuelscoop = 2,
	min_crew = 1,
	max_crew = 6,
	capacity = 2900,
	hull_mass = 2800,
	fuel_tank_mass = 1000,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 3e7,
	price = 1562000,
	hyperdrive_class = 7,
} 
