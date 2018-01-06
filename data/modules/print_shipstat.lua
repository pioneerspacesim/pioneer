
local shipdefs = import("ShipDef")
local equipment = import("Equipment")
local maxfuel
local shipMass
local jumprange
local toprange = 0
local topship = nil

for _,ship in pairs(shipdefs) do
	--print("Ship name", "Hyperdrive", "Un-used capacity (t)", "Jumprange")
	for _,drive in pairs(equipment["hyperspace"]) do
		maxfuel = drive.capabilities.hyperclass^2
		if ship.capacity >= (drive.capabilities.mass + maxfuel) then
			shipMass = ship.hullMass + ship.capacity + ship.fuelTankMass
			jumprange = 200.0 * (drive.capabilities.hyperclass ^ 2) / shipMass
			print(ship.name, drive.l10n_key, "rcap=" .. (ship.capacity - (drive.capabilities.mass + maxfuel)), "range=" .. jumprange)
			if toprange < jumprange then
				topship = ship
				toprange = jumprange
			end
		end
	end
end

if topship then
	print("top ship=" .. topship.name, "range" .. toprange)
end