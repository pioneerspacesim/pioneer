-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Equipment = require 'Equipment'
local Event = require 'Event'
local Game = require 'Game'
local Lang = require 'Lang'
local ShipDef = require 'ShipDef'
local InfoView = require 'pigui.views.info-view'
local Vector2 = Vector2

local ui = require 'pigui'
local l = Lang.GetResource("ui-core")

local fonts = ui.fonts
local textTable = require 'pigui.libs.text-table'

local equipmentWidget = require 'pigui.libs.ship-equipment'.New("ShipInfo")

local function shipStats()
	local player = Game.player

	-- Taken directly from ShipInfo.lua.
	local shipDef       =  ShipDef[player.shipId]
	local shipLabel     =  player:GetLabel()
	local hyperdrive    =  table.unpack(player:GetEquip("engine"))
	local frontWeapon   =  table.unpack(player:GetEquip("laser_front"))
	local rearWeapon    =  table.unpack(player:GetEquip("laser_rear"))
	local cabinEmpty    =  player:CountEquip(Equipment.misc.cabin, "cabin")
	local cabinOccupied =  player:CountEquip(Equipment.misc.cabin_occupied, "cabin")

	hyperdrive =  hyperdrive  or nil
	frontWeapon = frontWeapon or nil
	rearWeapon =  rearWeapon  or nil

	local mass_with_fuel = player.staticMass + player.fuelMassLeft

	local fwd_acc = player:GetAcceleration("forward")
	local bwd_acc = player:GetAcceleration("reverse")
	local up_acc = player:GetAcceleration("up")

	local atmo_shield = table.unpack(player:GetEquip("atmo_shield")) or nil
	local atmo_shield_cap = 1
	if atmo_shield then
		atmo_shield_cap = atmo_shield.capabilities.atmo_shield
	end

	textTable.draw({
		{ l.REGISTRATION_NUMBER..":",	shipLabel},
		{ l.HYPERDRIVE..":",			hyperdrive and hyperdrive:GetName() or l.NONE },
		{
			l.HYPERSPACE_RANGE..":",
			string.interp( l.N_LIGHT_YEARS_N_MAX, {
				range    = string.format("%.1f",player:GetHyperspaceRange()),
				maxRange = string.format("%.1f",player.maxHyperspaceRange)
			})
		},
		false,
		{ l.WEIGHT_EMPTY..":",  string.format("%dt", player.staticMass - player.usedCapacity) },
		{ l.CAPACITY_USED..":", string.format("%dt (%dt "..l.FREE..")", player.usedCapacity,  player.freeCapacity) },
		{ l.CARGO_SPACE..":", string.format("%dt (%dt "..l.MAX..")", player.totalCargo, shipDef.equipSlotCapacity.cargo) },
		{ l.CARGO_SPACE_USED..":", string.format("%dt (%dt "..l.FREE..")", player.usedCargo, player.totalCargo - player.usedCargo) },
		{ l.FUEL_WEIGHT..":",   string.format("%dt (%dt "..l.MAX..")", player.fuelMassLeft, shipDef.fuelTankMass ) },
		{ l.ALL_UP_WEIGHT..":", string.format("%dt", mass_with_fuel ) },
		false,
		{ l.FRONT_WEAPON..":", frontWeapon and frontWeapon:GetName() or l.NONE },
		{ l.REAR_WEAPON..":",  rearWeapon and rearWeapon:GetName() or l.NONE },
		{ l.FUEL..":",         string.format("%d%%", player.fuel) },
		{ l.DELTA_V..":",      string.format("%d km/s", player:GetRemainingDeltaV() / 1000) },
		false,
		{ l.FORWARD_ACCEL..":",  string.format("%.2f m/s² (%.1f g)", fwd_acc, fwd_acc / 9.81) },
		{ l.BACKWARD_ACCEL..":", string.format("%.2f m/s² (%.1f g)", bwd_acc, bwd_acc / 9.81) },
		{ l.UP_ACCEL..":",       string.format("%.2f m/s² (%.1f g)", up_acc, up_acc / 9.81) },
		false,
		{ l.MINIMUM_CREW..":",                shipDef.minCrew },
		{ l.CREW_CABINS..":",                 shipDef.maxCrew },
		{ l.UNOCCUPIED_PASSENGER_CABINS..":", cabinEmpty },
		{ l.OCCUPIED_PASSENGER_CABINS..":",   cabinOccupied },
		{ l.PASSENGER_CABIN_CAPACITY..":",    shipDef.equipSlotCapacity.cabin},
		false,
		{ l.MISSILE_MOUNTS..":",            shipDef.equipSlotCapacity.missile},
		{ l.SCOOP_MOUNTS..":",              shipDef.equipSlotCapacity.scoop},
		false,
		{ l.ATMOSPHERIC_SHIELDING..":",     shipDef.equipSlotCapacity.atmo_shield > 0 and l.YES or l.NO },
		{ l.ATMO_PRESS_LIMIT..":", string.format("%d atm", shipDef.atmosphericPressureLimit * atmo_shield_cap) },
	})
end

table.insert(equipmentWidget.tabs, 1, {
	name = l.SHIP_INFORMATION,
	draw = function()
		ui.withStyleVars({ ItemSpacing = Vector2(8, 6) }, function()
			shipStats()
		end)
	end
})

InfoView:registerView({
	id = "shipInfo",
	name = l.SHIP_INFORMATION,
	icon = ui.theme.icons.info,
	showView = true,
	draw = function()
		equipmentWidget:draw()
	end,
	refresh = function()
		equipmentWidget.ship = Game.player
		equipmentWidget.showShipNameEdit = true
		equipmentWidget.showEmptySlots = false
		equipmentWidget:refresh()
	end,
	debugReload = function()
		equipmentWidget:debugReload()
		package.reimport()
	end
})

Event.Register("onShipTypeChanged", function(ship)
	if ship == Game.player then
		equipmentWidget.ship = Game.player
		equipmentWidget:refresh()
	end
end)
