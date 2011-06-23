local music = {}

local getCategoryForSong = function (name)
	if not name then return "" end
	local _, _, category = string.find(name, "^core/([%l-]+)/")
	return category
end

local playRandomSongFromCategory = function (category)
	-- if there's no song in the wanted category then do nothing
	if not music[category] then return end

	local current_song = Music.GetSongName()
	local current_category = getCategoryForSong(current_song)

	if Music.IsPlaying() then
		-- no category means some other script is playing something and we
		-- don't want to override that
		if not current_category then return end

		-- don't change song if we're already playing something from this category
		if current_category == category then return end
	end

	-- choosing a song
	local song = ""

	-- if the category only has one song then the choice is simple
	if #music[category] == 1 then
		song = music[category][1]
	
	-- more than one so select at random
	-- XXX base on system seed so you get the starting point for the system
	else
		-- don't choose the song currently playing (or last played)
		while song ~= current_song do
			song = music[category][Engine.rand:Integer(1,#music[category])]
		end
	end

	if Music.IsPlaying() then
		Music.FadeIn(song, 0.5, false)
	else
		Music.Play(song, false)
	end
end

EventQueue.onGameStart:Connect(function () 
	music = {}

	-- get all the interesting songs by category
	songs = Music.GetSongList()
	for n,key in pairs(songs) do
		local category = getCategoryForSong(key)
		if category then
			if not music[category] then music[category] = {} end
			table.insert(music[category], key)
		end
	end

	playRandomSongFromCategory("ambient")
end)

-- if a song finishes fall back to ambient
EventQueue.onSongFinished:Connect(function ()
	playRandomSongFromCategory("ambient")
end)

-- start some ambient music when first arriving in system
EventQueue.onEnterSystem:Connect(function ()
	playRandomSongFromCategory("ambient")
end)

-- ship or player destruction (aka game over)
EventQueue.onShipDestroyed:Connect(function (ship, attacker)
	if ship:IsPlayer() then
		playRandomSongFromCategory("player-destroyed")
	elseif attacker:isa("Ship") and attacker:IsPlayer() then
		playRandomSongFromCategory("ship-destroyed")
	end
end)

-- player docked
EventQueue.onShipDocked:Connect(function (ship, station)
	if not ship:IsPlayer() then return end
	playRandomSongFromCategory("docked")
end)

-- player undocked
EventQueue.onShipUndocked:Connect(function (ship, station)
	if not ship:IsPlayer() then return end
	playRandomSongFromCategory("undocked")
end)

-- ship near the player
EventQueue.onShipAlertChanged:Connect(function (ship, alert)
	if not ship:IsPlayer() then return end
	if alert == "SHIP_NEARBY" then
		playRandomSongFromCategory("ship-nearby")
	elseif alert == "SHIP_FIRING" then
		playRandomSongFromCategory("ship-firing")
	end
end)

-- player near a planet surface or orbital station
EventQueue.onFrameChanged:Connect(function (body)
	if not body:isa("Ship") then return end
	if not body:IsPlayer() then return end
	if not body.frameRotating then return end

	local near = body.frameBody
	if near:isa("Planet") then
		playRandomSongFromCategory("near-planet")
	elseif near:isa("SpaceStation") then
		playRandomSongFromCategory("near-spacestation")
	end
end)
