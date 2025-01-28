


-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Event = require 'Event'
local Game = require 'Game'
local Engine = require 'Engine'
local Timer = require 'Timer'
local Serializer = require 'Serializer'
local Comms = require 'Comms'

local fuel = require 'pigui.libs.fuel-transfer'

-- store which station sent them out
local policeDispatched = false

local loaded_data


local onGameStart = function ()
	if (loaded_data) then
	    fuel.setComputerReserve(loaded_data.mainReserve, loaded_data.cargoReserve)
	end
	loaded_data = nil
end


local serialize = function ()
	local data = fuel.getComputerReserve()
	return data
end


local unserialize = function (data)
	loaded_data = data
end



local onEnterSystem = function (player)
    if not player:IsPlayer() then return end

	if (loaded_data) then
		    fuel.setComputerReserve(loaded_data.mainReserve, loaded_data.cargoReserve)
	end

	loaded_data = nil

	if(fuel.hasFuelComputerCapability()) then
	    fuel.computerTransfer()
	end
end




Event.Register("onGameStart", onGameStart)
Event.Register("onEnterSystem", onEnterSystem)
Event.Register("", onEnterSystem)


Serializer:Register("FuelComputer", serialize, unserialize)
