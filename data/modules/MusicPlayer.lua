-- Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Game = import("Game")
local Music = import("Music")
local Event = import("Event")
local SystemPath = import("SystemPath")

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

	local sol = SystemPath.New(0, 0, 0, 0, 0)
	local unexplored_distance = 690

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
		if Game.system then
			if Game.system:DistanceTo(sol) > unexplored_distance then
				playRandomSongFromCategory("unexplored")
			else
				playRandomSongFromCategory("space")
			end
		else
			playRandomSongFromCategory("unexplored")
		end
		return
	end

	-- switch to the specific music. if the music doesn't start (ie we don't
	-- have any specific music) then fall back to normal space music
	playRandomSongFromCategory(category)
	if not Music.IsPlaying() then
		if Game.system then
			if Game.system:DistanceTo(sol) > unexplored_distance then
				playRandomSongFromCategory("unexplored")
			else
				playRandomSongFromCategory("space")
			end
		else
			playRandomSongFromCategory("unexplored")
		end
	end
end

Event.Register("onGameStart", function ()
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
Event.Register("onSongFinished", function ()
	playAmbient()
end)

-- play discovery music if player enters an interesting system
-- otherwise, start some ambient music
Event.Register("onEnterSystem", function ()
	local player_body = Game.player.frameBody
	if player_body and not Game.system.explored then
		if player_body.type == 'STAR_SM_BH' then 
			playRandomSongFromCategory("discovery")
		elseif player_body.type == 'STAR_IM_BH' then
			playRandomSongFromCategory("discovery")
		elseif player_body.type == 'STAR_S_BH' then
			playRandomSongFromCategory("discovery")
		elseif player_body.type == 'STAR_O_WF' then
			playRandomSongFromCategory("discovery")
		elseif player_body.type == 'STAR_B_WF' then
			playRandomSongFromCategory("discovery")
		elseif player_body.type == 'STAR_M_WF' then
			playRandomSongFromCategory("discovery")
		else
			playAmbient()
		end
	else
		playAmbient()
	end
end)

-- ship or player destruction (aka game over)
Event.Register("onShipDestroyed", function (ship, attacker)
	if ship:IsPlayer() then
		playRandomSongFromCategory("player-destroyed")
	elseif attacker:isa("Ship") and attacker:IsPlayer() then
		playRandomSongFromCategory("ship-destroyed")
	end
end)

-- player docked
Event.Register("onShipDocked", function (ship, station)
	if not ship:IsPlayer() then return end
	playRandomSongFromCategory("docked")
end)

-- player undocked
Event.Register("onShipUndocked", function (ship, station)
	if not ship:IsPlayer() then return end
	playRandomSongFromCategory("undocked")
end)

-- ship near the player
Event.Register("onShipAlertChanged", function (ship, alert)
	if not ship:IsPlayer() then return end
	if alert == "SHIP_NEARBY" and not ship:IsDocked() then
		playRandomSongFromCategory("ship-nearby")
	elseif alert == "SHIP_FIRING" then
		playRandomSongFromCategory("ship-firing")
	end
end)

-- player changed frame and might be near a planet or orbital station
Event.Register("onFrameChanged", function (body)
	if not body:isa("Ship") then return end
	if not body:IsPlayer() then return end

	playAmbient()
end)
