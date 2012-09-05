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
			print(('{label} ({type}) out of fuel'):interp({label=ship.label,type=ship.shipType}))
		end
	end
end

Event.Register("onShipFuelChanged", onShipFuelChanged)
