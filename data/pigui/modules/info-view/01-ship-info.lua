-- Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = import 'pigui/pigui.lua'
local InfoView = import 'pigui/views/info-view'
local ModelSpinner = import 'PiGui.Modules.ModelSpinner'
local Lang = import 'Lang'
local Game = import 'Game'
local ShipDef = import 'ShipDef'
local Equipment = import 'Equipment'
local Vector2 = _G.Vector2

local l = Lang.GetResource("ui-core")
local lec = import 'Lang'.GetResource("equipment-core")

local fonts = ui.fonts

local drawTable = import 'pigui.libs.table'

-- use the old InfoView style layout instead of the new sidebar layout.
local _OLD_LAYOUT = true

local modelSpinner = ModelSpinner()
local cachedShip = nil
local cachedPattern = nil

local function shipSpinner()
	local spinnerWidth = _OLD_LAYOUT and ui.getColumnWidth() or ui.getContentRegion().x
	modelSpinner:setSize(Vector2(spinnerWidth, spinnerWidth / 1.5))

	local player = Game.player
	local shipDef = ShipDef[player.shipId]

	if shipDef.modelName ~= cachedShip or player.model.pattern ~= cachedPattern then
		cachedShip = shipDef.modelName
		cachedPattern = player.model.pattern
		modelSpinner:setModel(cachedShip, player:GetSkin(), cachedPattern)
	end

	ui.group(function ()
		local font = ui.fonts.orbiteer.large
		ui.withFont(font.name, font.size, function()
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
	local closed = ui.withFont(fonts.pionillium.medium, function()
		return not ui.collapsingHeader("Ship Information", collapsingHeaderFlags)
	end)

	-- TODO: draw info ontop of the header

	if closed then return end

	local player = Game.player

	-- Taken directly from ShipInfo.lua.
	local shipDef     =    ShipDef[player.shipId]
	local shipLabel   =    player:GetLabel()
	local hyperdrive  =    table.unpack(player:GetEquip("engine"))

	hyperdrive =  hyperdrive  or nil

	local mass_with_fuel = player.staticMass + player.fuelMassLeft

	local fwd_acc = player:GetAcceleration("forward")
	local bwd_acc = player:GetAcceleration("reverse")
	local up_acc = player:GetAcceleration("up")

	local accum = {
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
		{ l.FUEL..":",         string.format("%d%%", Game.player.fuel)},
		{ l.DELTA_V..":",      string.format("%d km/s", player:GetRemainingDeltaV() / 1000)},
		false,
		{ l.FORWARD_ACCEL..":",  string.format("%.2f m/s² (%.1f g)", fwd_acc, fwd_acc / 9.81) },
		{ l.BACKWARD_ACCEL..":", string.format("%.2f m/s² (%.1f g)", bwd_acc, bwd_acc / 9.81) },
		{ l.UP_ACCEL..":",       string.format("%.2f m/s² (%.1f g)", up_acc, up_acc / 9.81) },
		false,
		{ l.MINIMUM_CREW..":", shipDef.minCrew },
		{ l.CREW_CABINS..":",  shipDef.maxCrew },
		false,
		{ l.FRONT_INT_GUN_MOUNTS..":", shipDef.equipSlotCapacity.laser_front},
	}

	for i=1,(Game.player:GetUsedMountsNumber() + Game.player:GetFreeMountsNumber()) do
		if Game.player:MountIsFront(i - 1) then
			table.insert(accum, 
				{"",  i..": "..Game.player:MountBarrelNum(i - 1).." "..l.BARRELS}
			)
		end
	end

	table.insert(accum, false)
	table.insert(accum, { l.REAR_INT_GUN_MOUNTS..":", shipDef.equipSlotCapacity.laser_rear})

	for i=1,(Game.player:GetUsedMountsNumber() + Game.player:GetFreeMountsNumber()) do
		if Game.player:MountIsFront(i - 1) == false then
			table.insert(accum,
				{"", i..": "..Game.player:MountBarrelNum(i - 1).." "..l.BARRELS}
			)
		end
	end

	table.insert(accum, false)
	table.insert(accum, { l.MISSILE_MOUNTS..":",            shipDef.equipSlotCapacity.missile})
	table.insert(accum,	
		{ l.ATMOSPHERIC_SHIELDING..":",     shipDef.equipSlotCapacity.atmo_shield > 0 and l.YES or l.NO }
	)
	table.insert(accum, { l.SCOOP_MOUNTS..":",              shipDef.equipSlotCapacity.scoop})

	drawTable.draw(accum)

end

local current_mount = nil

local function equipmentList()
	local closed = ui.withFont(fonts.pionillium.medium, function()
		return not ui.collapsingHeader("Equipment", collapsingHeaderFlags)
	end)

	if closed then return end

	local player = Game.player

	-- TODO: there's definitely a better way to do this, but it's copied from ShipInfo.lua for now.
	local equipItems = {}
	local equips = {Equipment.cargo, Equipment.misc, Equipment.hyperspace, Equipment.laser}
	for _,t in pairs(equips) do
		for k,et in pairs(t) do
			local slot = et:GetDefaultSlot(Game.player)
			if (slot ~= "cargo" and slot ~= "missile" and slot ~= "engine" and slot ~= "laser_front" and slot ~= "laser_rear") then
				local count = player:CountEquip(et)
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

	for i=1,(Game.player:GetUsedMountsNumber() + Game.player:GetFreeMountsNumber()) do
		local gunId = player:GetGunOnMount(i - 1)
		if gunId >= 0 then
			ui.text(lec[player:GetGunName(gunId)]..", "..l.BARRELS..": "..player:GetNumBarrels(gunId).." ("..l.MOUNTED_ON..": "..i..")")
			if ui.isItemHovered() and ui.isMouseClicked(0) then
				current_mount = i
				print("OpenPopup for 'current_mount'= "..current_mount)
				ui.openPopup("##select_mount")
			end
		end
	end
	ui.popup("##select_mount", function()
		ui.text("Move to mount:")
		for j=1,(Game.player:GetUsedMountsNumber() + Game.player:GetFreeMountsNumber()) do
			if current_mount ~= j and ui.selectable(j, true, {}) then
				local ret = Game.player:SwapGuns((current_mount - 1), (j - 1))
			end
		end
	end)
end

InfoView:registerView({
	id = "shipInfo",
	name = l.SHIP_INFORMATION,
	icon = ui.theme.icons.info,
	showView = true,
	draw = function()
		ui.withStyleVars({ WindowPadding = Vector2(24, 24) }, function()
			ui.child("", Vector2(0, 0), ui.WindowFlags {"AlwaysUseWindowPadding"}, function()
				ui.withFont(fonts.pionillium.medium, function()
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
			end)
		end)
	end,
	refresh = function() end,
})
