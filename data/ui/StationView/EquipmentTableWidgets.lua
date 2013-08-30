-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Lang = import("Lang")
local Game = import("Game")
local EquipDef = import("EquipDef")

-- XXX equipment strings are in core. this sucks
local lcore = Lang.GetResource("core")

local ui = Engine.ui

local equipIcon = {
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

local EquipmentTableWidgets = {}

function EquipmentTableWidgets.Pair (config)
	local equipTypes = {}
	for k,v in pairs(EquipDef) do
		if config.isValidSlot(v.slot) and k ~= "NONE" then
			table.insert(equipTypes, k)
		end
	end
	table.sort(equipTypes)

	local stationTable =
		ui:Table()
			:SetRowSpacing(5)
			:SetColumnSpacing(10)
			:SetHeadingRow({ "", "Name", "Price", "In stock" })
			:SetHeadingFont("LARGE")
			:SetMouseEnabled(true)

	local function fillStationTable ()
		stationTable:ClearRows()

		local rowEquip = {}
		local row = 1
		for i=1,#equipTypes do
			local e = equipTypes[i]
			local icon = equipIcon[e] and ui:Image("icons/goods/"..equipIcon[e]..".png") or ""

			stationTable:AddRow({ icon, lcore[e], string.format("%.02f", config.station:GetEquipmentPrice(e)), config.station:GetEquipmentStock(e) })
			rowEquip[row] = e
			row = row + 1
		end

		return rowEquip
	end
	local stationRowEquip = fillStationTable()

	local shipTable =
		ui:Table()
			:SetRowSpacing(5)
			:SetColumnSpacing(10)
			:SetHeadingRow({ "", "Name", "Amount" })
			:SetHeadingFont("LARGE")
			:SetMouseEnabled(true)

	local function fillShipTable ()
		shipTable:ClearRows()

		local rowEquip = {}
		local row = 1
		for i=1,#equipTypes do
			local e = equipTypes[i]
			local n = Game.player:GetEquipCount(EquipDef[e].slot, e)
			if n > 0 then
				local icon = equipIcon[e] and ui:Image("icons/goods/"..equipIcon[e]..".png") or ""
				shipTable:AddRow({ icon, lcore[e], n })

				rowEquip[row] = e
				row = row + 1
			end
		end

		return rowEquip
	end
	local shipRowEquip = fillShipTable()

	stationTable.onRowClicked:Connect(function(row)
		local e = stationRowEquip[row+1]

		config.onBuy(e)

		stationRowEquip = fillStationTable()
		shipRowEquip = fillShipTable()
	end)

	shipTable.onRowClicked:Connect(function (row)
		local e = shipRowEquip[row+1]

		config.onSell(e)

		stationRowEquip = fillStationTable()
		shipRowEquip = fillShipTable()
	end)

	return stationTable, shipTable
end

return EquipmentTableWidgets
