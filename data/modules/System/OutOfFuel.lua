local t = Translate:GetTranslator()

local onShipFuelChanged = function (ship, state)
	if ship:IsPlayer() then
		if state == "WARNING" then
			UI.ImportantMessage(t('Your fuel tank is almost empty.'))
		elseif state == "EMPTY" then
			UI.ImportantMessage(t('Your fuel tank is empty.'))
		end
	else
		ship:SetFuelPercent() -- Quietly refuel NPCs, because they're stupid
	end
end

EventQueue.onShipFuelChanged:Connect(onShipFuelChanged)
