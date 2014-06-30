-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Lang = import("Lang")
local Game = import("Game")
local utils = import("utils")
local Format = import("Format")
local Equipment = import("Equipment")

local MessageBox = import("ui/MessageBox")

local l = Lang.GetResource("ui-core")

-- XXX equipment strings are in core. this sucks
local lcore = Lang.GetResource("core")

local ui = Engine.ui

-- loose money when you sell parts back to the station.
local sellPriceReduction = 0.8

local defaultFuncs = {
	-- can we trade in this item
	canTrade = function (e)
		return e.purchasable and e:IsValidSlot("cargo") and Game.system:IsCommodityLegal(e)
	end,

	-- how much of this item do we have in stock?
	getStock = function (e)
		return Game.player:GetDockedWith():GetEquipmentStock(e)
	end,

	-- what do we charge for this item if we are buying
	getBuyPrice = function (e)
		return Game.player:GetDockedWith():GetEquipmentPrice(e)
	end,

	-- what do we get for this item if we are selling
	getSellPrice = function (e)
		basePrice = Game.player:GetDockedWith():GetEquipmentPrice(e)
		if basePrice > 0 then
			return sellPriceReduction * basePrice
		else
			return 1.0/sellPriceReduction * basePrice
		end
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
	buy   = l.BUY,
	sell  = l.SELL,
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

local defaultStationColumnValue = {
	icon  = function (e, funcs) return e.icon_name and ui:Image("icons/goods/"..e.icon_name..".png") or "" end,
	name  = function (e, funcs) return e:GetName() end,
	price = function (e, funcs) return Format.Money(funcs.getBuyPrice(e)) end,
	buy   = function (e, funcs) return Format.Money(funcs.getBuyPrice(e)) end,
	sell  = function (e, funcs) return Format.Money(funcs.getSellPrice(e)) end,
	stock = function (e, funcs) return funcs.getStock(e) end,
	mass  = function (e, funcs) return string.format("%dt", e.capabilities.mass) end,
}

local defaultShipColumnValue = {
	icon      = function (e, funcs) return e.icon_name and ui:Image("icons/goods/"..e.icon_name..".png") or "" end,
	name      = function (e, funcs) return e:GetName() end,
	amount    = function (e, funcs) return Game.player:CountEquip(e) end,
	mass      = function (e, funcs) return string.format("%dt", e.capabilities.mass) end,
	massTotal = function (e, funcs) return string.format("%dt", Game.player:CountEquip(e)*e.capabilities.mass) end,
}


local EquipmentTableWidgets = {}

function EquipmentTableWidgets.Pair (config)
	local funcs = {
		canTrade = config.canTrade or defaultFuncs.canTrade,
		getStock = config.getStock or defaultFuncs.getStock,
		getBuyPrice = config.getBuyPrice or defaultFuncs.getBuyPrice,
		getSellPrice = config.getSellPrice or defaultFuncs.getSellPrice,
		onClickBuy = config.onClickBuy or defaultFuncs.onClickBuy,
		onClickSell = config.onClickSell or defaultFuncs.onClickSell,
		bought = config.bought or defaultFuncs.bought,
		sold = config.sold or defaultFuncs.sold,
	}

	local stationColumnValue = {
		icon  = config.icon  or defaultStationColumnValue.icon,
		name  = config.name  or defaultStationColumnValue.name,
		price = config.price or defaultStationColumnValue.price,
		buy   = config.buy   or defaultStationColumnValue.buy,
		sell  = config.sell  or defaultStationColumnValue.sell,
		stock = config.stock or defaultStationColumnValue.stock,
		mass  = config.mass  or defaultStationColumnValue.mass,
	}

	local shipColumnValue = {
		icon      = config.icon      or defaultShipColumnValue.icon,
		name      = config.name      or defaultShipColumnValue.name,
		amount    = config.amount    or defaultShipColumnValue.amount,
		mass      = config.mass      or defaultShipColumnValue.mass,
		massTotal = config.massTotal or defaultShipColumnValue.massTotal,
	}

	local equipTypes = {}
	for _,t in pairs({Equipment.cargo, Equipment.misc, Equipment.laser, Equipment.hyperspace}) do
		for k,e in pairs(t) do
			if funcs.canTrade(e) then
				table.insert(equipTypes, e)
			end
		end
	end

	local sortingFunction = function(e1,e2)
		if e1:GetDefaultSlot() == e2:GetDefaultSlot() then
			if e1:GetDefaultSlot() == "cargo" then
				return e1:GetName() < e2:GetName()        -- cargo sorted on translated name
			else
				if e1:GetDefaultSlot():find("laser") then -- can be laser_front or _back
					if e1.l10n_key:find("PULSE") and e2.l10n_key:find("PULSE") or
					e1.l10n_key:find("PLASMA") and e2.l10n_key:find("PLASMA") then
						return e1.price < e2.price
					else
						return e1.l10n_key < e2.l10n_key
					end
				else
					return e1.l10n_key < e2.l10n_key
				end
			end
		else
			return e1:GetDefaultSlot() < e2:GetDefaultSlot()
		end
	end
	table.sort(equipTypes, sortingFunction)

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
		for i, e in ipairs(equipTypes) do
			stationTable:AddRow(utils.build_table(utils.map(function (k,v) return k,stationColumnValue[v](e,funcs) end, ipairs(config.stationColumns))))
			rowEquip[i] = e
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
		for i,e in ipairs(equipTypes) do
			local n = Game.player:CountEquip(e)
			if n > 0 then
				local icon = e.icon_name and ui:Image("icons/goods/"..e.icon_name..".png") or ""
				shipTable:AddRow(utils.build_table(utils.map(function (k,v) return k,shipColumnValue[v](e, funcs) end, ipairs(config.shipColumns))))
				table.insert(rowEquip, e)
			end
		end

		return rowEquip
	end
	local shipRowEquip = fillShipTable()

	local function onBuy (e)
		if not funcs.onClickBuy(e) then return end

		if funcs.getStock(e) <= 0 then
			MessageBox.Message(l.ITEM_IS_OUT_OF_STOCK)
			return
		end

		local player = Game.player

		-- if this ship model doesn't support fitting of this equip:
		if player:GetEquipSlotCapacity(e:GetDefaultSlot(player)) < 1 then
			MessageBox.Message(string.interp(l.NOT_SUPPORTED_ON_THIS_SHIP,
				 {equipment = e:GetName(),}))
			return
		end

		-- if ship maxed out in this slot
		if player:GetEquipFree(e:GetDefaultSlot(player)) < 1 then
			MessageBox.Message(l.SHIP_IS_FULLY_EQUIPPED)
			return
		end

		-- if ship too heavy to support more
		if player.freeCapacity < e.capabilities.mass then
			MessageBox.Message(l.SHIP_IS_FULLY_LADEN)
			return
		end


		local price = funcs.getBuyPrice(e)
		if player:GetMoney() < funcs.getBuyPrice(e) then
			MessageBox.Message(l.YOU_NOT_ENOUGH_MONEY)
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
		player:AddMoney(funcs.getSellPrice(e))

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
