-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

--Ships not available for purchase (ambient ships)
define_static_ship {
	name='Lynx Bulk Carrier',
	model='lynx',
	forward_thrust = 800e5,
	reverse_thrust = 533e5,
	up_thrust = 360e5,
	down_thrust = 224e5,
	left_thrust = 224e5,
	right_thrust = 224e5,
	angular_thrust = 12000e5,
	gun_mounts =
	{
		{ v(0,0,-150), v(0,0,-1), 5, 'HORIZONTAL' },
		{ v(0,0,-150), v(0,0,-1), 5, 'HORIZONTAL' }
	},
	max_cargo = 3850,
	max_laser = 0,
	max_missile = 0,
	max_cargoscoop = 0,
	capacity = 3850,
	hull_mass = 800,
	fuel_tank_mass = 1200,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 55324e3,
	price = 6.5e6,
	hyperdrive_class = 8,
}
