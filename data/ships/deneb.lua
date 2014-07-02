-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Deneb',
	ship_class='medium_freighter',
	manufacturer='albr',
	model='deneb',
	forward_thrust = 5e7,
	reverse_thrust = 3e7,
	up_thrust = 4e7,
	down_thrust = 1e7,
	left_thrust = 1e7,
	right_thrust = 1e7,
	angular_thrust = 50e7,
	atmoshield = 1,
	min_atmoshield = 1,
	
	hull_mass = 175,
	fuel_tank_mass = 225,
	capacity = 1400,
	slots = {
		cargo = 1400,
		laser_front = 1,
		missile = 8,
		cargo_scoop = 1,
		fuel_scoop = 1,
	},
	min_crew = 2,
	max_crew = 3,
	effective_exhaust_velocity = 51784e3,
	price = 4300e3,
	hyperdrive_class = 3,
}