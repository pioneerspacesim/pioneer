-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Lang = import("Lang")
local Game = import("Game")
local EquipDef = import("EquipDef")
local ShipDef = import("ShipDef")

local ModelSpinner = import("UI.Game.ModelSpinner")

local ui = Engine.ui
local l = Lang.GetResource("ui-core");

local shipInfo = function (args)
	local shipDef = ShipDef[Game.player.shipId]

	local hyperdrive =              table.unpack(Game.player:GetEquip("ENGINE"))
	local frontWeapon, rearWeapon = table.unpack(Game.player:GetEquip("LASER"))

	hyperdrive =  hyperdrive  or "NONE"
	frontWeapon = frontWeapon or "NONE"
	rearWeapon =  rearWeapon  or "NONE"

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
	for i = 1,#Constants.EquipType do
		local type = Constants.EquipType[i]
		local et = EquipDef[type]
		local slot = et.slot
		if (slot ~= "CARGO" and slot ~= "MISSILE" and slot ~= "ENGINE" and slot ~= "LASER") then
			local count = Game.player:GetEquipCount(slot, type)
			if count > 0 then
				if count > 1 then
					if type == "SHIELD_GENERATOR" then
						table.insert(equipItems,
							ui:Label(string.interp(l.N_SHIELD_GENERATORS, { quantity = string.format("%d", count) })))
					elseif type == "PASSENGER_CABIN" then
						table.insert(equipItems,
							ui:Label(string.interp(l.N_OCCUPIED_PASSENGER_CABINS, { quantity = string.format("%d", count) })))
					elseif type == "UNOCCUPIED_CABIN" then
						table.insert(equipItems,
							ui:Label(string.interp(l.N_UNOCCUPIED_PASSENGER_CABINS, { quantity = string.format("%d", count) })))
					else
						table.insert(equipItems, ui:Label(et.name))
					end
				else
					table.insert(equipItems, ui:Label(et.name))
				end
			end
		end
	end

	return
		ui:Grid({48,4,48},1)
			:SetColumn(0, {
				ui:Table():AddRows({
					ui:Table():SetColumnSpacing(10):AddRows({
						{ l.HYPERDRIVE..":", EquipDef[hyperdrive].name },
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
						{ l.FUEL_WEIGHT..":",   string.format("%dt (%dt "..l.MAX..")", player.fuelMassLeft, ShipDef[Game.player.shipId].fuelTankMass ) },
						{ l.ALL_UP_WEIGHT..":", string.format("%dt", mass_with_fuel ) },
						"",
						{ l.FRONT_WEAPON..":", EquipDef[frontWeapon].name },
						{ l.REAR_WEAPON..":",  EquipDef[rearWeapon].name },
						{ l.FUEL..":",         string.format("%d%%", Game.player.fuel)},
						{ l.DELTA_V..":",      string.format("%d km/s", deltav / 1000)},
						"",
						{ l.FORWARD_ACCEL..":",  string.format("%.2f m/s² (%.1f G)", fwd_acc, fwd_acc / 9.81) },
						{ l.BACKWARD_ACCEL..":", string.format("%.2f m/s² (%.1f G)", bwd_acc, bwd_acc / 9.81) },
						{ l.UP_ACCEL..":",       string.format("%.2f m/s² (%.1f G)", up_acc, up_acc / 9.81) },
						"",
						{ l.MINIMUM_CREW..":", ShipDef[Game.player.shipId].minCrew },
						{ l.CREW_CABINS..":",  ShipDef[Game.player.shipId].maxCrew },
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
