-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Lang = require 'Lang'
local Comms = require 'Comms'
local Event = require 'Event'

local l = Lang.GetResource("module-system")

local onShipFuelChanged = function (ship, state)
	if ship:IsPlayer() then
		if state == "WARNING" then
			Comms.ImportantMessage(l.YOUR_FUEL_TANK_IS_ALMOST_EMPTY)
		elseif state == "EMPTY" then
			Comms.ImportantMessage(l.YOUR_FUEL_TANK_IS_EMPTY)
			Event.Queue('onShipOutOfFuel', ship)
		end
	else
		if state == "EMPTY" then
			print(('{label} ({id}) out of fuel'):interp({label=ship.label,id=ship.shipId}))
			Event.Queue('onShipOutOfFuel', ship)
		end
	end
end

Event.Register("onShipFuelChanged", onShipFuelChanged)
