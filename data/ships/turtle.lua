-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Turtle',
	model='turtle',
	forward_thrust = 165e5,
	reverse_thrust = 165e5,
	up_thrust = 165e5,
	down_thrust = 165e5,
	left_thrust = 165e5,
	right_thrust = 165e5,
	angular_thrust = 390e5,
	camera_offset = v(0,-2,-16),
	gun_mounts =
	{
		{ v(0,-4,-10.2), v(0,0,-1), 5, 'HORIZONTAL' },
		{ v(0,-0.5,0), v(0,0,1), 5, 'HORIZONTAL' },
	},
	max_cargo = 90,
	max_laser = 2,
	max_missile = 4,
	max_cargoscoop = 0,
	capacity = 90,
	hull_mass = 47,
	fuel_tank_mass = 52,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 58226e3,
	price = 250000,
	hyperdrive_class = 3,
}
