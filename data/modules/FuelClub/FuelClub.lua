-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Lang = import("Lang")
local Game = import("Game")
local Comms = import("Comms")
local Rand = import("Rand")
local Event = import("Event")
local Character = import("Character")
local Format = import("Format")
local Serializer = import("Serializer")

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

	local setMessage = function (message)
		form:SetMessage(message:interp({
			hydrogen = l.HYDROGEN,
			military_fuel = l.MILITARY_FUEL,
			radioactives = l.RADIOACTIVES,
			water = l.WATER,
			clubname = ad.flavour.clubname,
		}))
	end

	form:Clear()
	form:SetFace(ad.character)
	form:SetTitle(ad.flavour.welcome:interp({clubname = ad.flavour.clubname}))
	local membership = memberships[ad.flavour.clubname]

	if membership and (membership.joined + membership.expiry > Game.time) then
		-- members get refuelled, whether or not the station managed to do it
		Game.player:SetFuelPercent()
		-- members get the trader interface
		setMessage(ad.flavour.member_intro)
		form:AddGoodsTrader({
			canTrade = function (ref, commodity)
				return ({
					['HYDROGEN'] = true,
					['MILITARY_FUEL'] = true,
					['RADIOACTIVES'] = true,
					['WATER'] = true,
				})[commodity]
			end,
			getStock = function (ref, commodity)
				ad.stock[commodity] = ({
					-- Hydrogen: Between 5 and 75 units, tending to lower values
					['HYDROGEN'] = ad.stock.HYDROGEN or (Engine.rand:Integer(2,50) + Engine.rand:Integer(3,25)),
					-- Milfuel: Between 5 and 50 units, tending to median values
					['MILITARY_FUEL'] = ad.stock.MILITARY_FUEL or (Engine.rand:Integer(2,25) + Engine.rand:Integer(3,25)),
					['WATER'] = ad.stock.WATER or (Engine.rand:Integer(2,25) + Engine.rand:Integer(3,25)),
					-- Always taken away
					['RADIOACTIVES'] = 0,
				})[commodity]
				return ad.stock[commodity]
			end,
			getPrice = function (ref, commodity)
				return ad.station:GetEquipmentPrice(commodity) * ({
					['HYDROGEN'] = 0.5, -- half price Hydrogen
					['MILITARY_FUEL'] = 0.80, -- 20% off Milfuel
					['WATER'] = 0.60, -- 40% off Water
					['RADIOACTIVES'] = 0, -- Radioactives go free
				})[commodity]
			end,
			-- Next two functions: If your membership is nearly up, you'd better
			-- trade quickly, because we do check!
			-- Also checking that the player isn't abusing radioactives sales...
			onClickBuy = function (ref, commodity)
				return membership.joined + membership.expiry > Game.time
			end,
			onClickSell = function (ref, commodity)
				if (commodity == 'RADIOACTIVES' and membership.milrads < 1) then
					Comms.Message(l.YOU_MUST_BUY:interp({
						military_fuel = l.MILITARY_FUEL,
						radioactives = l.RADIOACTIVES,
						water = l.WATER,
					}))
					return false
				end
				return	membership.joined + membership.expiry > Game.time
			end,
			bought = function (ref, commodity)
				ad.stock[commodity] = ad.stock[commodity] + 1
				if commodity == 'MILITARY_FUEL' or commodity == 'RADIOACTIVES' then
					membership.milrads = membership.milrads -1
				end
			end,
			sold = function (ref, commodity)
				ad.stock[commodity] = ad.stock[commodity] - 1
				if commodity == 'MILITARY_FUEL' or commodity == 'RADIOACTIVES' then
					membership.milrads = membership.milrads +1
				end
			end,
		})

	elseif option == -1 then
		-- hang up
		form:Close()

	elseif option == 1 then
		-- Player asked the question about radioactives
		setMessage(l.WE_WILL_ONLY_DISPOSE_OF)
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
			setMessage(l.YOU_ARE_NOW_A_MEMBER:interp({
				expiry_date = Format.Date(memberships[ad.flavour.clubname].joined + memberships[ad.flavour.clubname].expiry)
			}))
			form:AddOption(l.BEGIN_TRADE,0)
		else
			-- Membership application unsuccessful
			setMessage(l.YOUR_MEMBERSHIP_APPLICATION_HAS_BEEN_DECLINED)
		end

	else
		-- non-members get offered membership
		setMessage(ad.flavour.nonmember_intro:interp({
			membership_fee = Format.Money(ad.flavour.annual_fee)
		}))
		form:AddOption(l.WHAT_CONDITIONS_APPLY:interp({radioactives = l.RADIOACTIVES}),1)
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
