-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

round = function(number, significand)
	last_number = number * 10^(significand+1)
	last_number = math.floor(last_number)
	length = string.len(last_number)
	last_number = string.sub(last_number, length)

	number = number * 10^significand
	if tonumber(last_number) < 5 then
		number = math.floor(number)
	else
		number = math.ceil(number)
	end
	number = number / 10^significand
	return tonumber(number)
end

-- just to have something to manage the calls from ship definitions with deprecated
-- values for gun_mounts and camera_offset
v = function(a, b, c)
	return nil
end

thrust_empty = function(thrust, hull_mass, fuel_tank_mass)
	this_thrust_empty = thrust / (9.81*1000*(hull_mass + fuel_tank_mass))
	return this_thrust_empty
end

thrust_full = function(thrust, hull_mass, fuel_tank_mass, capacity)
	this_thrust_empty = thrust / (9.81*1000*(hull_mass + fuel_tank_mass + capacity))
	return this_thrust_empty
end

