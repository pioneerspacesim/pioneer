-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Lang = require 'Lang'
local Game = require 'Game'
local Rand = require 'Rand'
local Event = require 'Event'
local Character = require 'Character'
local Format = require 'Format'
local Serializer = require 'Serializer'
local ModalWindow = require 'pigui.libs.modal-win'
local ui = require 'pigui'
local Commodities = require 'Commodities'

local rescaleVector = ui.rescaleUI(Vector2(1, 1), Vector2(1600, 900), true)
local popupSpacer = Vector2(0,0)
local popupButtonSize = Vector2(0,0)
local popupMsg = ''
local popup = ModalWindow.New('goodsTraderPopup', function(self)
	popupSpacer((ui.getContentRegion().x - 100*rescaleVector.x) / 2, 0)
	popupButtonSize(100 * rescaleVector.x, 0)
	ui.text(popupMsg)
	ui.dummy(popupSpacer)
	ui.sameLine()
	if ui.button("OK", popupButtonSize) then
		self:close()
	end
end)

---------------
-- Fuel Club --
---------------

-- The fuel club is an organisation that provides subsidized hydrogen fuel,
-- military fuel and radioactives processing for its members. Membership is
-- normally annual. A Goods Trader interface is provided. Facilities do not
-- exist on every station in the galaxy.

local l = Lang.GetResource("module-fuelclub")
local lc = Lang.GetResource("ui-core")

-- Default numeric values --
----------------------------
local oneday = 86400 -- One standard day
local oneyear = 31557600 -- One standard Julian year
-- 10, guaranteed random by D16 dice roll.
-- This is to make the BBS name different from the station welcome character.

-- 27, guaranteed random by D100 dice roll.
-- This is to make the BBS name different from the station welcome character.
-- This will be unnecessary once ported to Character
local seedbump = 27
local ads = {}
local memberships = {
-- some_club = {
--	joined = 0,
--	expiry = oneyear,
--  milrads = 0, -- counter for military fuel / radioactives balance
-- }
}

local flavours = {
	{    -- Independent club
		title    = l.FLAVOUR_0_TITLE,
		clubname = l.FLAVOUR_0_CLUBNAME,
		desc     = l.FLAVOUR_0_DESC,
		welcome = l.FLAVOUR_0_WELCOME,
		nonmember_intro = l.FLAVOUR_0_NONMEMBER_INTRO,
		member_intro = l.FLAVOUR_0_MEMBER_INTRO,
		annual_fee = 400,
		availability = {FED = 0.1, CIW = 0.1, HABER = 0, IND = 0.4} -- probability for these factions
	},
	{   -- SolFed club (reuse some messages)
		title    = l.FLAVOUR_1_TITLE,
		clubname = l.FLAVOUR_1_CLUBNAME,
		desc     = l.FLAVOUR_1_DESC,
		welcome = l.FLAVOUR_0_WELCOME,
		nonmember_intro = l.FLAVOUR_1_NONMEMBER_INTRO,
		member_intro = l.FLAVOUR_0_MEMBER_INTRO,
		annual_fee = 400,
		availability = {FED = 0.4, CIW = 0, HABER = 0, IND = 0}
	},
	{   -- Confederation club
		title    = l.FLAVOUR_2_TITLE,
		clubname = l.FLAVOUR_2_CLUBNAME,
		desc     = l.FLAVOUR_2_DESC,
		welcome = l.FLAVOUR_2_WELCOME,
		nonmember_intro = l.FLAVOUR_2_NONMEMBER_INTRO,
		member_intro = l.FLAVOUR_0_MEMBER_INTRO,
		annual_fee = 300,
		availability = {FED = 0, CIW = 0.4, HABER = 0, IND = 0}
	},
	{   -- Haber fuel division
		title    = l.FLAVOUR_3_TITLE,
		clubname = l.FLAVOUR_3_CLUBNAME,
		desc     = l.FLAVOUR_3_DESC,
		welcome = l.FLAVOUR_0_WELCOME,
		nonmember_intro = l.FLAVOUR_3_NONMEMBER_INTRO,
		member_intro = l.FLAVOUR_3_MEMBER_INTRO,
		annual_fee = 600,
		availability = {FED = 0, CIW = 0, HABER=1, IND = 0}
	}
}

local loaded_data -- empty unless the game is loaded


local onDelete = function (ref)
	-- ad has been destroyed; forget its details
	ads[ref] = nil
end

local onChat
-- This can recurse now!
onChat = function (form, ref, option)
	local ad = ads[ref]

	form:Clear()
	form:SetFace(ad.character)
	form:SetTitle(ad.flavour.welcome:interp({clubname = ad.flavour.clubname}))
	local membership = memberships[ad.flavour.clubname]

	if option == 0 and membership and (membership.joined + membership.expiry > Game.time) then
		local saleables = {
			[Commodities.hydrogen] = 0.5,       -- half price Hydrogen
			[Commodities.military_fuel] = 0.80, -- 20% off Milfuel
			[Commodities.radioactives] = 0.0    -- Radioactives go free
		}

		-- members get the trader interface
		form:SetMessage(string.interp(ad.flavour.member_intro, {radioactives=Commodities.radioactives:GetName()}))
		form:AddGoodsTrader({
			canTrade = function (ref, commodity)
				return saleables[commodity] and true or false
			end,
			canDisplayItem = function (ref, commodity)
				return saleables[commodity] and true or false
			end,
			getStock = function (ref, commodity)
				local prev = ad.stock[commodity.name]
				if prev then
					return prev
				end
				if commodity == Commodities.radioactives then
					ad.stock[commodity.name] = 0
					return 0
				end
				local cur = Engine.rand:Integer(2, (commodity == Commodities.military_fuel and 25 or 50)) + Engine.rand:Integer(3, 25)
				ad.stock[commodity.name] = cur
				return cur
			end,
			getBuyPrice = function (ref, commodity)
				return ad.station:GetCommodityPrice(commodity) * saleables[commodity]
			end,
			getSellPrice = function (ref, commodity)
				return ad.station:GetCommodityPrice(commodity) * saleables[commodity]
			end,
			-- Next two functions: If your membership is nearly up, you'd better
			-- trade quickly, because we do check!
			-- Also checking that the player isn't abusing radioactives sales...
			onClickBuy = function (ref, commodity)
				return membership.joined + membership.expiry > Game.time
			end,
			onClickSell = function (ref, commodity, market)
				local count = 1
				if market.tradeAmount ~= nil then
					count = market.tradeAmount
				end

				if (commodity == Commodities.radioactives and membership.milrads < count) then
					popupMsg = string.interp(l.YOU_MUST_BUY, {
						military_fuel = Commodities.military_fuel:GetName(),
						radioactives = Commodities.radioactives:GetName(),
					})
					popup:open()
					return false
				end
				return	membership.joined + membership.expiry > Game.time
			end,
			bought = function (ref, commodity, market)
				local count = 1
				if market.tradeAmount ~= nil then
					count = market.tradeAmount
				end

				ad.stock[commodity.name] = ad.stock[commodity.name] - count
				if commodity == Commodities.radioactives or commodity == Commodities.military_fuel then
					membership.milrads = membership.milrads + count
				end
			end,
			sold = function (ref, commodity, market)
				local count = 1
				if market.tradeAmount ~= nil then
					count = market.tradeAmount
				end

				ad.stock[commodity.name] = ad.stock[commodity.name] + count
				if commodity == Commodities.radioactives or commodity == Commodities.military_fuel then
					membership.milrads = membership.milrads - count
				end
			end,
		})
		-- members can only refuel once a day
		if not membership.refueled and (membership.refueling_date + oneday < Game.time) and Game.player.fuel < 100 then
			form:AddOption(l.REFUEL_FREE,3)
		end

	elseif option == -1 then
		-- hang up
		form:Close()

	elseif option == 1 then
		-- Player asked the question about radioactives
		form:SetMessage(string.interp(l.WE_WILL_ONLY_DISPOSE_OF, {
						military_fuel = Commodities.military_fuel:GetName(),
						radioactives = Commodities.radioactives:GetName()}))
		form:AddOption(l.APPLY_FOR_MEMBERSHIP,2)
		form:AddOption(lc.GO_BACK,0)

	elseif option == 2 then
		-- Player applied for membership
		if Game.player:GetMoney() > 500 then
			-- Membership application successful
			memberships[ad.flavour.clubname] = {
				joined = Game.time,
				expiry = oneyear,
				milrads = 0,
				refueled = false,
				refueling_date = 0,
			}
			Game.player:AddMoney(0 - ad.flavour.annual_fee)
			form:SetMessage(l.YOU_ARE_NOW_A_MEMBER:interp({
				expiry_date = Format.Date(memberships[ad.flavour.clubname].joined + memberships[ad.flavour.clubname].expiry)
			}))
			form:AddOption(l.BEGIN_TRADE,0)
		else
			-- Membership application unsuccessful
			form:SetMessage(l.YOUR_MEMBERSHIP_APPLICATION_HAS_BEEN_DECLINED)
		end

	elseif option == 3 then
		form:Clear() -- remove goods trader table
		form:SetFace(ad.character)
		form:SetTitle(ad.flavour.welcome:interp({clubname = ad.flavour.clubname}))
		form:SetMessage(string.interp(l.FUEL_LEVEL, {level = math.floor(Game.player.fuel)}))
		form:AddOption(lc.REFUEL_FULL,4)
		form:AddOption(lc.GO_BACK,0)

	elseif option == 4 then
		Game.player:SetFuelPercent()
		membership.refueled = true
		membership.refueling_date = Game.time
		form:SetMessage(string.interp(l.FUEL_LEVEL, {level = math.floor(Game.player.fuel)}))
		form:AddOption(lc.GO_BACK,0)

	else
		-- non-members get offered membership
		local message = ad.flavour.nonmember_intro:interp({clubname=ad.flavour.clubname}).."\n"..
			"\n\t* " ..l.LIST_BENEFITS_FUEL_INTRO..
			"\n\t* "..string.interp(l.LIST_BENEFITS_FUEL, {fuel=Commodities.hydrogen:GetName()})..
			"\n\t* "..string.interp(l.LIST_BENEFITS_FUEL, {fuel=Commodities.military_fuel:GetName()})..
			"\n\t* "..string.interp(l.LIST_BENEFITS_DISPOSAL, {radioactives=Commodities.radioactives:GetName()})..
			"\n\t* "..l.LIST_BENEFITS_FUEL_TANK..
			"\n\n"  ..string.interp(l.LIST_BENEFITS_JOIN, {membership_fee=Format.Money(ad.flavour.annual_fee)})

		form:SetMessage(message)
		form:AddOption(l.WHAT_CONDITIONS_APPLY:interp({radioactives = Commodities.radioactives:GetName()}),1)
		form:AddOption(l.APPLY_FOR_MEMBERSHIP,2)
	end
end

local onCreateBB = function (station)

	local faction = Game.system.faction.name

	-- For convenes, map long faction name to shorter table key
	local faction_key
	if faction == "Solar Federation" then
		faction_key = "FED"
	elseif faction == "Commonwealth of Independent Worlds" then
		faction_key = "CIW"
	elseif faction == "Haber Corporation" then
		faction_key = "HABER"
	else
		faction_key = "IND"
	end

	-- deterministically generate our instance
	local rand = Rand.New(station.seed + seedbump)

	for k,flavour in pairs(flavours) do
		if rand:Number(0,1) < flavour.availability[faction_key] then
			-- Create our bulletin board ad
			local ad = {station = station, stock = {}, price = {}}
			ad.flavour = flavour
			ad.character = Character.New({
					title = ad.flavour.clubname,
					armour = false,
			})
			ads[station:AddAdvert({
				title       = ad.flavour.title,
				description = ad.flavour.desc,
				icon        = "fuel_club",
				onChat      = onChat,
				onDelete    = onDelete
			})] = ad
		end
	end
end

local onShipUndocked = function (ship, station)
	if not ship:IsPlayer() then return end

	for _, membership in pairs(memberships) do
		membership.refueled = false
	end
end

local onGameStart = function ()

	if loaded_data and loaded_data.ads then
		-- rebuild saved adverts
		for k,ad in pairs(loaded_data.ads) do
			ads[ad.station:AddAdvert({
				title       = ad.flavour.title,
				description = ad.flavour.desc,
				icon        = "fuel_club",
				onChat      = onChat,
				onDelete    = onDelete
			})] = ad
		end
		-- load membership info
		memberships = loaded_data.memberships
		loaded_data = nil
	else
		-- Hopefully this won't be necessary after Pioneer handles Lua teardown
		memberships = {}
	end
end

local serialize = function ()
	return { ads = ads, memberships = memberships }
end

local unserialize = function (data)
	loaded_data = data
end

Event.Register("onCreateBB", onCreateBB)
Event.Register("onShipUndocked", onShipUndocked)
Event.Register("onGameStart", onGameStart)

Serializer:Register("FuelClub", serialize, unserialize)
