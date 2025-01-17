-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Event = require 'Event'
local Game = require 'Game'
local Lang = require 'Lang'
local ShipDef = require 'ShipDef'
local InfoView = require 'pigui.views.info-view'
local Passengers = require 'Passengers'
local Vector2 = Vector2

local ui = require 'pigui'
local l = Lang.GetResource("ui-core")

local textTable = require 'pigui.libs.text-table'

local equipmentWidget = require 'pigui.libs.ship-equipment'.New("ShipInfo")

local function shipStats()
	local player = Game.player
	local equipSet = player:GetComponent("EquipSet")

	-- Taken directly from ShipInfo.lua.
	local shipDef       =  ShipDef[player.shipId]
	local shipLabel     =  player:GetLabel()
	local hyperdrive    =  equipSet:GetInstalledOfType("hyperdrive")[1]
	local frontWeapon   =  equipSet:GetInstalledOfType("weapon")[1]
	local rearWeapon    =  equipSet:GetInstalledOfType("weapon")[2]
	local cabinEmpty    =  Passengers.CountFreeBerths(player)
	local cabinOccupied =  Passengers.CountOccupiedBerths(player)
	local cabinMaximum  =  cabinEmpty + cabinOccupied

	hyperdrive =  hyperdrive  or nil
	frontWeapon = frontWeapon or nil
	rearWeapon =  rearWeapon  or nil

	local mass_with_fuel = player.staticMass + player.fuelMassLeft

	local fwd_acc = player:GetAcceleration("forward")
	local bwd_acc = player:GetAcceleration("reverse")
	local up_acc = player:GetAcceleration("up")

	local atmo_shield = equipSet:GetInstalledOfType("hull.atmo_shield")[1]
	local atmo_shield_cap = player["atmo_shield_cap"] or 1

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
		{ l.WEIGHT_EMPTY..":",     string.format("%dt", player.staticMass - player.loadedMass) },
		{ l.CAPACITY_USED..":",    string.format("%s (%s "..l.MAX..")", ui.Format.Volume(player.equipVolume), ui.Format.Volume(player.totalVolume) ) },
		{ l.CARGO_SPACE_USED..":", string.format("%dcu (%dcu "..l.MAX..")", player.usedCargo, player.totalCargo) },
		{ l.FUEL_WEIGHT..":",      string.format("%.1ft (%.1ft "..l.MAX..")", player.fuelMassLeft, shipDef.fuelTankMass ) },
		{ l.ALL_UP_WEIGHT..":",    string.format("%dt", mass_with_fuel ) },
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
		{ l.PASSENGER_CABIN_CAPACITY..":",    cabinMaximum },
		false,
		{ l.ATMOSPHERIC_SHIELDING..":", atmo_shield and l.YES or l.NO },
		{ l.ATMO_PRESS_LIMIT..":",      string.format("%d atm", shipDef.atmosphericPressureLimit * atmo_shield_cap) },
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
