-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Mola Mola',
	ship_class='light_freighter',
	manufacturer='kaluri',
	model='molamola',
	forward_thrust = 5e6,
	reverse_thrust = 22e5,
	up_thrust = 19e5,
	down_thrust = 6e5,
	left_thrust = 6e5,
	right_thrust = 6e5,
	angular_thrust = 20e5,
	
	hull_mass = 27,
	fuel_tank_mass = 40,
	capacity = 80,
	max_cargo = 80,
	max_laser = 1,
	max_missile = 0,
	max_cargoscoop = 1,
	max_fuelscoop = 1,
	min_crew = 1,
	max_crew = 3,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 178e5,
	price = 199e3,
	hyperdrive_class = 1,
}
