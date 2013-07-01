-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Deneb Transport',
	model='deneb',
	forward_thrust = 15e6,
	reverse_thrust = 5e6,
	up_thrust = 10e6,
	down_thrust = 4e6,
	left_thrust = 4e6,
	right_thrust = 4e6,
	angular_thrust = 50e6,
	camera_offset = v(0,0,-16),
	gun_mounts =
	{
		{ v(0,-0.5,-10.7), v(0,0,-1), 5, 'HORIZONTAL' },
	},
	atmoshield = 1,
	min_atmoshield = 1,
	max_cargo = 235,
	max_laser = 1,
	max_missile = 8,
	max_cargoscoop = 1,
	max_fuelscoop = 1,
	min_crew = 2,
	max_crew = 3,
	capacity = 235,
	hull_mass = 100,
	fuel_tank_mass = 100,
	--thruster_fuel_use = 0.0001,
	effective_exhaust_velocity = 51784e3,
	price = 430000,
	hyperdrive_class = 3,
}
