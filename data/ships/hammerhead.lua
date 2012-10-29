-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Hammerhead Heavy Freighter',
	model='hh',
	forward_thrust = 930e5,
	reverse_thrust = 332e5,
	up_thrust = 332e5,
	down_thrust = 133e5,
	left_thrust = 133e5,
	right_thrust = 133e5,
	angular_thrust = 65e7,
	camera_offset = v(-1,1.3,-54.5),
	gun_mounts =
	{
		{ v(0,-2,-46), v(0,0,-1), 5, 'HORIZONTAL' },
		{ v(0,0,0), v(0,0,1), 5, 'HORIZONTAL' },
	},
	max_atmoshield = 0,
	max_cargo = 1600,
	max_laser = 2,
	max_missile = 12,
	max_cargoscoop = 0,
	capacity = 1600,
	hull_mass = 666,
	fuel_tank_mass = 622, --full tank, tons in addition to hull_mass
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 55712e3, --exhaust speed of propellant in m/s,
	price = 3e6,
	hyperdrive_class = 7,
}
