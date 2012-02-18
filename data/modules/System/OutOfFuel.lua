local t = Translate:GetTranslator()

local onShipFuelEmpty = function (ship)
	if ship:IsPlayer() then
		UI.ImportantMessage(t('Your fuel tank is empty.'))
	else
		ship:SetFuelPercent() -- Quietly refuel NPCs, because they're stupid
	end
end

EventQueue.onShipFuelEmpty:Connect(onShipFuelEmpty)
