-- Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Lang = require 'Lang'
local Game = require 'Game'
local Event = require 'Event'
local Serializer = require 'Serializer'
local Character = require 'Character'
local Format = require 'Format'
local Equipment = require 'Equipment'
local Calibration = require 'Calibration'

local l = Lang.GetResource("module-soldout")

local StockOne = Calibration.StockPriceOne
local PriceExp = Calibration.PriceExponential
local avgStock = Calibration.avgStock

local ads = {}

local onChat = function (form, ref, option)
	local ad = ads[ref]
	-- Maximum amount the player can sell:
	local max = Game.player:CountEquip(ad.commodity, "cargo")
	max = ad.amount < max and ad.amount or max

	form:Clear()
	form:SetTitle(ad.title)
	form:SetFace(ad.client)

	if option == 0 then
		form:SetMessage(ad.message .. "\n\n" .. string.interp(l.AMOUNT, {amount = ad.amount}))
	elseif option == -1 then
		form:Close()
		return
	elseif option == "max" then
		if max < 1 then
			form:SetMessage(string.interp(l.NOT_IN_STOCK, {commodity = ad.commodity:GetName()}))
		else
			Game.player:RemoveEquip(ad.commodity, max)
			Game.player:AddMoney(ad.price * max)
			ad.amount = ad.amount - max
			form:SetMessage(ad.message .. "\n\n" .. string.interp(l.AMOUNT, {amount = ad.amount}))
		end
	elseif option > 0 then
		if max < option then
			form:SetMessage(string.interp(l.NOT_IN_STOCK, {commodity = ad.commodity:GetName()}))
		else
			Game.player:RemoveEquip(ad.commodity, option)
			Game.player:AddMoney(ad.price * option)
			ad.amount = ad.amount - option
			form:SetMessage(ad.message .. "\n\n" .. string.interp(l.AMOUNT, {amount = ad.amount}))
		end
	end

	-- Buttons asking how much quantity to sell, store value as an int
	-- wrapped in a table:
	form:AddOption(l.SELL_1, 1)
	for i, val in pairs{10, 100, 1000} do
		form:AddOption(string.interp(l.SELL_N, {N=val}), val)
	end
	form:AddOption(l.SELL_ALL, "max")

	-- Buyer has bought all they want?
	if ad.amount == 0 then
		form:RemoveAdvertOnClose()
		ads[ref] = nil
		form:Close()
	end

	return
end

local onDelete = function (ref)
	ads[ref] = nil
end

-- Numbers that span orders of magnitude are best modelled by
-- random sampling from uniformly distributed density in log-space:
local lograndom = function(min, max)
	assert(min > 0)
	return math.exp(Engine.rand:Number(math.log(min), math.log(max)))
end

local makeAdvert = function(station, commodity)
	-- Select commodity to run low on
	local ad = {
		station   = station,
		client    = Character.New(),
		price     = 2 * station:GetEquipmentPrice(commodity),
		commodity = commodity,
	}

	-- Amount that the desperado wants to buy is determined by the
	-- wallet, i.e. fewer units if expensive commodity.
	-- Amount should not exceed what station normally wants (relevant values at line 57 (former line 48 of SpaceStation.lua).
	-- Actually we set to desperately want up to what a Major exports offer at double its price. ad.price/2 because it is already doubled.
	-- Replaces the use of lograndom function.
	local maxMoney = math.max(math.abs(ad.price)+1.00, Calibration.InvestmentOfPrice(math.abs(ad.price)/2) * avgStock * 2)
	local money_to_spend = Engine.rand:Number(math.abs(ad.price), maxMoney)

	ad.amount = math.min(math.ceil(money_to_spend / math.abs(ad.price)), Game.player.totalCargo * 2)

	ad.title = string.interp(l.TITLE,
		{commodity = ad.commodity:GetName(), price = Format.Money(ad.price)})

	ad.message = string.interp(l.MESSAGE,
		{name = ad.client.name, commodity = commodity:GetName(), price = Format.Money(ad.price)})

	local ref = station:AddAdvert({
		description = ad.title,
		icon        = "donate_to_cranks",
		onChat      = onChat,
		onDelete    = onDelete})
	ads[ref] = ad
end

local onCreateBB = function (station)
	-- Only relevant to create an advert for goods that are have zero stock
	-- (probability for that is set in SpaceStation.lua)
	local sold_out = {}
	for i, c in pairs(Equipment.cargo) do
		local stock = station:GetEquipmentStock(c)
		if stock == 0 then
			table.insert(sold_out, c)
		end
	end

	-- low random chance of spawning
	for i, c in pairs(sold_out) do
		if Engine.rand:Number(0, 1) > 0.5 then
			makeAdvert(station, c)
		end
	end
end

local loaded_data

local onGameStart = function ()
	ads = {}

	if not loaded_data or not loaded_data.ads then return end

	for k,ad in pairs(loaded_data.ads) do
		local ref = ad.station:AddAdvert({
			description = string.interp(l.TITLE, {commodity = ad.commodity:GetName(), price = Format.Money(ad.price)}),
			icon        = "donate_to_cranks",
			onChat      = onChat,
			onDelete    = onDelete})
		ads[ref] = ad
	end

	loaded_data = nil
end

local serialize = function ()
	return { ads = ads }
end

local unserialize = function (data)
	loaded_data = data
end

Event.Register("onCreateBB", onCreateBB)
Event.Register("onGameStart", onGameStart)

Serializer:Register("SoldOut", serialize, unserialize)
