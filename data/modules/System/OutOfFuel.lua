-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Comms = import("Comms")
local Translate = import("Translate")
local Event = import("Event")

local t = Translate:GetTranslator()

local onShipFuelChanged = function (ship, state)
	if ship:IsPlayer() then
		if state == "WARNING" then
			Comms.ImportantMessage(t('Your fuel tank is almost empty.'))
		elseif state == "EMPTY" then
			Comms.ImportantMessage(t('Your fuel tank is empty.'))
		end
	else
		if state == "EMPTY" then
			print(('{label} ({id}) out of fuel'):interp({label=ship.label,id=ship.shipId}))
		end
	end
end

Event.Register("onShipFuelChanged", onShipFuelChanged)
