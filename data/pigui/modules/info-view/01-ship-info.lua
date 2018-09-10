-- Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = import 'pigui/pigui.lua'
local InfoView = import 'pigui/views/info-view'
local Lang = import 'Lang'
local Game = import 'Game'
local Vector = import 'Vector'
local ShipDef = import 'ShipDef'
local Equipment = import 'Equipment'

local pionillium = ui.fonts.pionillium
local orbiteer = ui.fonts.orbiteer

local drawTable = import 'pigui/libs/table.lua'

local l = Lang.GetResource("ui-core")

local function drawShipEquipment()
	ui.withFont(orbiteer.large.name, orbiteer.large.size, function()
		ui.text(l.EQUIPMENT)
	end)
	ui.text("")
	local equips = {Equipment.cargo, Equipment.misc, Equipment.hyperspace, Equipment.laser}
	for _,t in pairs(equips) do
		for k,et in pairs(t) do
			local slot = et:GetDefaultSlot(Game.player)
			if (slot ~= "cargo" and slot ~= "missile" and slot ~= "engine" and slot ~= "laser_front" and slot ~= "laser_rear") then
				local count = Game.player:CountEquip(et)
				if count == 1 then
					ui.text(et:GetName())
				elseif count > 1 then
					if et == Equipment.misc.shield_generator then
						ui.text(string.interp(l.N_SHIELD_GENERATORS, { quantity = string.format("%d", count) }))
					elseif et == Equipment.misc.cabin_occupied then
						ui.text(string.interp(l.N_OCCUPIED_PASSENGER_CABINS, { quantity = string.format("%d", count) }))
					elseif et == Equipment.misc.cabin then
						ui.text(string.interp(l.N_UNOCCUPIED_PASSENGER_CABINS, { quantity = string.format("%d", count) }))
					else
						ui.text(et:GetName())
					end
				end
			end
		end
	end
end

local function drawShipInfo()
	local player = Game.player
	local shipDef = ShipDef[player.shipId]
	local hyperdrive = table.unpack(player:GetEquip("engine"))
	local frontWeapon = table.unpack(player:GetEquip("laser_front"))
	local rearWeapon = table.unpack(player:GetEquip("laser_rear"))

	local mass_with_fuel = player.staticMass + player.fuelMassLeft
	local mass_with_fuel_kg = 1000 * mass_with_fuel

	-- ship stats mass is in tonnes; scale by 1000 to convert to kg
	local fwd_acc = -shipDef.linearThrust.FORWARD / mass_with_fuel_kg
	local bwd_acc = shipDef.linearThrust.REVERSE / mass_with_fuel_kg
	local up_acc = shipDef.linearThrust.UP / mass_with_fuel_kg

	-- delta-v calculation according to http://en.wikipedia.org/wiki/Tsiolkovsky_rocket_equation
	local deltav = shipDef.effectiveExhaustVelocity * math.log((player.staticMass + player.fuelMassLeft) / player.staticMass)

	drawTable.withHeading(l.SHIP_INFORMATION, orbiteer.large, {
		2, "ship_info", false, {
			{ l.REGISTRATION_NUMBER .. ":", Game.player:GetLabel() },
			{ l.HYPERDRIVE..":", hyperdrive and hyperdrive:GetName() or l.NONE },
			{
				l.HYPERSPACE_RANGE..":",
				string.interp(l.N_LIGHT_YEARS_N_MAX, {
					range    = string.format("%.1f",player.hyperspaceRange),
					maxRange = string.format("%.1f",player.maxHyperspaceRange)
				})
			},
			{ "", "" },
			{ l.WEIGHT_EMPTY..":", string.format("%dt", player.staticMass - player.usedCapacity) },
			{ l.CAPACITY_USED..":", string.format("%dt (%dt "..l.FREE..")", player.usedCapacity,  player.freeCapacity) },
			{ l.CARGO_SPACE..":", string.format("%dt (%dt "..l.MAX..")", player.totalCargo, shipDef.equipSlotCapacity.cargo) },
			{ l.CARGO_SPACE_USED..":", string.format("%dt (%dt "..l.FREE..")", player.usedCargo, player.totalCargo - player.usedCargo) },
			{ l.FUEL_WEIGHT..":", string.format("%dt (%dt "..l.MAX..")", player.fuelMassLeft, shipDef.fuelTankMass ) },
			{ l.ALL_UP_WEIGHT..":", string.format("%dt", mass_with_fuel ) },
			{ "", "" },
			{ l.FRONT_WEAPON..":", frontWeapon and frontWeapon:GetName() or l.NONE },
			{ l.REAR_WEAPON..":",  rearWeapon and rearWeapon:GetName() or l.NONE },
			{ l.FUEL..":",         string.format("%d%%", Game.player.fuel)},
			{ l.DELTA_V..":",      string.format("%d km/s", deltav / 1000)},
			{ "", "" },
			{ l.FORWARD_ACCEL..":",  string.format("%.2f m/s² (%.1f G)", fwd_acc, fwd_acc / 9.81) },
			{ l.BACKWARD_ACCEL..":", string.format("%.2f m/s² (%.1f G)", bwd_acc, bwd_acc / 9.81) },
			{ l.UP_ACCEL..":",       string.format("%.2f m/s² (%.1f G)", up_acc, up_acc / 9.81) },
			{ "", "" },
			{ l.MINIMUM_CREW..":", shipDef.minCrew },
			{ l.CREW_CABINS..":",  shipDef.maxCrew },
			{ "", "" },
			{ l.MISSILE_MOUNTS..":",            shipDef.equipSlotCapacity.missile},
			{ l.ATMOSPHERIC_SHIELDING..":", (shipDef.equipSlotCapacity.atmo_shield > 0 and l.YES or l.NO)},
			{ l.SCOOP_MOUNTS..":",              shipDef.equipSlotCapacity.scoop},
		}
	})
end

local function drawShipView()
	ui.text("This is a test.")
end

InfoView.registerView("shipInfo", {
	name = l.SHIP_INFORMATION,
	icon = ui.theme.icons.ship,
	draw = function()
		local info_column_width = 400 * ui.screenWidth / 1200
		local equip_column_width = 300 * ui.screenWidth / 1200

		ui.withStyleVars({WindowPadding = Vector(10,10)}, function()
			ui.withFont(pionillium.medlarge.name, pionillium.medlarge.size, function()
				ui.child("ShipInfoDetails", Vector(info_column_width, 0), drawShipInfo)
				ui.sameLine()
				ui.child("ShipEquipDetails", Vector(equip_column_width, 0), drawShipEquipment)
				ui.sameLine()
				ui.child("ShipView", Vector(0, 0), drawShipView)
			end)
		end)
	end
})
