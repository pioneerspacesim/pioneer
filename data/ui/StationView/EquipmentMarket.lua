-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
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
	local stationTable, shipTable = EquipmentTableWidgets.Pair({
		stationColumns = { "name", "price", "mass", "stock" },
		shipColumns = { "name", "amount", "mass", "massTotal" },

		canTrade = function (e) return EquipDef[e].purchasable and EquipDef[e].slot ~= "CARGO" end,
	})

	return
		ui:Grid({48,4,48},1)
			:SetColumn(0, {
				ui:VBox():PackEnd({
					ui:Label(l.AVAILABLE_FOR_PURCHASE):SetFont("HEADING_LARGE"),
					ui:Expand():SetInnerWidget(stationTable),
				})
			})
			:SetColumn(2, {
				ui:VBox():PackEnd({
					ui:Label(l.EQUIPPED):SetFont("HEADING_LARGE"),
					ui:Expand():SetInnerWidget(shipTable),
				})
			})
end

return equipmentMarket
