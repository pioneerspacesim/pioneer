-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Eagle MK-II',
	model = 'eagle_mk2',
	forward_thrust = -34e5,
	reverse_thrust = 17e5,
	up_thrust = 8e5,
	down_thrust = -8e5,
	left_thrust = -8e5,
	right_thrust = 8e5,
	angular_thrust = 64e5,
	rear_camera = v(0,5,-10),
	gun_mounts =
	{
		{ v(0,-.7,-40), v(0,0,-1) },
		{ v(0,-.7,25), v(0,0,1) },
	},
	max_cargo = 22,
	max_missile = 2,
	max_fuelscoop = 0,
	max_cargoscoop = 0,
	capacity = 22,
	hull_mass = 9,
	fuel_tank_mass = 6,
	thruster_fuel_use = 0.0001,
	price = 41000,
	hyperdrive_class = 1,
}
