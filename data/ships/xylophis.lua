-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Xylophis',
	ship_class='light_cargo_shuttle',
	manufacturer='opli',
	model='xylophis',
	forward_thrust = 675e3,
	reverse_thrust = 225e3,
	up_thrust = 320e3,
	down_thrust = 225e3,
	left_thrust = 225e3,
	right_thrust = 225e3,
	angular_thrust = 45e5,
	
	hull_mass = 5,
	fuel_tank_mass = 10,
	capacity = 12,
	max_cargo = 12,
	max_laser = 1,
	max_missile = 0,
	max_cargoscoop = 1,
	max_fuelscoop = 0,
	min_crew = 1,
	max_crew = 1,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 68e5,
	price = 57000,
	hyperdrive_class = 1,
}
