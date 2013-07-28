-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Game = import("Game")
local EquipDef = import("EquipDef")
local Lang = import("Lang")

local InfoGauge = import("ui/InfoGauge")

local ui = Engine.ui

-- XXX equipment strings are in core. this sucks
local l = Lang.GetResource("core")

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
	local station = Game.player:GetDockedWith()

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

	for i=1,#cargoTypes do
		local e = cargoTypes[i]
		local icon = cargoIcon[e] and ui:Image("icons/goods/"..cargoIcon[e]..".png") or ""
		stationCargo:AddRow({ icon, l[e], string.format("%.02f", station:GetEquipmentPrice(e)), station:GetEquipmentStock(e) })
	end

	local shipCargo =
		ui:Table()
			:SetRowSpacing(5)
			:SetColumnSpacing(10)
			:SetHeadingRow({ "", "Name", "Amount" })
			:SetHeadingFont("LARGE")

	for i=1,#cargoTypes do
		local e = cargoTypes[i]
		local n = Game.player:GetEquipCount("CARGO", e)
		if n > 0 then
			local icon = cargoIcon[e] and ui:Image("icons/goods/"..cargoIcon[e]..".png") or ""
			shipCargo:AddRow({ icon, l[e], n })
		end
	end

	local cargoGauge = InfoGauge.New({
		formatter = function (v)
			return string.format("%d/%dt", Game.player.usedCargo, Game.player.freeCapacity)
		end
	})
	cargoGauge:SetValue(Game.player.usedCargo/Game.player.freeCapacity)

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
						"Cargo space: ",
						cargoGauge
					})
				})
			})
end

return commodityMarket
