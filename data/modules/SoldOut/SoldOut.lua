-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Lang = require 'Lang'
local Game = require 'Game'
local Event = require 'Event'
local Serializer = require 'Serializer'
local Character = require 'Character'
local Format = require 'Format'
local Commodities = require 'Commodities'

local l = Lang.GetResource("module-soldout")

local ads = {}

local onChat = function (form, ref, option)
	local ad = ads[ref]

	---@type CargoManager
	local cargoMgr = Game.player:GetComponent('CargoManager')

	-- Maximum amount the player can sell:
	local max = cargoMgr:CountCommodity(ad.commodity)
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
			cargoMgr:RemoveCommodity(ad.commodity, max)
			Game.player:AddMoney(ad.price * max)
			ad.amount = ad.amount - max
			form:SetMessage(ad.message .. "\n\n" .. string.interp(l.AMOUNT, {amount = ad.amount}))
		end
	elseif option > 0 then
		if max < option then
			form:SetMessage(string.interp(l.NOT_IN_STOCK, {commodity = ad.commodity:GetName()}))
		else
			cargoMgr:RemoveCommodity(ad.commodity, option)
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
		price     = 2 * station:GetCommodityPrice(commodity) * ((not Game.system:IsCommodityLegal(commodity:GetName()) and 2) or 1),
		commodity = commodity,
	}

	-- Amount that the desperado wants to buy is determined by the
	-- wallet, i.e. fewer units if expensive commodity.
	-- Money is uniformly distirubted in log space
	local money_to_spend = lograndom(1e2, 1e5)
	ad.amount = math.ceil(money_to_spend / math.abs(ad.price))

	ad.title = string.interp(l.ADTEXT,
		{commodity = ad.commodity:GetName(), price = Format.Money(ad.price)})

	ad.message = string.interp(l.MESSAGE,
		{name = ad.client.name, commodity = commodity:GetName(), price = Format.Money(ad.price)})

	local ref = station:AddAdvert({
		title       = l.TITLE,
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
	for i, c in pairs(Commodities) do
		if c.purchasable then
			local stock = station:GetCommodityStock(c)
			if stock == 0 then
				table.insert(sold_out, c)
			end
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
			title       = l.TITLE,
			description = string.interp(l.ADTEXT, {commodity = ad.commodity:GetName(), price = Format.Money(ad.price)}),
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
