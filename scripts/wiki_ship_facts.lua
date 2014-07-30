#!/usr/bin/lua
-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

require("wiki_commons")

define_ship = function(shipTable)
	-- calculate thrust values
	forward_thrust_empty = round(thrust_empty(shipTable.forward_thrust, shipTable.hull_mass, shipTable.fuel_tank_mass), 1)
	forward_thrust_full = round(thrust_full(shipTable.forward_thrust, shipTable.hull_mass, shipTable.fuel_tank_mass, shipTable.capacity), 1)
	reverse_thrust_empty = round(thrust_empty(shipTable.reverse_thrust, shipTable.hull_mass, shipTable.fuel_tank_mass), 1)
	reverse_thrust_full = round(thrust_full(shipTable.reverse_thrust, shipTable.hull_mass, shipTable.fuel_tank_mass, shipTable.capacity), 1)
	up_thrust_empty = round(thrust_empty(shipTable.up_thrust, shipTable.hull_mass, shipTable.fuel_tank_mass), 1)
	up_thrust_full = round(thrust_full(shipTable.up_thrust, shipTable.hull_mass, shipTable.fuel_tank_mass, shipTable.capacity), 1)
	down_thrust_empty = round(thrust_empty(shipTable.down_thrust, shipTable.hull_mass, shipTable.fuel_tank_mass), 1)
	down_thrust_full = round(thrust_full(shipTable.down_thrust, shipTable.hull_mass, shipTable.fuel_tank_mass, shipTable.capacity), 1)
	left_thrust_empty = round(thrust_empty(shipTable.left_thrust, shipTable.hull_mass, shipTable.fuel_tank_mass), 1)
	left_thrust_full = round(thrust_full(shipTable.left_thrust, shipTable.hull_mass, shipTable.fuel_tank_mass, shipTable.capacity), 1)
	right_thrust_empty = round(thrust_empty(shipTable.right_thrust, shipTable.hull_mass, shipTable.fuel_tank_mass), 1)
	right_thrust_full = round(thrust_full(shipTable.right_thrust, shipTable.hull_mass, shipTable.fuel_tank_mass, shipTable.capacity), 1)
	angular_thrust_empty = round(thrust_empty(shipTable.angular_thrust, shipTable.hull_mass, shipTable.fuel_tank_mass), 1)
	angular_thrust_full = round(thrust_full(shipTable.angular_thrust, shipTable.hull_mass, shipTable.fuel_tank_mass, shipTable.capacity), 1)

	-- check for hyperdrive
	if shipTable.max_engine == nil or shipTable.max_engine == 1 then
		max_engine = "yes"
	else
		max_engine = "no"
	end

	-- calculate effective exhaust velocity
	effective_exhaust_velocity_empty = shipTable.effective_exhaust_velocity * math.log((shipTable.hull_mass + shipTable.fuel_tank_mass) / shipTable.hull_mass)
	effective_exhaust_velocity_full = shipTable.effective_exhaust_velocity * math.log((shipTable.hull_mass + shipTable.fuel_tank_mass + shipTable.capacity) / (shipTable.hull_mass + shipTable.capacity))
	effective_exhaust_velocity_empty = round(effective_exhaust_velocity_empty, -3) / 1000
	effective_exhaust_velocity_full = round(effective_exhaust_velocity_full, -3) / 1000

	-- print wiki syntax
	print('{{Infobox_Ship')
	print("|name = "..shipTable.name)
	print("|type = "..shipTable.ship_class)
	print("|manufacturer = "..shipTable.manufacturer)
	print("|forward_thrust = empty: "..forward_thrust_empty.."G full: "..forward_thrust_full.."G")
	print("|reverse_thrust = empty: "..reverse_thrust_empty.."G full: "..reverse_thrust_full.."G")
	print("|up_thrust = empty: "..up_thrust_empty.."G full: "..up_thrust_full.."G")
	print("|down_thrust = empty: "..down_thrust_empty.."G full: "..down_thrust_full.."G")
	print("|left_thrust = empty: "..left_thrust_empty.."G full: "..left_thrust_full.."G")
	print("|right_thrust = empty: "..right_thrust_empty.."G full: "..right_thrust_full.."G")
	print("|angular_thrust = empty: "..angular_thrust_empty.."G full: "..angular_thrust_full.."G")
	print("|max_cargo = "..shipTable.max_cargo)
	print("|max_laser = 1"..shipTable.max_laser)
	print("|max_missile = "..shipTable.max_missile)
	print("|max_cargoscoop = "..shipTable.max_cargoscoop)
	print("|max_fuelscoop = "..shipTable.max_fuelscoop)
	print("|min_crew = "..shipTable.min_crew)
	print("|max_crew = "..shipTable.max_crew)
	print("|capacity = "..shipTable.capacity)
	print("|hull_mass = "..shipTable.hull_mass)
	print("|fuel_tank_mass = "..shipTable.fuel_tank_mass)
	print("|effective_exhaust_velocity = empty: "..effective_exhaust_velocity_empty.."km/s full: "..effective_exhaust_velocity_full.."km/s")
	print("|price = "..shipTable.price)
	print("|hyperdrive_class = "..shipTable.hyperdrive_class)
	print("|max_engine = "..max_engine)
	print("}}")
end

usage_error = function()
	print("usage:")
	print("start from scripts directory")
	print(arg[0], "../data/ships/<ship_script_name>")
	os.exit(1)
end

file_exists = function(file_name)
	local f=io.open(file_name,"r")
	if f ~= nil then
		io.close(f)
		return true
	else
		return false
	end
end

-- check arg count
if #arg ~= 1 then
	usage_error()
end

-- check if file exists
if file_exists(arg[1]) == false then
	print("file \""..arg[1].."\"does not exist")
	usage_error()
end

-- remove extension
-- extPos = string.find(arg[1], ".lua")
-- shipNameEnd = extPos - 1
-- shipModule = string.sub(arg[1], 1, shipNameEnd)

-- load ship module
dofile(arg[1])
