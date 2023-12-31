-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Game = require 'Game'
local Music = require 'Music'
local Event = require 'Event'
local SystemPath = require 'SystemPath'

local MusicPlayer = {}

local music = {}

local inMapView = false

function MusicPlayer.getCategoryForSong(name)
	if not name then return "" end
	local _, _, category = string.find(name, "^music/core/([%l-]+)/")
	return category
end

function MusicPlayer.playRandomSongFromCategory(category, loop)
	-- if there's no song in the wanted category then do nothing
	if not music[category] then return end

	local current_song = Music.GetSongName()
	local current_category = MusicPlayer.getCategoryForSong(current_song)

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
		Music.FadeIn(song, 0.5, loop)
	else
		Music.Play(song, loop)
	end
end

-- handle separate planet/station-specific ambient music
local playAmbient = function ()
	local current_song = Music.GetSongName()
	local current_category = MusicPlayer.getCategoryForSong(current_song)
	if current_category == "menu" then
		Music.FadeOut(0.5)
	end
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
				MusicPlayer.playRandomSongFromCategory("unexplored")
			else
				MusicPlayer.playRandomSongFromCategory("space")
			end
		else
			MusicPlayer.playRandomSongFromCategory("unexplored")
		end
		return
	end

	-- switch to the specific music. if the music doesn't start (ie we don't
	-- have any specific music) then fall back to normal space music
	MusicPlayer.playRandomSongFromCategory(category)
	if not Music.IsPlaying() or current_category == "map-core" or current_category == "map-unexplored" then
		if Game.system then
			if Game.system:DistanceTo(sol) > unexplored_distance then
				MusicPlayer.playRandomSongFromCategory("unexplored")
			else
				MusicPlayer.playRandomSongFromCategory("space")
			end
		else
			MusicPlayer.playRandomSongFromCategory("unexplored")
		end
	end
end

function MusicPlayer.rebuildSongList()
	music = {}

	-- get all the interesting songs by category
	local songs = Music.GetSongList()
	for n,key in pairs(songs) do
		local category = MusicPlayer.getCategoryForSong(key)
		if category then
			if not music[category] then music[category] = {} end
			table.insert(music[category], key)
		end
	end
end

Event.Register("onGameStart", function ()
	MusicPlayer.rebuildSongList()
	if Game.player:GetDockedWith() and music["docked"] then
		MusicPlayer.playRandomSongFromCategory("docked")
	else
		playAmbient()
	end
end)

-- if a song finishes fall back to ambient music
-- unless the player is in a map view, in which case
-- we start map music instead
Event.Register("onSongFinished", function ()
	if Game.player:GetDockedWith() then
		if music["docked"] then
			MusicPlayer.playRandomSongFromCategory("docked")
		else
			playAmbient()
		end
	elseif Game.CurrentView() == "sector" or Game.CurrentView() == "system_info" or Game.CurrentView() == "system" then
		if Game.system and Game.system:DistanceTo(SystemPath.New(0, 0, 0, 0, 0)) < 1000 then -- farther than where ambient music switches
			if music["map-core"] then
				MusicPlayer.playRandomSongFromCategory("map-core")
			else -- fall back to ambient if category is empty
				playAmbient()
			end
		else
			if music["map-unexplored"] then
				MusicPlayer.playRandomSongFromCategory("map-unexplored")
			else -- fall back to ambient if category is empty
				playAmbient()
			end
		end
		inMapView = true
	else
		playAmbient()
	end
end)

-- play discovery music if player enters an interesting system
-- otherwise, start some ambient music
Event.Register("onEnterSystem", function ()
	local player_body = Game.player.frameBody
	if player_body and not Game.system.explored then
		if player_body.type == 'STAR_SM_BH' then
			MusicPlayer.playRandomSongFromCategory("discovery")
		elseif player_body.type == 'STAR_IM_BH' then
			MusicPlayer.playRandomSongFromCategory("discovery")
		elseif player_body.type == 'STAR_S_BH' then
			MusicPlayer.playRandomSongFromCategory("discovery")
		elseif player_body.type == 'STAR_O_WF' then
			MusicPlayer.playRandomSongFromCategory("discovery")
		elseif player_body.type == 'STAR_B_WF' then
			MusicPlayer.playRandomSongFromCategory("discovery")
		elseif player_body.type == 'STAR_M_WF' then
			MusicPlayer.playRandomSongFromCategory("discovery")
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
		MusicPlayer.playRandomSongFromCategory("player-destroyed")
	--elseif attacker:isa("Ship") and attacker:IsPlayer() then
	--	MusicPlayer.playRandomSongFromCategory("ship-destroyed")
	end
end)

-- player docked
Event.Register("onShipDocked", function (ship, station)
	if not ship:IsPlayer() then return end
	MusicPlayer.playRandomSongFromCategory("docked")
end)

-- player undocked
Event.Register("onShipUndocked", function (ship, station)
	if not ship:IsPlayer() then return end
	MusicPlayer.playRandomSongFromCategory("undocked")
end)

-- ship near the player
Event.Register("onShipAlertChanged", function (ship, alert)
	if not ship:IsPlayer() then return end
	if alert == "SHIP_NEARBY" and not ship:IsDocked() then
		MusicPlayer.playRandomSongFromCategory("ship-nearby")
	elseif alert == "SHIP_FIRING" then
		MusicPlayer.playRandomSongFromCategory("ship-firing", true)
	else
		playAmbient()
	end
end)

-- player changed frame and might be near a planet or orbital station
Event.Register("onFrameChanged", function (body)
	if Game.player:GetAlertState() == 'ship-firing' or Game.player:GetAlertState() == 'ship-nearby' then return end
	if not body:isa("Ship") then return end
	if not body:IsPlayer() then return end

	if not inMapView then
		playAmbient()
	end
end)

-- view has changed, so player might have left the map view
Event.Register("onViewChanged", function()
	if inMapView and Game.CurrentView() == "world" then
		playAmbient()
		inMapView = false
	end
end)

return MusicPlayer
