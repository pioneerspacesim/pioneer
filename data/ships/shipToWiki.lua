-- TODO:
-- - check about gunmounts
-- - check about max_engine/Hyperdrive mount = yes
-- - check exhaust velocity
-- - wiki all ships table
-- - update ships in wiki

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

thrust_empty = function(thrust, hull_mass, fuel_tank_mass)
	this_thrust_empty = thrust / (9.81*1000*(hull_mass + fuel_tank_mass))
	return this_thrust_empty
end

thrust_full = function(thrust, hull_mass, fuel_tank_mass, capacity)
	this_thrust_empty = thrust / (9.81*1000*(hull_mass + fuel_tank_mass + capacity))
	return this_thrust_empty
end

define_ship = function(shipTable)
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

	print('{{Infobox_Ship')
	print("|name = "..shipTable.name)
	print("|type = "..shipTable.ship_class)
	print("|manufacturer = "..shipTable.manufacturer)
	print("|forward_thrust = "..forward_thrust_empty.."G ("..forward_thrust_full.."G full)")
	print("|reverse_thrust = "..reverse_thrust_empty.."G ("..reverse_thrust_full.."G full)")
	print("|up_thrust = "..up_thrust_empty.."G ("..up_thrust_full.."G full)")
	print("|down_thrust = "..down_thrust_empty.."G ("..down_thrust_full.."G full)")
	print("|left_thrust = "..left_thrust_empty.."G ("..left_thrust_full.."G full)")
	print("|right_thrust = "..right_thrust_empty.."G ("..right_thrust_full.."G full)")
	print("|angular_thrust = "..angular_thrust_empty.."G ("..angular_thrust_full.."G full)")
	print("|gun_mounts = ")
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
	print("|effective_exhaust_velocity = "..shipTable.effective_exhaust_velocity)
	print("|price = "..shipTable.price)
	print("|hyperdrive_class = "..shipTable.hyperdrive_class)
	print("|max_engine = ")
	print("}}")
end

usage_error = function()
	print('usage: lua shipToWiki.lua <ship_script_name>')
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
extPos = string.find(arg[1], ".lua")
shipNameEnd = extPos - 1
shipModule = string.sub(arg[1], 1, shipNameEnd)

-- load ship module
require(shipModule)
