-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Lang = import("Lang")
local Game = import("Game")
local EquipDef = import("EquipDef")
local Comms = import("Comms")
local utils = import("utils")

local l = Lang.GetResource("ui-core")

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

local defaultFuncs = {
	-- can we trade in this item
	canTrade = function (e)
		return EquipDef[e].purchasable and EquipDef[e].slot == "CARGO" and Game.system:IsCommodityLegal(e)
	end,

	-- how much of this item do we have in stock?
	getStock = function (e)
		return Game.player:GetDockedWith():GetEquipmentStock(e)
	end,

	-- what do we charge for this item?
	getPrice = function (e)
		return Game.player:GetDockedWith():GetEquipmentPrice(e)
	end,

	-- do something when a "buy" button is clicked
	-- return true if the buy can proceed
	onClickBuy = function (e)
		return true -- allow buy
	end,

	-- do something when a "sell" button is clicked
	-- return true if the buy can proceed
	onClickSell = function (e)
		return true -- allow sell
	end,

	-- do something when we buy this commodity
	bought = function (e)
		-- add one to our stock
		Game.player:GetDockedWith():AddEquipmentStock(e, 1)
	end,

	-- do something when we sell this items
	sold = function (e)
		Game.player:GetDockedWith():AddEquipmentStock(e, -1)
	end,
}

local stationColumnHeading = {
	icon  = "",
	name  = l.NAME_OBJECT,
	price = l.PRICE,
	stock = l.IN_STOCK,
	mass  = l.MASS,
}
local shipColumnHeading = {
	icon      = "",
	name      = l.NAME_OBJECT,
	amount    = l.AMOUNT,
	mass      = l.MASS,
	massTotal = l.TOTAL_MASS,
}

local stationColumnValue = {
	icon  = function (e, funcs) return equipIcon[e] and ui:Image("icons/goods/"..equipIcon[e]..".png") or "" end,
	name  = function (e, funcs) return lcore[e] end,
	price = function (e, funcs) return string.format("%0.2f", funcs.getPrice(e)) end,
	stock = function (e, funcs) return funcs.getStock(e) end,
	mass  = function (e, funcs) return string.format("%dt", EquipDef[e].mass) end,
}
local shipColumnValue = {
	icon      = function (e, funcs) return equipIcon[e] and ui:Image("icons/goods/"..equipIcon[e]..".png") or "" end,
	name      = function (e, funcs) return lcore[e] end,
	amount    = function (e, funcs) return Game.player:GetEquipCount(EquipDef[e].slot, e) end,
	mass      = function (e, funcs) return string.format("%dt", EquipDef[e].mass) end,
	massTotal = function (e, funcs) return string.format("%dt", Game.player:GetEquipCount(EquipDef[e].slot,e)*EquipDef[e].mass) end,
}

local EquipmentTableWidgets = {}

function EquipmentTableWidgets.Pair (config)
	local funcs = {
		canTrade = config.canTrade or defaultFuncs.canTrade,
		getStock = config.getStock or defaultFuncs.getStock,
		getPrice = config.getPrice or defaultFuncs.getPrice,
		onClickBuy = config.onClickBuy or defaultFuncs.onClickBuy,
		onClickSell = config.onClickSell or defaultFuncs.onClickSell,
		bought = config.bought or defaultFuncs.bought,
		sold = config.sold or defaultFuncs.sold,
	}

	local equipTypes = {}
	for k,v in pairs(EquipDef) do
		if funcs.canTrade(v.id) and k ~= "NONE" then
			table.insert(equipTypes, k)
		end
	end
	table.sort(equipTypes)

	local stationTable =
		ui:Table()
			:SetRowSpacing(5)
			:SetColumnSpacing(10)
			:SetHeadingRow(utils.build_table(utils.map(function (k,v) return k,stationColumnHeading[v] end, ipairs(config.stationColumns))))
			:SetHeadingFont("LARGE")
			:SetRowAlignment("CENTER")
			:SetMouseEnabled(true)

	local function fillStationTable ()
		stationTable:ClearRows()

		local rowEquip = {}
		local row = 1
		for i=1,#equipTypes do
			local e = equipTypes[i]
			stationTable:AddRow(utils.build_table(utils.map(function (k,v) return k,stationColumnValue[v](e,funcs) end, ipairs(config.stationColumns))))
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
			:SetHeadingRow(utils.build_table(utils.map(function (k,v) return k,shipColumnHeading[v] end, ipairs(config.shipColumns))))
			:SetHeadingFont("LARGE")
			:SetRowAlignment("CENTER")
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
				shipTable:AddRow(utils.build_table(utils.map(function (k,v) return k,shipColumnValue[v](e, funcs) end, ipairs(config.shipColumns))))
				rowEquip[row] = e
				row = row + 1
			end
		end

		return rowEquip
	end
	local shipRowEquip = fillShipTable()

	local function onBuy (e)
		if not funcs.onClickBuy(e) then return end

		if funcs.getStock(e) <= 0 then
			Comms.Message(l.ITEM_IS_OUT_OF_STOCK)
			return
		end

		local player = Game.player

		if player:GetEquipFree(EquipDef[e].slot) < 1 then
			Comms.Message(l.SHIP_IS_FULLY_LADEN)
			return
		end

		if player.freeCapacity < EquipDef[e].mass then
			Comms.Message(l.SHIP_IS_FULLY_LADEN)
			return
		end

		local price = funcs.getPrice(e)
		if player:GetMoney() < funcs.getPrice(e) then
			Comms.Message(l.YOU_NOT_ENOUGH_MONEY)
			return
		end

		assert(player:AddEquip(e) == 1)
		player:AddMoney(-price)

		funcs.sold(e)
	end

	local function onSell (e)
		if not funcs.onClickSell(e) then return end

		local player = Game.player

		player:RemoveEquip(e)
		player:AddMoney(funcs.getPrice(e))

		funcs.bought(e)
	end

	stationTable.onRowClicked:Connect(function(row)
		local e = stationRowEquip[row+1]

		onBuy(e)

		stationRowEquip = fillStationTable()
		shipRowEquip = fillShipTable()
	end)

	shipTable.onRowClicked:Connect(function (row)
		local e = shipRowEquip[row+1]

		onSell(e)

		stationRowEquip = fillStationTable()
		shipRowEquip = fillShipTable()
	end)

	return stationTable, shipTable
end

return EquipmentTableWidgets
