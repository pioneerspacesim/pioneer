-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Natrix',
	ship_class='light_freighter',
	manufacturer='opli',
	model='natrix',
	forward_thrust = 61e5,
	reverse_thrust = 38e5,
	up_thrust = 38e5,
	down_thrust = 18e5,
	left_thrust = 18e5,
	right_thrust = 18e5,
	angular_thrust = 195e5,
	
	hull_mass = 50,
	fuel_tank_mass = 60,
	capacity = 80,
	slots = {
		cargo = 80 ,
		laser_front = 1,
		missile = 0,
		cargo_scoop = 0,
		fuel_scoop = 0,
		atmo_shield = 0,
	},
	min_crew = 1,
	max_crew = 3,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 182e5,
	price = 241e3,
	hyperdrive_class = 2,
}
