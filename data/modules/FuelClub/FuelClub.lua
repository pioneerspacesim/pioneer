-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Lang = import("Lang")
local Game = import("Game")
local Rand = import("Rand")
local Event = import("Event")
local Character = import("Character")
local Format = import("Format")
local Serializer = import("Serializer")
local Equipment = import("Equipment")

local MessageBox = import("ui/MessageBox")

---------------
-- Fuel Club --
---------------

-- The fuel club is an organisation that provides subsidized hydrogen fuel,
-- military fuel and radioactives processing for its members. Membership is
-- normally annual. A Goods Trader interface is provided. Facilities do not
-- exist on every station in the galaxy.

local l = Lang.GetResource("module-fuelclub")

-- Default numeric values --
----------------------------
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

-- 1 / probability that you'll see one in a BBS
local chance_of_availability = 3

local flavours = {
	{
		clubname = l.FLAVOUR_0_CLUBNAME,
		welcome = l.FLAVOUR_0_WELCOME,
		nonmember_intro = l.FLAVOUR_0_NONMEMBER_INTRO,
		member_intro = l.FLAVOUR_0_MEMBER_INTRO,
		annual_fee = 400,
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

	if membership and (membership.joined + membership.expiry > Game.time) then
		-- members get refuelled, whether or not the station managed to do it
		Game.player:SetFuelPercent()
		-- members get the trader interface
		form:SetMessage(string.interp(ad.flavour.member_intro, {radioactives=Equipment.cargo.radioactives:GetName()}))
		form:AddGoodsTrader({
			canTrade = function (ref, commodity)
				return ({
					[Equipment.cargo.hydrogen] = true,
					[Equipment.cargo.military_fuel] = true,
					[Equipment.cargo.radioactives] = true
				})[commodity]
			end,
			getStock = function (ref, commodity)
				local prev = ad.stock[commodity]
				if prev then
					return prev
				end
				if commodity == Equipment.cargo.radioactives then
					ad.stock[commodity] = 0
					return 0
				end
				local cur = Engine.rand:Integer(2, (commodity == Equipment.cargo.military_fuel and 25 or 50)) + Engine.rand:Integer(3, 25)
				ad.stock[commodity] = cur
				return cur
			end,
			getPrice = function (ref, commodity)
				return ad.station:GetEquipmentPrice(commodity) * ({
					[Equipment.cargo.hydrogen] = 0.5, -- half price Hydrogen
					[Equipment.cargo.military_fuel] = 0.80, -- 20% off Milfuel
					[Equipment.cargo.radioactives] = 0, -- Radioactives go free
				})[commodity]
			end,
			-- Next two functions: If your membership is nearly up, you'd better
			-- trade quickly, because we do check!
			-- Also checking that the player isn't abusing radioactives sales...
			onClickBuy = function (ref, commodity)
				return membership.joined + membership.expiry > Game.time
			end,
			onClickSell = function (ref, commodity)
				if (commodity == Equipment.cargo.radioactives and membership.milrads < 1) then
					MessageBox.Message(string.interp(l.YOU_MUST_BUY, {
						military_fuel = Equipment.cargo.military_fuel:GetName(),
						radioactives = Equipment.cargo.radioactives:GetName(),
					}))
					return false
				end
				return	membership.joined + membership.expiry > Game.time
			end,
			bought = function (ref, commodity)
				ad.stock[commodity] = ad.stock[commodity] + 1
				if commodity == Equipment.cargo.radioactives or commodity == Equipment.cargo.military_fuel then
					membership.milrads = membership.milrads -1
				end
			end,
			sold = function (ref, commodity)
				ad.stock[commodity] = ad.stock[commodity] - 1
				if commodity == Equipment.cargo.radioactives or commodity == Equipment.cargo.military_fuel then
					membership.milrads = membership.milrads +1
				end
			end,
		})

	elseif option == -1 then
		-- hang up
		form:Close()

	elseif option == 1 then
		-- Player asked the question about radioactives
		form:SetMessage(string.interp(l.WE_WILL_ONLY_DISPOSE_OF, {
						military_fuel = Equipment.cargo.military_fuel:GetName(),
						radioactives = Equipment.cargo.radioactives:GetName()}))
		form:AddOption(l.APPLY_FOR_MEMBERSHIP,2)
		form:AddOption(l.GO_BACK,0)

	elseif option == 2 then
		-- Player applied for membership
		if Game.player:GetMoney() > 500 then
			-- Membership application successful
			memberships[ad.flavour.clubname] = {
				joined = Game.time,
				expiry = oneyear,
				milrads = 0,
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

	else
		-- non-members get offered membership
		message = ad.flavour.nonmember_intro:interp({clubname=ad.flavour.clubname}).."\n"..
			"\n\t* " ..l.LIST_BENEFITS_FUEL_INTRO..
			"\n\t* "..string.interp(l.LIST_BENEFITS_FUEL, {fuel=Equipment.cargo.hydrogen:GetName()})..
			"\n\t* "..string.interp(l.LIST_BENEFITS_FUEL, {fuel=Equipment.cargo.military_fuel:GetName()})..
			"\n\t* "..string.interp(l.LIST_BENEFITS_DISPOSAL, {radioactives=Equipment.cargo.radioactives:GetName()})..
			"\n\t* "..l.LIST_BENEFITS_FUEL_TANK..
			"\n\n"  ..string.interp(l.LIST_BENEFITS_JOIN, {membership_fee=Format.Money(ad.flavour.annual_fee)})

		form:SetMessage(message)
		form:AddOption(l.WHAT_CONDITIONS_APPLY:interp({radioactives = Equipment.cargo.radioactives:GetName()}),1)
		form:AddOption(l.APPLY_FOR_MEMBERSHIP,2)
	end
end

local onCreateBB = function (station)
	-- deterministically generate our instance
	local rand = Rand.New(station.seed + seedbump)
	if rand:Integer(1,chance_of_availability) == 1 then
		-- Create our bulletin board ad
		local ad = {station = station, stock = {}, price = {}}
		ad.flavour = flavours[rand:Integer(1,#flavours)]
		ad.character = Character.New({
			title = ad.flavour.clubname,
			armour = false,
		})
		ads[station:AddAdvert({
			description = ad.flavour.clubname,
			icon        = "fuel_club",
			onChat      = onChat,
			onDelete    = onDelete})] = ad
	end
end

local onGameStart = function ()
	local ref
	if loaded_data then
		-- rebuild saved adverts
		for k,ad in pairs(loaded_data.ads) do
			ads[ad.station:AddAdvert({
				description = ad.flavour.clubname,
				icon        = "fuel_club",
				onChat      = onChat,
				onDelete    = onDelete})] = ad
		end
		-- load membership info
		memberships = loaded_data.memberships
		for k,v in pairs(memberships) do
			for l,w in pairs(v) do
			end
		end
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
Event.Register("onGameStart", onGameStart)

Serializer:Register("FuelClub", serialize, unserialize)
