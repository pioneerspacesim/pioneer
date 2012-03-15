define_ship {
	name='Walrus',
	model='walrus',
	forward_thrust = -40e6,
	reverse_thrust = 12e6,
	up_thrust = 12e6,
	down_thrust = -6e6,
	left_thrust = -6e6,
	right_thrust = 6e6,
	angular_thrust = 70e6,
	gun_mounts = { { v(0,-0.5,0), v(0,0,-1) }, { v(0,0,0), v(0,0,1) }, },
	max_cargo = 320,
	max_laser = 2,
	max_missile = 6,
	max_cargoscoop = 0,
	capacity = 320,
	hull_mass = 200,
	fuel_tank_mass = 100, --full tank, tons in addition to hull_mass
	thruster_fuel_use = 0.00015, --percent, per second (at max thrust, determined by strongest thruster)
	price = 350000,
	hyperdrive_class = 5,
}
