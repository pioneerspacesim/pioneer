-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Stardust',
	model='stardust',
	forward_thrust = -200e5,
	reverse_thrust = 80e5,
	up_thrust = 40e5,
	down_thrust = -40e5,
	left_thrust = -40e5,
	right_thrust = 40e5,
	angular_thrust = 320e5,
	gun_mounts =
	{
		{ v(0,0,-26), v(0,0,-1) },
		{ v(0,0,5), v(0,0,1) },
	},
	max_cargo = 100,
	max_laser = 2,
	max_fuelscoop = 0,
	max_cargoscoop = 0,
	capacity = 100,
	hull_mass = 35,
	fuel_tank_mass = 65,
	thruster_fuel_use = 0.0002,
	price = 150000,
	hyperdrive_class = 4,
}
