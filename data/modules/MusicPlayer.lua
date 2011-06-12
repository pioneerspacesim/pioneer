local test = function()
	Music.Play("under");
end

local test2 = function(ship, alert)
	Music.Play("knighty");
end

local test3 = function(ship, station)
	Music.Play("love")
end

local test4 = function(ship, station)
	Music.Play("biisi")
end

local test5 = function()
	Music.Play("tingle")
end

EventQueue.onGameStart:Connect(test)
EventQueue.onShipAlertChanged:Connect(test2)
EventQueue.onShipDocked:Connect(test3)
EventQueue.onShipUndocked:Connect(test4)
--~ EventQueue.onSongFinished:Connect(test5)

