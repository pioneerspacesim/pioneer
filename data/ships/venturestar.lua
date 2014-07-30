-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Venturestar',
	ship_class='medium_freighter',
	manufacturer='albr',
	model='venturestar',
	forward_thrust = 10e7,
	reverse_thrust = 50e6,
	up_thrust = 50e6,
	down_thrust = 16e6,
	left_thrust = 16e6,
	right_thrust = 16e6,
	angular_thrust = 10e7,

	hull_mass = 900,
	fuel_tank_mass = 500,
	capacity = 3000,
	slots = {
		cargo = 3000,
		laser_front = 1,
		missile = 12,
		cargo_scoop = 1,
		fuel_scoop = 1,
		atmo_shield = 1,
	},
	
	min_crew = 2,
	max_crew = 5,
	
	effective_exhaust_velocity = 81784e3,
	price = 5800e3,
	hyperdrive_class = 9,
}
