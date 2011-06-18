--~ If you really need to have these :) http://paahdin.com/projects/pioneer/lol.zip
local test = function()
	Music.Play("under");
	print("Available songs:\n--------------")
	songs = Music.GetSongList()
	for key,value in pairs(songs) do
		print(value)
	end
end

local test2 = function(ship, alert)
	if not ship:IsPlayer() then return end
	if alert == 'SHIP_NEARBY' then
		Music.Play("knighty");
	end
end

local test3 = function(ship, station)
	if not ship:IsPlayer() then return end
	Music.FadeIn("love", 0.5)
end

local test4 = function(ship, station)
	if not ship:IsPlayer() then return end
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

