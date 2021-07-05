-- Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Equipment = require 'Equipment'
local Event = require 'Event'
local Game = require 'Game'
local Lang = require 'Lang'
local ShipDef = require 'ShipDef'
local ModelSpinner = require 'PiGui.Modules.ModelSpinner'
local InfoView = require 'pigui.views.info-view'
local Vector2 = _G.Vector2

local ui = require 'pigui'
local l = Lang.GetResource("ui-core")

local fonts = ui.fonts

local textTable = require 'pigui.libs.text-table'

-- use the old InfoView style layout instead of the new sidebar layout.
local _OLD_LAYOUT = true

local modelSpinner

local function resetModelSpinner()
	modelSpinner = ModelSpinner()
	local player = Game.player
	local shipDef = ShipDef[player.shipId]
	modelSpinner:setModel(shipDef.modelName, player:GetSkin(), player.model.pattern)
end

local function shipSpinner()
	if not modelSpinner then resetModelSpinner() end
	local spinnerWidth = ui.getColumnWidth()
	modelSpinner:setSize(Vector2(spinnerWidth, spinnerWidth / 1.5))

	local player = Game.player
	local shipDef = ShipDef[player.shipId]
	ui.group(function ()
		local font = ui.fonts.orbiteer.large
		ui.withFont(font.name, font.size, function()
			ui.alignTextToFramePadding()
			ui.text(shipDef.name)
			ui.sameLine()
			ui.pushItemWidth(-1.0)
			local entry, apply = ui.inputText("##ShipNameEntry", player.shipName, ui.InputTextFlags {"EnterReturnsTrue"})
			ui.popItemWidth()

			if (apply) then player:SetShipName(entry) end
		end)

		modelSpinner:draw()
	end)
end

local collapsingHeaderFlags = ui.TreeNodeFlags { "DefaultOpen" }

local function shipStats()
	local closed = ui.withFont(fonts.pionillium.medlarge, function()
		return not ui.collapsingHeader(l.SHIP_INFORMATION, collapsingHeaderFlags)
	end)

	-- TODO: draw info ontop of the header

	if closed then return end

	local player = Game.player

	-- Taken directly from ShipInfo.lua.
	local shipDef     =    ShipDef[player.shipId]
	local shipLabel   =    player:GetLabel()
	local hyperdrive  =    table.unpack(player:GetEquip("engine"))
	local frontWeapon =    table.unpack(player:GetEquip("laser_front"))
	local rearWeapon  =    table.unpack(player:GetEquip("laser_rear"))

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

	textTable.draw {
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
		{ l.MINIMUM_CREW..":", shipDef.minCrew },
		{ l.CREW_CABINS..":",  shipDef.maxCrew },
		false,
		{ l.MISSILE_MOUNTS..":",            shipDef.equipSlotCapacity.missile},
		{ l.SCOOP_MOUNTS..":",              shipDef.equipSlotCapacity.scoop},
		false,
		{ l.ATMOSPHERIC_SHIELDING..":",     shipDef.equipSlotCapacity.atmo_shield > 0 and l.YES or l.NO },
		{ l.ATMO_PRESS_LIMIT..":", string.format("%d atm", shipDef.atmosphericPressureLimit * atmo_shield_cap) },
	}
end

local function equipmentList()
	local closed = ui.withFont(fonts.pionillium.medlarge, function()
		return not ui.collapsingHeader(l.EQUIPMENT, collapsingHeaderFlags)
	end)

	if closed then return end

	-- TODO: there's definitely a better way to do this, but it's copied from ShipInfo.lua for now.
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
							ui.text(string.interp(l.N_SHIELD_GENERATORS, { quantity = string.format("%d", count) }))
						elseif et == Equipment.misc.cabin_occupied then
							ui.text(string.interp(l.N_OCCUPIED_PASSENGER_CABINS, { quantity = string.format("%d", count) }))
						elseif et == Equipment.misc.cabin then
							ui.text(string.interp(l.N_UNOCCUPIED_PASSENGER_CABINS, { quantity = string.format("%d", count) }))
						else
							ui.text(et:GetName())
						end
					else
						ui.text(et:GetName())
					end
				end
			end
		end
	end
end

InfoView:registerView({
	id = "shipInfo",
	name = l.SHIP_INFORMATION,
	icon = ui.theme.icons.info,
	showView = true,
	draw = function()
		ui.withFont(fonts.pionillium.medlarge, function()
			if _OLD_LAYOUT then
				ui.columns(2, "shipInfo")

				shipStats()
				equipmentList()
				ui.nextColumn()

				shipSpinner()
				ui.columns(1, "")
			else
				shipSpinner()
				shipStats()
				equipmentList()
			end
		end)
	end,
	refresh = function() end,
	debugReload = function() package.reimport() end
})

Event.Register("onGameStart", resetModelSpinner)
Event.Register("onShipTypeChanged", function(ship)
	if ship == Game.player then resetModelSpinner() end
end)
