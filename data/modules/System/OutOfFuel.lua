local t = Translate:GetTranslator()

local onShipError = function (ship,errorcode)
	if ship:IsPlayer() then
		if errorcode=='OUT_OF_FUEL' then
			UI.ImportantMessage(t('Your fuel tank is empty.'))
		end
	else
		if errorcode=='OUT_OF_FUEL' then
			ship:SetFuelPercent() -- Quietly refuel NPCs, because they're stupid
		end
	end
end

EventQueue.onShipError:Connect(onShipError)
