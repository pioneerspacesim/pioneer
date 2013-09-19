-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local SpaceStation = import("SpaceStation")
local Game = import("Game")
local Event = import("Event")
local Format = import("Format")

local ModelSpinner = import("UI.Game.ModelSpinner")
local ModelSkin = import("SceneGraph.ModelSkin")

local ui = Engine.ui

local shipTable =
	ui:Table()
		:SetRowSpacing(5)
		:SetColumnSpacing(10)
		:SetHeadingRow({"Ship","Price","Capacity"})
		:SetMouseEnabled(true)

local shipInfo =
	ui:Expand("VERTICAL")

shipTable.onRowClicked:Connect(function (row)
	local station = Game.player:GetDockedWith()
	local sos = SpaceStation.shipsOnSale[station][row+1]
	local def = sos.def

	shipInfo:SetInnerWidget(
		ui:VBox():PackEnd({
			ui:Label(def.name):SetFont("HEADING_LARGE"),
			ModelSpinner.New(ui, def.modelName, sos.skin),
			ui:Align("MIDDLE", ui:Button("Buy Ship"):SetFont("HEADING_LARGE")),
		})
	)
end)

local function updateStation (station, shipsOnSale)
	if station ~= Game.player:GetDockedWith() then return end

	shipTable:ClearRows()

	for i = 1,#shipsOnSale do
		local def = shipsOnSale[i].def
		shipTable:AddRow({def.name, Format.Money(def.basePrice), def.capacity.."t"})
	end
end

Event.Register("onShipMarketUpdate", updateStation)

local shipMarket = function (args)
	updateStation(station, SpaceStation.shipsOnSale[Game.player:GetDockedWith()])

	return
		ui:Grid(2,1)
			:SetColumn(0, {shipTable})
			:SetColumn(1, {shipInfo})
end

return shipMarket
