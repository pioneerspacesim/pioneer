-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Translate = import("Translate")
local Engine = import("Engine")
local Game = import("Game")
local EquipDef = import("EquipDef")
local ShipDef = import("ShipDef")

local ModelSpinner = import("UI.Game.ModelSpinner")

local ui = Engine.ui
local t = Translate:GetTranslator()

local shipInfo = function (args)
	local shipDef = ShipDef[Game.player.shipId]

	local hyperdrive =              table.unpack(Game.player:GetEquip("ENGINE"))
	local frontWeapon, rearWeapon = table.unpack(Game.player:GetEquip("LASER"))

	hyperdrive =  hyperdrive  or "NONE"
	frontWeapon = frontWeapon or "NONE"
	rearWeapon =  rearWeapon  or "NONE"

	local stats = Game.player:GetStats()

	local mass_with_fuel = stats.totalMass + stats.fuelMassLeft
	local mass_with_fuel_kg = 1000 * mass_with_fuel

	-- ship stats mass is in tonnes; scale by 1000 to convert to kg
	local fwd_acc = -shipDef.linearThrust.FORWARD / mass_with_fuel_kg
	local bwd_acc = shipDef.linearThrust.REVERSE / mass_with_fuel_kg
	local up_acc = shipDef.linearThrust.UP / mass_with_fuel_kg

	-- delta-v calculation according to http://en.wikipedia.org/wiki/Tsiolkovsky_rocket_equation
	local deltav = shipDef.effectiveExhaustVelocity * math.log((stats.totalMass + stats.fuelMassLeft) / stats.totalMass)

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
							ui:Label(string.interp(t("{quantity} Shield Generators"), { quantity = string.format("%d", count) })))
					elseif type == "PASSENGER_CABIN" then
						table.insert(equipItems,
							ui:Label(string.interp(t("{quantity} Occupied Passenger Cabins"), { quantity = string.format("%d", count) })))
					elseif type == "UNOCCUPIED_CABIN" then
						table.insert(equipItems,
							ui:Label(string.interp(t("{quantity} Unoccupied Passenger Cabins"), { quantity = string.format("%d", count) })))
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
		ui:Grid(2,1)
			:SetColumn(0, {
				ui:Table():AddRows({
					ui:Table():SetColumnSpacing(10):AddRows({
						{ t("Hyperdrive")..":", EquipDef[hyperdrive].name },
						{
							t("Hyperspace range")..":",
							string.interp(
								t("{range} light years ({maxRange} max)"), {
									range    = string.format("%.1f",stats.hyperspaceRange),
									maxRange = string.format("%.1f",stats.maxHyperspaceRange)
								}
							),
						},
						"",
						{ t("Weight empty:"),      string.format("%dt", stats.totalMass - stats.usedCapacity) },
						{ t("Capacity used")..":", string.format("%dt (%dt "..t("free")..")", stats.usedCapacity,  stats.freeCapacity) },
						{ t("FUEL_WEIGHT")..":",   string.format("%dt (%dt "..t("max")..")", stats.fuelMassLeft, stats.maxFuelTankMass ) },
						{ t("All-up weight")..":",  string.format("%dt", mass_with_fuel ) },
						"",
						{ t("Front weapon")..":", EquipDef[frontWeapon].name },
						{ t("Rear weapon")..":",  EquipDef[rearWeapon].name },
						{ t("FUEL")..":",         string.format("%d%%", Game.player.fuel)},
						{ t("deltaV")..":",       string.format("%d km/s", deltav / 1000)},
						"",
						{ t("Forward Acceleration")..":",  string.format("%.2f m/s² (%.1f G)", fwd_acc, fwd_acc / 9.81) },
						{ t("Backward Acceleration")..":", string.format("%.2f m/s² (%.1f G)", bwd_acc, bwd_acc / 9.81) },
						{ t("Up Acceleration")..":",       string.format("%.2f m/s² (%.1f G)", up_acc, up_acc / 9.81) },
						"",
						{ t("Minimum crew")..":", ShipDef[Game.player.shipId].minCrew },
						{ t("Crew cabins")..":",  ShipDef[Game.player.shipId].maxCrew },
					}),
					"",
					ui:Label(t("Equipment")):SetFont("HEADING_LARGE"),
					ui:Table():AddRows(equipItems),
				})
			})
			:SetColumn(1, {
				ui:VBox(10)
					:PackEnd(ui:Label(shipDef.name):SetFont("HEADING_LARGE"))
					:PackEnd(ModelSpinner.New(ui, shipDef.modelName, Game.player:GetSkin()))
			})
end

return shipInfo
