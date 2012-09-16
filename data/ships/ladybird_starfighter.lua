-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Ladybird Starfighter',
	model='ladybird',
	forward_thrust = -10e6,
	reverse_thrust = 4e6,
	up_thrust = 3e6,
	down_thrust = -3e6,
	left_thrust = -2e6,
	right_thrust = 2e6,
	angular_thrust = 16e6,
	gun_mounts = { { v(0,-0.5,0), v(0,0,-1) }, { v(0,0,0), v(0,0,1) }, },
	max_cargo = 60,
	max_missile = 2,
	max_laser = 2,
	max_cargoscoop = 0,
	capacity = 60,
	hull_mass = 40,
	fuel_tank_mass = 20,
	thruster_fuel_use = 0.0004,
	price = 87000,
	hyperdrive_class = 3,
}
