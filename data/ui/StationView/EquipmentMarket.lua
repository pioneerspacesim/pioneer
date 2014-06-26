-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Lang = import("Lang")
local Game = import("Game")
local ShipDef = import("ShipDef")
local Format = import("Format")

local EquipmentTableWidgets = import("EquipmentTableWidgets")

local l = Lang.GetResource("ui-core")

local ui = Engine.ui


local equipmentMarket = function (args)
	local stationTable, shipTable = EquipmentTableWidgets.Pair({
		stationColumns = { "name", "buy", "sell", "mass", "stock" },
		shipColumns = { "name", "amount", "mass", "massTotal" },

		canTrade = function (e) return e.purchasable and not e:IsValidSlot("cargo", Game.player) end,
		buy = function (e, funcs) return Format.Money(funcs.getBuyPrice(e),false) end,
		sell = function (e, funcs) return Format.Money(funcs.getSellPrice(e),false) end,
	})

	return
		ui:Grid({52,2,46},1)
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
