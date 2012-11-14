-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Meteor',
	model='meteor',
	forward_thrust = 42e5,
	reverse_thrust = 4e5,
	up_thrust = 8e5,
	down_thrust = 8e5,
	left_thrust = 8e5,
	right_thrust = 8e5,
	angular_thrust = 13e5,
	camera_offset = v(0,1.3,-2.3),
	gun_mounts =
	{
		{ v(0,0,-25), v(0,0,-1), 5, 'HORIZONTAL' },
	},
	max_cargo = 12,
	max_missile = 4,
	max_cargoscoop = 0,
	max_ecm = 0,
	hyperdrive_class = 1,
	capacity = 12,
	hull_mass = 6,
	fuel_tank_mass = 13,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 75833e3,
	price = 18000,
}
