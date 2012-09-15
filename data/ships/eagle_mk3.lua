-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See COPYING.txt for details

define_ship {
	name='Eagle MK-III',
	model = 'eagle_mk3',
	forward_thrust = -36e5,
	reverse_thrust = 25e5,
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
	hull_mass = 10,
	fuel_tank_mass = 5,
	thruster_fuel_use = 0.00015,
	price = 43000,
	hyperdrive_class = 1,
}
