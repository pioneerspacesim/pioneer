-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Military Fighter',
	model = 'military_fighter',
	forward_thrust = 52e5,
	reverse_thrust = 52e5,
	up_thrust = 130e5,
	down_thrust = 130e5,
	left_thrust = 130e5,
	right_thrust = 130e5,
	angular_thrust = 130e5,
	max_cargo = 18,
	max_missile = 2,
	max_fuelscoop = 0,
	max_cargoscoop = 0,
	min_crew = 1,
	max_crew = 1,
	capacity = 80,
	hull_mass = 28,
	fuel_tank_mass = 10,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 69286e3,
	price = 1,
	hyperdrive_class = 0,
}
