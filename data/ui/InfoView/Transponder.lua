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
	
	local shipLabelEntry = ui:TextEntry(player.fakeLabel):SetFont("HEADING_SMALL")
	shipLabelEntry.onChange:Connect(function (newFakeLabel)
		player:SetFakeTransponderLabel(newFakeLabel)
	end )

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
					ui:Label(player.label):SetFont("HEADING_LARGE"),
					ui:Table():SetColumnSpacing(10):AddRows({
						{ l.HYPERDRIVE..":", hyperdrive and hyperdrive:GetName() or "None" },
						"",
						{ l.FRONT_WEAPON..":", frontWeapon and frontWeapon:GetName() or "None"},
						{ l.REAR_WEAPON..":",  rearWeapon and rearWeapon:GetName() or "None" },
					}),
					"",
					ui:Label(l.EQUIPMENT):SetFont("HEADING_LARGE"),
					ui:Table():AddRows(equipItems),
				})
			})
			:SetColumn(2, {
				ui:VBox(10)
					:PackEnd(ui:HBox(10):PackEnd({
						ui:VBox(5):PackEnd({
							ui:Label(shipDef.name):SetFont("HEADING_LARGE"),
						}),
						ui:VBox(5):PackEnd({
							ui:Expand("HORIZONTAL", shipLabelEntry),
						})
					}))
					:PackEnd(ModelSpinner.New(ui, shipDef.modelName, Game.player:GetSkin()))
			})
end

return shipInfo
