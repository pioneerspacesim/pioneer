-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Nerodia',
	ship_class='medium_freighter',
	manufacturer='opli',
	model='nerodia',
	forward_thrust = 125e6,
	reverse_thrust = 7e7,
	up_thrust = 5e7,
	down_thrust = 4e7,
	left_thrust = 4e7,
	right_thrust = 4e7,
	angular_thrust = 8e7,
	
	hull_mass = 380,
	fuel_tank_mass = 1000,
	capacity = 2900,
	slots = {
		cargo = 2500,
		laser_front = 1,
		laser_rear = 1,
		missile = 4,
		cargoscoop = 1,
		fuelscoop = 1,
	},
	min_crew = 1,
	max_crew = 6,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 20e6,
	price = 4827e3,
	hyperdrive_class = 7,
} 

