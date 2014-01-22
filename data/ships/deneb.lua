-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Deneb',
	ship_class='medium_freighter',
	manufacturer='albr',
	model='deneb',
	forward_thrust = 10e6,
	reverse_thrust = 6e6,
	up_thrust = 6e6,
	down_thrust = 3e6,
	left_thrust = 3e6,
	right_thrust = 3e6,
	angular_thrust = 50e6,
	atmoshield = 1,
	min_atmoshield = 1,

	hull_mass = 75,
	fuel_tank_mass = 100,
	capacity = 235,
	max_cargo = 235,
	max_laser = 1,
	max_missile = 8,
	max_cargoscoop = 1,
	max_fuelscoop = 1,
	min_crew = 2,
	max_crew = 3,
	--thruster_fuel_use = 0.0001,
	effective_exhaust_velocity = 186e5,
	price = 392e3,
	hyperdrive_class = 3,
}
