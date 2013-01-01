-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

--Ships not available for purchase (ambient ships)
define_static_ship {
	name='Long Range Cruiser',
	model='lrc',
	forward_thrust = 2e8,
	reverse_thrust = 5e7,
	up_thrust = 5e7,
	down_thrust = 5e7,
	left_thrust = 5e7,
	right_thrust = 5e7,
	angular_thrust = 5e7,
	gun_mounts =
	{
		{ v(0,0,-150), v(0,0,-1), 5, 'HORIZONTAL' },
		{ v(0,0,-150), v(0,0,-1), 5, 'HORIZONTAL' }
	},
	max_cargo = 15000,
	max_laser = 0,
	max_missile = 0,
	max_cargoscoop = 0,
	capacity = 15000,
	hull_mass = 4000,
	fuel_tank_mass = 1000,
	thruster_fuel_use = 0.0, -- These can be parked, engines running
	price = 3.1e8,
	hyperdrive_class = 10,
}
