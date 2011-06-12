--~ If you really need to have these :) http://paahdin.com/projects/pioneer/lol.zip
local test = function()
	Music.Play("under");
end

local test2 = function(ship, alert)
	--todo: check alert level so this does not play so often
	Music.Play("knighty");
end

local test3 = function(ship, station)
	Music.FadeIn("love", 0.5)
end

local test4 = function(ship, station)
	Music.FadeIn("biisi", 0.5)
	--~ Music.FadeOut(0.5)
end

local test5 = function()
	Music.Play("tingle")
end

EventQueue.onGameStart:Connect(test)
EventQueue.onShipAlertChanged:Connect(test2)
EventQueue.onShipDocked:Connect(test3)
EventQueue.onShipUndocked:Connect(test4)
--~ EventQueue.onSongFinished:Connect(test5)

