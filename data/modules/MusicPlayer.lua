local music = {}

local getCategoryForSong = function (name)
	if not name then return "" end
	local _, _, category = string.find(name, "^music/core/([%l-]+)/")
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
		repeat
			song = music[category][Engine.rand:Integer(1,#music[category])]
		until song ~= current_song
	end

	if Music.IsPlaying() then
		Music.FadeIn(song, 0.5, false)
	else
		Music.Play(song, false)
	end
end

-- handle separate planet/station-specific ambient music
local playAmbient = function ()
	local category

	-- if we're near a planet or spacestation then choose something specific
	-- player can usually be in a planet's frame but still be so far away that
	-- they'd say they're not near it, so only use its rotating frame (which
	-- only extends to about the radius of the atmosphere). orbital stations
	-- however are tiny so use their whole frame
	local near = Game.player.frameBody
	if near then
		if near:isa("Planet") and Game.player.frameRotating then
			category = "near-planet"
		elseif near:isa("SpaceStation") then
			category = "near-spacestation"
		end
	end

	-- not near anything interesting so just use the normal space music
	if not category then
		playRandomSongFromCategory("space")
		return
	end

	-- switch to the specific music. if the music doesn't start (ie we don't
	-- have any specific music) then fall back to normal space music
	playRandomSongFromCategory(category)
	if not Music.IsPlaying() then
		playRandomSongFromCategory("space")
	end
end

EventQueue.onGameStart:Connect(function () 
	music = {}

	-- get all the interesting songs by category
	local songs = Music.GetSongList()
	for n,key in pairs(songs) do
		local category = getCategoryForSong(key)
		if category then
			if not music[category] then music[category] = {} end
			table.insert(music[category], key)
		end
	end

	playAmbient()
end)

-- if a song finishes fall back to ambient music
EventQueue.onSongFinished:Connect(function ()
	playAmbient()
end)

-- start some ambient music when first arriving in system
EventQueue.onEnterSystem:Connect(function ()
	playAmbient()
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

-- player changed frame and might be near a planet or orbital station
EventQueue.onFrameChanged:Connect(function (body)
	if not body:isa("Ship") then return end
	if not body:IsPlayer() then return end

	playAmbient()
end)
