define_missile {
	name = 'MISSILE_UNGUIDED',
	model = 'missile',
	forward_thrust = -4e5,
	hull_mass=1,
	price=100
}

define_missile {
	name = 'MISSILE_GUIDED',
	model = 'missile',
	forward_thrust = -2e5,
	reverse_thrust = 1e5,
	angular_thrust = 2e4,
	hull_mass=1,
	price=100
}

define_missile {
	name = 'MISSILE_SMART',
	model = 'missile',
	forward_thrust = -3e5,
	reverse_thrust = 1.5e5,
	angular_thrust = 2e4,
	hull_mass=1,
	price=100
}

define_missile {
	name = 'MISSILE_NAVAL',
	model = 'missile',
	forward_thrust = -4e5,
	reverse_thrust = 2e5,
	angular_thrust = 2e4,
	hull_mass=1,
	price=100
}
