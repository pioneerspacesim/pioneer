-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

require("wiki_commons")

define_ship = function(ship_table)
	forward_thrust_empty = round(thrust_empty(ship_table.forward_thrust, ship_table.hull_mass, ship_table.fuel_tank_mass), 1)

	print("|-")
	print("|[["..ship_table.name.."]]")
	print("|"..forward_thrust_empty)
end

-- ship files
ships = {"amphiesma", "deneb", "dsminer", "kanara_civ", "kanara", "lunarshuttle", "malabar", "molamola", "natrix", "nerodia", "pumpkinseed", "sinonatrix", "vatakara", "venturestar", "wave", "xylophis"}
table.sort(ships)

-- print wiki intro/heading
print('{| class="wikitable sortable"')
print("|+Ships Comparison")
print("! Name")
print("! Forward Thrust")

-- for each print stats
for i = 1, #ships, 1 do
	require(ships[i])
end

-- print wiki outro
print("|}")
