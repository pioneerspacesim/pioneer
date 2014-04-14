-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_ship {
	name='Vatakara',
	ship_class='medium_freighter',
	manufacturer='mandarava_csepel',
	model='vatakara',
	forward_thrust = 160e6,
	reverse_thrust = 80e6,
	up_thrust = 70e6,
	down_thrust = 50e6,
	left_thrust = 50e6,
	right_thrust = 50e6,
	angular_thrust = 170e6,

	hull_mass = 660,
	fuel_tank_mass = 2200,
	capacity = 3000,
	slots = {
		cargo = 3000,
		missile = 0,
		laser_front = 1,
		laser_rear = 1,
		cargoscoop = 1,
        fuelscoop = 1,
		engine = 1,
	},
	min_crew = 1,
	max_crew = 8,
	-- Exhaust velocity Vc [m/s] is equivalent of engine efficiency and depend on used technology. Higher Vc means lower fuel consumption.
	-- Smaller ships built for speed often mount engines with higher Vc. Another way to make faster ship is to increase fuel_tank_mass.
	effective_exhaust_velocity = 1794e4,
	price = 6176e3,
	hyperdrive_class = 0,
}
