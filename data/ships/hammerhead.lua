-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Hammerhead Heavy Freighter',
	model='hh',
	forward_thrust = 14e7,
	reverse_thrust = 5e7,
	up_thrust = 5e7,
	down_thrust = 2e7,
	left_thrust = 2e7,
	right_thrust = 2e7,
	angular_thrust = 50e7,
	camera_offset = v(-1,1.3,-54.5),
	gun_mounts =
	{
		{ v(0,-2,-46), v(0,0,-1), 5, 'HORIZONTAL' },
		{ v(0,0,0), v(0,0,1), 5, 'HORIZONTAL' },
	},
	max_atmoshield = 0,
	max_cargo = 1220,
	max_laser = 2,
	max_missile = 12,
	max_cargoscoop = 0,
	min_crew = 8,
	max_crew = 14,
	capacity = 1220,
	hull_mass = 666,
	fuel_tank_mass = 337, --full tank, tons in addition to hull_mass
	thruster_fuel_use = 0.0003, --percent, per second (at max thrust, determined by strongest thruster)
	price = 3e6,
	hyperdrive_class = 7,
}
