-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Translate = import("Translate")
local Engine = import("Engine")
local Game = import("Game")
local ShipType = import("ShipType")
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
						{ t("HYPERDRIVE")..":", EquipDef[hyperdrive].name },
						{
							t("HYPERSPACE_RANGE")..":",
							string.interp(
								t("{range} light years ({maxRange} max)"), {
									range    = string.format("%.1f",stats.hyperspaceRange),
									maxRange = string.format("%.1f",stats.maxHyperspaceRange)
								}
							),
						},
						"",
						{ t("Weight empty:"),      string.format("%dt", stats.totalMass - stats.usedCapacity) },
						{ t("CAPACITY_USED")..":", string.format("%dt (%dt "..t("free")..")", stats.usedCapacity,  stats.freeCapacity) },
						{ t("FUEL_WEIGHT")..":",   string.format("%dt (%dt "..t("max")..")", math.floor(Game.player.fuel/100*stats.maxFuelTankMass + 0.5), stats.maxFuelTankMass ) },
						{ t("TOTAL_WEIGHT")..":",  string.format("%dt", math.floor(stats.totalMass+Game.player.fuel/100*stats.maxFuelTankMass + 0.5) ) },
						"",
						{ t("FRONT_WEAPON")..":", EquipDef[frontWeapon].name },
						{ t("REAR_WEAPON")..":",  EquipDef[rearWeapon].name },
						{ t("FUEL")..":",         string.format("%d%%", Game.player.fuel) },
						"",
						{ t("Minimum crew")..":", ShipType.GetShipType(Game.player.shipId).minCrew },
						{ t("Crew cabins")..":",  ShipType.GetShipType(Game.player.shipId).maxCrew },
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
