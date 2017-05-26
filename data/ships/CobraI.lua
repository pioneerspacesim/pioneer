--cloned from 'cobra'
-- a favorite for limited trading and outpost raids 
-- low sensor strength if implemented.  Fuel tank on the small side
-- turns like a shark, average speed


define_ship {
	name='Cobra Mk II',
	model='CobraI',
	forward_thrust = -12e6,
	reverse_thrust = 5e6,
	up_thrust = 4e6,
	down_thrust = -4e6,
	left_thrust = -4e6,
	right_thrust = 4e6,
	angular_thrust = 28e6,
	gun_mounts = {
		{ v(0.000, -0.25, -1.16), v(0.000, 0.000, -1.000) },
		{ v(0.000, -0.29, 1.35), v(0.000, 0.000, 1.000) },
	},
	max_cargo = 60,
	max_laser = 2,
	max_missile = 2,
	max_cargoscoop = 0,
	max_fuelscoop = 0,
	capacity = 60,
	hull_mass = 40,
	fuel_tank_mass = 20,
	thruster_fuel_use = 0.0002,
	price = 97000,
	hyperdrive_class = 2,
}
