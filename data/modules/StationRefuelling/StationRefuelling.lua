-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Lang = require 'Lang'
local Game = require 'Game'
local Comms = require 'Comms'
local Event = require 'Event'
local Format = require 'Format'

local l = Lang.GetResource("module-stationrefuelling")

local minimum_fee = 1
local maximum_fee = 6

local calculateFee = function ()
	local fee = math.ceil(Game.system.population * 3)
	return math.clamp(fee, minimum_fee, maximum_fee)
end


local onShipDocked = function (ship, station)
	if not ship:IsPlayer() then
		ship:SetFuelPercent() -- refuel NPCs for free.
		return
	end

	local fee = calculateFee()
	if ship:GetMoney() < fee then
		if station.isGroundStation == true then
			Comms.Message(l.THIS_IS_STATION_YOU_DO_NOT_HAVE_ENOUGH_LANDING:interp({station = station.label,fee = Format.Money(fee)}))
		else
			Comms.Message(l.THIS_IS_STATION_YOU_DO_NOT_HAVE_ENOUGH_DOCKING:interp({station = station.label,fee = Format.Money(fee)}))
		end
		ship:SetMoney(0)
	else
		if station.isGroundStation == true then
			Comms.Message(l.WELCOME_TO_STATION_FEE_DEDUCTED:interp({station = station.label,fee = Format.Money(fee)}))
		else
			Comms.Message(l.WELCOME_ABOARD_STATION_FEE_DEDUCTED:interp({station = station.label,fee = Format.Money(fee)}))
		end
		ship:AddMoney(0 - fee)
	end
end

Event.Register("onShipDocked", onShipDocked)
