-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Lang = import("Lang")
local Game = import("Game")
local Equipment = import("Equipment")
local ShipDef = import("ShipDef")

local ModelSpinner = import("UI.Game.ModelSpinner")

local ui = Engine.ui
local l = Lang.GetResource("ui-core");
local lcore = Lang.GetResource("core");

local yes_no = function (binary)
	if binary == 1 then
		return l.YES
	elseif binary == 0 then
		return l.NO
	else error("argument to yes_no not 0 or 1")
	end
end

local shipInfo = function (args)
	local shipDef = ShipDef[Game.player.shipId]

	local hyperdrive =              table.unpack(Game.player:GetEquip("engine"))
	local frontWeapon =             table.unpack(Game.player:GetEquip("laser_front"))
	local rearWeapon =              table.unpack(Game.player:GetEquip("laser_rear"))

	hyperdrive =  hyperdrive  or nil
	frontWeapon = frontWeapon or nil
	rearWeapon =  rearWeapon  or nil

	local player = Game.player

	local mass_with_fuel = player.totalMass + player.fuelMassLeft
	local mass_with_fuel_kg = 1000 * mass_with_fuel

	-- ship stats mass is in tonnes; scale by 1000 to convert to kg
	local fwd_acc = -shipDef.linearThrust.FORWARD / mass_with_fuel_kg
	local bwd_acc = shipDef.linearThrust.REVERSE / mass_with_fuel_kg
	local up_acc = shipDef.linearThrust.UP / mass_with_fuel_kg

	-- delta-v calculation according to http://en.wikipedia.org/wiki/Tsiolkovsky_rocket_equation
	local deltav = shipDef.effectiveExhaustVelocity * math.log((player.totalMass + player.fuelMassLeft) / player.totalMass)

	local equipItems = {}
	local equips = {Equipment.cargo, Equipment.misc, Equipment.hyperspace, Equipment.laser}
	for _,t in pairs(equips) do
		for k,et in pairs(t) do
			local slot = et:GetDefaultSlot(Game.player)
			if (slot ~= "cargo" and slot ~= "missile" and slot ~= "engine" and slot ~= "laser_front" and slot ~= "laser_rear") then
				local count = Game.player:CountEquip(et)
				if count > 0 then
					if count > 1 then
						if et == Equipment.misc.shield_generator then
							table.insert(equipItems,
								ui:Label(string.interp(l.N_SHIELD_GENERATORS, { quantity = string.format("%d", count) })))
						elseif et == Equipment.misc.cabin_occupied then
							table.insert(equipItems,
								ui:Label(string.interp(l.N_OCCUPIED_PASSENGER_CABINS, { quantity = string.format("%d", count) })))
						elseif et == Equipment.misc.cabin then
							table.insert(equipItems,
								ui:Label(string.interp(l.N_UNOCCUPIED_PASSENGER_CABINS, { quantity = string.format("%d", count) })))
						else
							table.insert(equipItems, ui:Label(et:GetName()))
						end
					else
						table.insert(equipItems, ui:Label(et:GetName()))
					end
				end
			end
		end
	end

	return
		ui:Grid({48,4,48},1)
			:SetColumn(0, {
				ui:Table():AddRows({
					ui:Table():SetColumnSpacing(10):AddRows({
						{ l.HYPERDRIVE..":", hyperdrive and hyperdrive:GetName() or "None" },
						{
							l.HYPERSPACE_RANGE..":",
							string.interp(
								l.N_LIGHT_YEARS_N_MAX, {
									range    = string.format("%.1f",player.hyperspaceRange),
									maxRange = string.format("%.1f",player.maxHyperspaceRange)
								}
							),
						},
						"",
						{ l.WEIGHT_EMPTY..":",  string.format("%dt", player.totalMass - player.usedCapacity) },
						{ l.CAPACITY_USED..":", string.format("%dt (%dt "..l.FREE..")", player.usedCapacity,  player.freeCapacity) },
						{ l.FUEL_WEIGHT..":",   string.format("%dt (%dt "..l.MAX..")", player.fuelMassLeft, shipDef.fuelTankMass ) },
						{ l.ALL_UP_WEIGHT..":", string.format("%dt", mass_with_fuel ) },
						"",
						{ l.FRONT_WEAPON..":", frontWeapon and frontWeapon:GetName() or "None"},
						{ l.REAR_WEAPON..":",  rearWeapon and rearWeapon:GetName() or "None" },
						{ l.FUEL..":",         string.format("%d%%", Game.player.fuel)},
						{ l.DELTA_V..":",      string.format("%d km/s", deltav / 1000)},
						"",
						{ l.FORWARD_ACCEL..":",  string.format("%.2f m/s² (%.1f G)", fwd_acc, fwd_acc / 9.81) },
						{ l.BACKWARD_ACCEL..":", string.format("%.2f m/s² (%.1f G)", bwd_acc, bwd_acc / 9.81) },
						{ l.UP_ACCEL..":",       string.format("%.2f m/s² (%.1f G)", up_acc, up_acc / 9.81) },
						"",
						{ l.MINIMUM_CREW..":", shipDef.minCrew },
						{ l.CREW_CABINS..":",  shipDef.maxCrew },
						"",
						{ l.MISSILE_MOUNTS..":",            shipDef.equipSlotCapacity.missile},
						{ lcore.ATMOSPHERIC_SHIELDING..":", yes_no(shipDef.equipSlotCapacity.atmo_shield)},
						{ lcore.FUEL_SCOOP..":",            yes_no(shipDef.equipSlotCapacity.fuel_scoop)},
						{ lcore.CARGO_SCOOP..":",           yes_no(shipDef.equipSlotCapacity.cargo_scoop)},
					}),
					"",
					ui:Label(l.EQUIPMENT):SetFont("HEADING_LARGE"),
					ui:Table():AddRows(equipItems),
				})
			})
			:SetColumn(2, {
				ui:VBox(10)
					:PackEnd(ui:Label(shipDef.name):SetFont("HEADING_LARGE"))
					:PackEnd(ModelSpinner.New(ui, shipDef.modelName, Game.player:GetSkin()))
			})
end

return shipInfo
