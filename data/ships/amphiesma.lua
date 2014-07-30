-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Amphiesma',
	ship_class='medium_courier',
	manufacturer='opli',
	model='amphiesma',
	forward_thrust = 4e6,
	reverse_thrust = 1e6,
	up_thrust = 1e6,
	down_thrust = 5e5,
	left_thrust = 5e5,
	right_thrust = 5e5,
	angular_thrust = 4e6,
	
	hull_mass = 18,
	fuel_tank_mass = 24,
	capacity = 38,
	slots = {
		cargo = 38,
		laser_front = 1,
		missile = 4,
		cargo_scoop = 1,
		fuel_scoop = 1,
	},
	min_crew = 1,
	max_crew = 2,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 286e5,
	price = 168e3,
	hyperdrive_class = 1,
}

