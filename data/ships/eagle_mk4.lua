-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Eagle MK-IV "Bomber"',
	model='eagle_mk4',
	forward_thrust = 50e5,
	reverse_thrust = 25e5,
	up_thrust = 14e5,
	down_thrust = 12e5,
	left_thrust = 12e5,
	right_thrust = 12e5,
	angular_thrust = 90e5,
	camera_offset = v(0,1,-12.8),
	gun_mounts =
	{
		{ v(0,-.7,-40), v(0,0,-1), 5, 'HORIZONTAL' },
		{ v(0,-.7,25), v(0,0,1), 5, 'HORIZONTAL' },
	},
	max_cargo = 36,
	max_laser = 2,
	max_missile = 6,
	max_fuelscoop = 1,
	max_cargoscoop = 1,
	capacity = 36,
	hull_mass = 15,
	fuel_tank_mass = 5,
	thruster_fuel_use = 0.00015,
	price = 56000,
	hyperdrive_class = 2,
}
