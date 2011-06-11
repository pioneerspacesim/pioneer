local test = function()
	Music.Play("biisi");
end

local test2 = function(ship, alert)
	Music.Play("knighty");
end

EventQueue.onGameStart:Connect(test)
EventQueue.onShipAlertChanged:Connect(test2)
