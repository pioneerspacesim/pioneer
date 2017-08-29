-- Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Lang = import("Lang")
local Game = import("Game")
local Comms = import("Comms")
local Event = import("Event")
local Format = import("Format")

local l = Lang.GetResource("module-stationrefuelling")

local minimum_fee = 1
local maximum_fee = 6

local calculateFee = function ()
	local fee = math.ceil(Game.system.population * 3)
	if fee < minimum_fee then
		return minimum_fee
	elseif fee > maximum_fee then
		return maximum_fee
	else
		return fee
	end
end


local onShipDocked = function (ship, station)
	if not ship:IsPlayer() then
		ship:SetFuelPercent() -- refuel NPCs for free.
		return
	end
	local fee = calculateFee()
	if ship:GetMoney() < fee then
		Comms.Message(l.THIS_IS_STATION_YOU_DO_NOT_HAVE_ENOUGH:interp({station = station.label,fee = Format.Money(fee)}))
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
