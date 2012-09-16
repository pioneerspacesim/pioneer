-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Natrix',
	model='natrix',
	forward_thrust = -6e6,
	reverse_thrust = 1e6,
	up_thrust = 1e6,
	down_thrust = -1e6,
	left_thrust = -1e6,
	right_thrust = 1e6,
	angular_thrust = 15e6,
	gun_mounts = {
		{ v(0.000, 0.000, -9.342), v(0.000, 0.000, -1.000) },
	},
	max_atmoshield = 0,
	max_cargo = 50,
	max_laser = 1,
	max_missile = 0,
	max_cargoscoop = 0,
	max_fuelscoop = 0,
	capacity = 40,
	hull_mass = 15,
	fuel_tank_mass = 15,
	thruster_fuel_use = 0.00015,
	price = 50000,
	hyperdrive_class = 2,
}
