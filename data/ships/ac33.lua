-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='AC33 Dropstar',
	model='ac33',
	ship_class='medium_freighter',
	manufacturer='albr',
	forward_thrust = 15e7,
	reverse_thrust = 50e6,
	up_thrust = 60e6,
	down_thrust = 20e6,
	left_thrust = 20e6,
	right_thrust = 20e6,
	angular_thrust = 15e7,
	
	hull_mass = 1150,
	fuel_tank_mass = 250,	
	capacity = 3000,
	slots = {
		cargo = 3000,
		laser_front = 1,
		missile = 16,
		cargo_scoop = 0,
		fuel_scoop = 0,
		atmo_shield = 1,
	},
	min_crew = 3,
	max_crew = 5,
	
	effective_exhaust_velocity = 91784e3,
	price = 4500e3,
	--hyperdrive_mil = 4,
	hyperdrive_class = 0,
}

