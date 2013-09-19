-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local SpaceStation = import("SpaceStation")
local Game = import("Game")
local Event = import("Event")

local ui = Engine.ui

local shipTable =
	ui:Table()
		:SetRowSpacing(5)
		:SetColumnSpacing(10)
		:SetHeadingRow("id")
		:SetMouseEnabled(true)

local function updateStation (station, shipsOnSale)
	if station ~= Game.player:GetDockedWith() then return end

	shipTable:ClearRows()

	for i = 1,#shipsOnSale do
		shipTable:AddRow({shipsOnSale[i].id})
	end
end

Event.Register("onShipMarketUpdate", updateStation)

local shipMarket = function (args)
	updateStation(station, SpaceStation.shipsOnSale[Game.player:GetDockedWith()])
	return shipTable
end

return shipMarket
