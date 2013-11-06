-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Lang = import("Lang")
local Game = import("Game")
local EquipDef = import("EquipDef")
local ShipDef = import("ShipDef")
local Comms = import("Comms")

local InfoGauge = import("ui/InfoGauge")

local ui = Engine.ui

local l = Lang.GetResource("ui-core")
-- XXX equipment strings are in core. this sucks
local lcore = Lang.GetResource("core")

local cargoIcon = {
	HYDROGEN =              "Hydrogen",
	LIQUID_OXYGEN =         "Liquid_Oxygen",
	METAL_ORE =             "Metal_ore",
	CARBON_ORE =            "Carbon_ore",
	METAL_ALLOYS =          "Metal_alloys",
	PLASTICS =              "Plastics",
	FRUIT_AND_VEG =         "Fruit_and_Veg",
	ANIMAL_MEAT =           "Animal_Meat",
	LIVE_ANIMALS =          "Live_Animals",
	LIQUOR =                "Liquor",
	GRAIN =                 "Grain",
	TEXTILES =              "Textiles",
	FERTILIZER =            "Fertilizer",
	WATER =                 "Water",
	MEDICINES =             "Medicines",
	CONSUMER_GOODS =        "Consumer_goods",
	COMPUTERS =             "Computers",
	ROBOTS =                "Robots",
	PRECIOUS_METALS =       "Precious_metals",
	INDUSTRIAL_MACHINERY =  "Industrial_machinery",
	FARM_MACHINERY =        "Farm_machinery",
	MINING_MACHINERY =      "Mining_machinery",
	AIR_PROCESSORS =        "Air_processors",
	SLAVES =                "Slaves",
	HAND_WEAPONS =          "Hand_weapons",
	BATTLE_WEAPONS =        "Battle_weapons",
	NERVE_GAS =             "Nerve_Gas",
	NARCOTICS =             "Narcotics",
	MILITARY_FUEL =         "Military_fuel",
	RUBBISH =               "Rubbish",
	RADIOACTIVES =          "Radioactive_waste",
}

local commodityMarket = function (args)
	local player = Game.player
	local station = player:GetDockedWith()

	local cargoTypes = {}
	for k,v in pairs(EquipDef) do
		if v.slot == "CARGO" and k ~= "NONE" then
			table.insert(cargoTypes, k)
		end
	end
	table.sort(cargoTypes)

	local stationCargo =
		ui:Table()
			:SetRowSpacing(5)
			:SetColumnSpacing(10)
			:SetHeadingRow({ "", "Name", "Price", "In stock" })
			:SetHeadingFont("LARGE")
			:SetMouseEnabled(true)

	local rowEquip = {}
	local row = 1
	for i=1,#cargoTypes do
		local e = cargoTypes[i]
		local icon = cargoIcon[e] and ui:Image("icons/goods/"..cargoIcon[e]..".png") or ""
		stationCargo:AddRow({ icon, lcore[e], string.format("%.02f", station:GetEquipmentPrice(e)), station:GetEquipmentStock(e) })
		rowEquip[row] = e
		row = row + 1
	end

	local shipCargo =
		ui:Table()
			:SetRowSpacing(5)
			:SetColumnSpacing(10)
			:SetHeadingRow({ "", "Name", "Amount" })
			:SetHeadingFont("LARGE")

	local function fillShipCargo ()
		shipCargo:ClearRows()
		for i=1,#cargoTypes do
			local e = cargoTypes[i]
			local n = player:GetEquipCount("CARGO", e)
			if n > 0 then
				local icon = cargoIcon[e] and ui:Image("icons/goods/"..cargoIcon[e]..".png") or ""
				shipCargo:AddRow({ icon, lcore[e], n })
			end
		end
	end
	fillShipCargo()

	local cargoGauge = InfoGauge.New({
		formatter = function (v)
			return string.format("%d/%dt", player.usedCargo, ShipDef[player.shipId].capacity-player.usedCapacity+player.usedCargo)
		end
	})
	cargoGauge:SetValue(player.usedCargo/(ShipDef[player.shipId].capacity-player.usedCapacity+player.usedCargo))

	local cashLabel = ui:Label(string.format("$%.2f", player:GetMoney()))

	stationCargo.onRowClicked:Connect( function (row)
		local e = rowEquip[row+1]

		if station:GetEquipmentStock(e) <= 0 then
			Comms.Message(l.ITEM_IS_OUT_OF_STOCK)
			return
		end

		-- XXX check slot capacity

		if player.freeCapacity <= 0 then
			Comms.Message(l.SHIP_IS_FULLY_LADEN)
			return
		end

		if player:GetMoney() < station:GetEquipmentPrice(e) then
			Comms.Message(l.YOU_NOT_ENOUGH_MONEY)
			return
		end

		assert(player:AddEquip(e) == 1)
		player:AddMoney(-station:GetEquipmentPrice(e))

		fillShipCargo()

		cargoGauge:SetValue(player.usedCargo/(ShipDef[player.shipId].capacity-player.usedCapacity+player.usedCargo))
		cashLabel:SetText(string.format("$%.2f", player:GetMoney()))
	end)

	return
		ui:Grid(2,1)
			:SetColumn(0, {
				ui:VBox():PackEnd({
					ui:Label("Available for purchase"):SetFont("HEADING_LARGE"),
					ui:Expand():SetInnerWidget(stationCargo),
				})
			})
			:SetColumn(1, {
				ui:VBox():PackEnd({
					ui:Label("In cargo hold"):SetFont("HEADING_LARGE"),
					ui:Expand():SetInnerWidget(shipCargo),
					ui:HBox():PackEnd({
						"Cash: ",
						cashLabel
					}),
					ui:HBox():PackEnd({
						"Cargo space: ",
						cargoGauge
					})
				})
			})
end

return commodityMarket
