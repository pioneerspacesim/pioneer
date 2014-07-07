#!/usr/bin/lua
-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

require("wiki_commons")

define_ship = function(ship_table)
	forward_thrust_empty = round(thrust_empty(ship_table.forward_thrust, ship_table.hull_mass, ship_table.fuel_tank_mass), 1)
	angular_thrust_empty = round(thrust_empty(ship_table.angular_thrust, ship_table.hull_mass, ship_table.fuel_tank_mass), 1)

	print("|-")
	print("|[["..ship_table.name.."]]")
	print("|"..ship_table.max_laser)
	print("|"..ship_table.max_missile)
	print("|"..ship_table.max_cargo)
	print("|"..forward_thrust_empty)
	print("|"..angular_thrust_empty)
	print("|"..ship_table.price)
end

-- ship files
ships = {"ac33", "amphiesma", "deneb", "dsminer", "kanara_civ", "kanara", "lunarshuttle", "malabar", "molamola", "natrix", "nerodia", "pumpkinseed", "sinonatrix", "varada" , "vatakara", "venturestar", "wave", "xylophis"}
table.sort(ships)

-- print wiki intro/heading
print('{| class="wikitable sortable"')
print("! Name")
print("! max Laser")
print("! max Missile")
print("! max Cargo")
print("! Forward Thrust")
print("! Angular Thrust")
print("! Price")

-- for each print stats
package.path = package.path..";../data/ships/?.lua"
for i = 1, #ships, 1 do
	require(ships[i])
end

-- print wiki outro
print("|}")
