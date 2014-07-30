-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Pumpkinseed',
	ship_class='light_courier',
	manufacturer='kaluri',
	model='pumpkinseed',
	forward_thrust = 3e6,
	reverse_thrust = 1e6,
	up_thrust = 5e5,
	down_thrust = 34e4,
	left_thrust = 34e4,
	right_thrust = 34e4,
	angular_thrust = 160e5,
	
	hull_mass = 10,
	fuel_tank_mass = 7,
	capacity = 17,
	slots = {
		cargo = 10,
		laser_front = 1,
		laser_rear = 1,
		missile = 4,
		cargoscoop = 0,
		fuelscoop = 1,
	},
	min_crew = 1,
	max_crew = 1,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 39e6,
	price = 162e3,
	hyperdrive_class = 1,
}
