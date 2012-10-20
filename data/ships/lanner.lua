-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Lanner',
	model='lanner_ub',
	forward_thrust = 478e5,
	reverse_thrust = 159e5,
	up_thrust = 159e5,
	down_thrust = 80e5,
	left_thrust = 80e5,
	right_thrust = 80e5,
	angular_thrust = 1170e5,
	cockpit_front = v(0,3,-28.5),
	cockpit_rear = v(0,5,-24),
	front_camera = v(0,0,-41),
	rear_camera = v(0,1,38.5),
	left_camera = v(-30,-5,18),
	right_camera = v(30,-5,18),
	top_camera = v(0,7,0),
	bottom_camera = v(0,-4,0),
	gun_mounts =
	{
	{v(0,-1.9,-38), v(0,0,-1)},
	{v(0,1,38), v(0,0,1)},
	},
	max_cargo = 191,
	max_laser = 2,
	max_missile = 4,
	max_cargoscoop = 0,
	capacity = 191,
	hull_mass = 130,
	fuel_tank_mass = 173,
	effective_exhaust_velocity = 56316e3,
	price = 280000,
	hyperdrive_class = 3,
}
