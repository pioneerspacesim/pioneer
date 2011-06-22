local music = {}

local onGameStart = function () 
	music = {}
	songs = Music.GetSongList()
	for n,key in pairs(songs) do
		local _, _, category = string.find(key, "^core/([%l-]+)/")
		if category then
			if not music[category] then music[category] = {} end
			table.insert(music[category], key)
		end
	end

	for category,list in pairs(music) do
		print(category)
		for n,key in pairs(list) do
			print("  "..key)
		end
	end
end

EventQueue.onGameStart:Connect(onGameStart)

--[[
--~ If you really need to have these :) http://paahdin.com/projects/pioneer/lol.zip
local test = function()
	Music.Play("under");
	print("Available songs:\n--------------")
	songs = Music.GetSongList()
	for key,value in pairs(songs) do
		print(key, value)
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
	Music.Play("biisi")
end

local test6 = function(ship, body)
	if ship:IsPlayer() then
		Music.Play("gameover") --boo
	else
		Music.Play("tingle", false) --hooray
	end
	--Music.FadeOut(0.5)
end

EventQueue.onGameStart:Connect(test)
EventQueue.onShipAlertChanged:Connect(test2)
EventQueue.onShipDocked:Connect(test3)
EventQueue.onShipUndocked:Connect(test4)
EventQueue.onSongFinished:Connect(test5)
EventQueue.onShipDestroyed:Connect(test6)
--]]
