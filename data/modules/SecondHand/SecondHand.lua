-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local utils = require 'utils'
local Lang = require 'Lang'
local Game = require 'Game'
local Event = require 'Event'
local Format = require 'Format'
local Serializer = require 'Serializer'
local Equipment = require 'Equipment'
local Character = require 'Character'
local utils = require 'utils'

local l = Lang.GetResource("module-secondhand")
local l2 = Lang.GetResource("ui-core")

-- average number of adverts per BBS and billion population
local N_equil = 0.1 -- [ads/(BBS*unit_population)]

-- inverse half life of an advert in (approximate) hours
local inv_tau = 1.0 / (4*24)

-- Note: including the 0
local max_flavour_index = 5
local max_surprise_index = 2

local flavours = {}
for i = 0,max_flavour_index do
    table.insert(flavours, {
        adtitle = l["FLAVOUR_" .. i .. "_TITLE"],
        adtext = l["FLAVOUR_" .. i .. "_TEXT"],
        adbody  = l["FLAVOUR_" .. i .. "_BODY"],
    })
end

local ads = {}

-- find a value in an array and return key
local onDelete = function (ref)
    ads[ref] = nil
end

-- check it fits on the ship, both the slot for that equipment and the
-- weight.
local canFit = function (e)

	-- todo: this is the same code as in equipmentTableWidgets, unify?
	local slot
	for i=1,#e.slots do
		if Game.player:GetEquipFree(e.slots[i]) > 0 then
			slot = e.slots[i]
			break
		end
	end

	-- if ship maxed out in any valid slot for e
	if not slot then
		return false, l2.SHIP_IS_FULLY_EQUIPPED
	end

	-- if ship too heavy with this equipment
	if Game.player.freeCapacity < e.capabilities.mass then
		return false, l2.SHIP_IS_FULLY_LADEN
	end

    return true, ""
end


local onChat = function (form, ref, option)
    local ad = ads[ref]

    form:Clear()

    if option == -1 then
        form:Close()
        return
    end

    form:SetFace(ad.character)

    if option == 0 then        -- state offer
        local adbody = string.interp(flavours[ad.flavour].adbody, {
            name  = ad.character.name,
            equipment = ad.equipment:GetName(),
            price = Format.Money(ad.price, false),
        })
        form:SetMessage(adbody)
		form:AddOption(l.HOW_DO_I_FITT_IT, 1);
    elseif option == 1 then    -- "How do I fit it"?
		form:SetMessage(l.FITTING_IS_INCLUDED_IN_THE_PRICE)
		form:AddOption(l.REPEAT_OFFER, 0);
    elseif option == 2 then    --- "Buy"
		if Engine.rand:Integer(0, 99) == 0 then -- This is a one in a hundred event
			form:SetMessage(l["NO_LONGER_AVAILABLE_" .. Engine.rand:Integer(0, max_surprise_index)])
			form:RemoveAdvertOnClose()
			ads[ref] = nil
		elseif Game.player:GetMoney() >= ad.price then
			local state, message_str = canFit(ad.equipment)
			if state then
				local buy_message = string.interp(l.HAS_BEEN_FITTED_TO_YOUR_SHIP,
												  {equipment = ad.equipment:GetName(),})
				form:SetMessage(buy_message)
				Game.player:AddEquip(ad.equipment, 1)
				Game.player:AddMoney(-ad.price)
				form:RemoveAdvertOnClose()
				ads[ref] = nil
			else
				form:SetMessage(message_str)
			end
        else
			form:SetMessage(l.YOU_DONT_HAVE_ENOUGH_MONEY)
        end

        return
    end

    form:AddOption(l.BUY, 2);
end


local makeAdvert = function (station)
    local character = Character.New()
    local flavour = Engine.rand:Integer(1, #flavours)

	-- Get all non-cargo or engines
	local avail_equipment = {}
	for k,v in pairs(Equipment) do
		if k == "laser" or k == "misc" then
			for _,e in pairs(v) do
				if e.purchasable then
					table.insert(avail_equipment,e)
				end
			end
		end
	end

	-- choose a random ship equipment
    local equipment = avail_equipment[Engine.rand:Integer(1,#avail_equipment)]

	-- buy back price in equipment market is 0.8, so make sure the value is higher
	local reduction = Engine.rand:Number(0.8,0.9)

    local price = utils.round(station:GetEquipmentPrice(equipment) * reduction, 2)

    local ad = {
        character = character,
        faceseed = Engine.rand:Integer(),
        flavour = flavour,
        price = price,
        equipment = equipment,
		station = station,
    }

    ad.desc = string.interp(flavours[ad.flavour].adtext, {
		equipment = equipment:GetName(),
    })
    local ref = station:AddAdvert({
		title       = flavours[ad.flavour].adtitle,
        description = ad.desc,
        icon        = "second_hand",
        onChat      = onChat,
        onDelete    = onDelete})
    ads[ref] = ad
end


-- Dynamics of adverts on the BBS --
------------------------------------
-- N(t) = Number of ads, lambda = decay constant:
--    d/dt N(t) = prod - lambda * N
-- and equilibrium:
--    dN/dt = 0 = prod - lambda * N_equil
-- and solution (if prod=0), with N_0 initial number:
--    N(t) = N_0 * exp(-lambda * t)
-- with tau = half life, i.e. N(tau) = 0.5*N_0 we get:
--    0.5*N_0 = N_0*exp(-lambda * tau)
-- else, with production:
--   N(t) = prod/lambda - N_0*exp(-lambda * t)
-- We want to have N_0 = N_equil, since BBS should spawn at equilibrium

local onCreateBB = function (station)

	-- instead of "for i = 1, Game.system.population do", get higher,
	-- and more consistent resolution
	local iter = 10

	-- create one ad for each unit of population with some probability
    for i = 1,iter do
		if Engine.rand:Number(0,1) < N_equil * Game.system.population / iter then
			makeAdvert(station)
		end
    end
end

local onUpdateBB = function (station)

	local lambda = 0.693147 * inv_tau    -- adv, del prob = ln(2) / tau
	local prod = N_equil * lambda        -- adv. add prob

	-- local add = 0 -- just to print statistics

	local inv_BBSnumber = 1.0 / #Game.system:GetStationPaths()

	-- remove with decay rate lambda. Call ONCE/hour for all adverts
	-- in system, thus normalize with #BBS, or we remove adverts with
	-- a probability a factor #BBS too high.
	for ref,ad in pairs(ads) do
		if Engine.rand:Number(0,1) < lambda * inv_BBSnumber then
			ad.station:RemoveAdvert(ref)
		end
	end

	local iter = 10
	local inv_iter = 1.0 / iter

	-- spawn new adverts, call for each BBS
    for i = 1,iter do
		if Engine.rand:Number(0,1) <= prod * Game.system.population * inv_iter then
			makeAdvert(station)
			-- add = add + 1
		end
	end

	if prod * Game.system.population * inv_iter >= 1 then
		print("Warning from Second Hand: too few ads spawning")
	end

	-- print(Game.time / (60*60*24), #utils.build_array(pairs(ads)),
	--		 Game.system.population, #Game.system:GetStationPaths(), add)
end


local loaded_data

local onGameStart = function ()
    ads = {}

    if not loaded_data or not loaded_data.ads then return end

    for k,ad in pairs(loaded_data.ads) do
        local ref = ad.station:AddAdvert({
			title       = flavours[ad.flavour].adtitle,
            description = ad.desc,
            icon        = "second_hand",
            onChat      = onChat,
            onDelete    = onDelete,
        })
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
Event.Register("onUpdateBB", onUpdateBB)

Serializer:Register("SecondHand", serialize, unserialize)
