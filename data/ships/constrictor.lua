define_ship {
	name = 'Constrictor',
	model='conny',
	forward_thrust = -259e5,
	reverse_thrust = 118e5,
	up_thrust = 6e6,
	down_thrust = -6e6,
	left_thrust = -6e6,
	right_thrust = 6e6,
	angular_thrust = 90e6,
	gun_mounts =
	{
		{ v(0,-2,-26), v(0,0,-1) },
		{ v(0,-2,19), v(0,0,1) }
	},
	max_cargo = 90,
	max_laser = 2,
	max_missile = 2,
	max_fuelscoop = 1,
	max_cargoscoop = 1,
	capacity = 90,
	hull_mass = 60,
	fuel_tank_mass = 30,
	thruster_fuel_use = 0.00035,
	price = 143000,
	hyperdrive_class = 3,
}
