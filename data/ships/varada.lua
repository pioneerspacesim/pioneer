-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Varada',
	ship_class='light_courier',
	manufacturer='mandarava_csepel',
	model='varada',
	forward_thrust = 600e3,
	reverse_thrust = 200e3,
	up_thrust = 120e3,
	down_thrust = 120e3,
	left_thrust = 240e3,
	right_thrust = 240e3,
	angular_thrust = 45e5,
	
	hull_mass = 2,
	fuel_tank_mass = 3,
	capacity = 5,
	slots = {
			cargo = 5,
			laser_front = 0,
			missile = 0,
			cargo_scoop = 0,
			fuel_scoop = 1,
		},
	min_crew = 1,
	max_crew = 1,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 140e5,
	price = 123e3,
	hyperdrive_class = 1,
}
