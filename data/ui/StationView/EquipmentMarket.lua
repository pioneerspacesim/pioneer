-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Lang = import("Lang")
local Game = import("Game")
local ShipDef = import("ShipDef")
local Comms = import("Comms")
local EquipDef = import("EquipDef")

local EquipmentTableWidgets = import("EquipmentTableWidgets")

local l = Lang.GetResource("ui-core")

local ui = Engine.ui


local equipmentMarket = function (args)
	local player = Game.player
	local station = player:GetDockedWith()

	local stationTable, shipTable = EquipmentTableWidgets.Pair({
		station = station,

		stationColumns = { "name", "price", "mass", "stock" },
		shipColumns = { "name", "amount", "mass", "massTotal" },

		isValidSlot = function (slot) return slot ~= "CARGO" end,

		onBuy = function (e)
			if station:GetEquipmentStock(e) <= 0 then
				Comms.message(l.ITEM_IS_OUT_OF_STOCK)
				return
			end

			if player:GetEquipFree(EquipDef[e].slot) < 1 then
				Comms.Message(l.SHIP_IS_FULLY_LADEN)
				return
			end

			if player.freeCapacity < EquipDef[e].mass then
				Comms.Message(l.SHIP_IS_FULLY_LADEN)
				return
			end

			if player:GetMoney() < station:GetEquipmentPrice(e) then
				Comms.Message(l.YOU_NOT_ENOUGH_MONEY)
				return
			end

			assert(player:AddEquip(e) == 1)
			player:AddMoney(-station:GetEquipmentPrice(e))
			station:AddEquipmentStock(e, -1)
		end,

		onSell = function (e)
			player:RemoveEquip(e)
			player:AddMoney(station:GetEquipmentPrice(e))
			station:AddEquipmentStock(e, 1)
		end,
	})

	return
		ui:Grid(2,1)
			:SetColumn(0, {
				ui:VBox():PackEnd({
					ui:Label("Available for purchase"):SetFont("HEADING_LARGE"),
					ui:Expand():SetInnerWidget(stationTable),
				})
			})
			:SetColumn(1, {
				ui:VBox():PackEnd({
					ui:Label("Equipped"):SetFont("HEADING_LARGE"),
					ui:Expand():SetInnerWidget(shipTable),
				})
			})
end

return equipmentMarket
