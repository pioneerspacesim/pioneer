-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Translate = import("Translate")
local Event = import("Event")
local Character = import("Character")
local Comms = import("Comms")

-- Stats-gathering module. Initially, gathers kill statistics for the player.
-- Can (and should) be expanded in the future to gather other information.

-- The information gathered here is stored in the player's character sheet.
-- This is globally available to all Lua scripts. Retrieval methods should
-- be implemented as part of Characters.lua.

-- This is used for tracking which ships were damaged by the player, so that
-- we can award assists as well as kills. One day, assists might contribute to
-- the combat rating.
local PlayerDamagedShips = {}

-- We need this to translate "Right on, Commander" etc
-- Until dependencies are fixed, this can't move to data/libs/
local t = Translate:GetTranslator()


local onShipDestroyed = function (ship, attacker)
	if attacker:isa('Ship') and attacker:IsPlayer() then
		-- Increment player's kill count
		Character.persistent.player.killcount = Character.persistent.player.killcount + 1
		PlayerDamagedShips[ship]=nil
		if Character.persistent.player.killcount == 1 or (Character.persistent.player.killcount < 256 and Character.persistent.player.killcount % 16 == 0) or (Character.persistent.player.killcount % 256 == 0) then
			-- On the first kill, every 16th kill until 256, and every 256th
			-- kill thereafter
			Comms.Message(t("Well done commander! Your combat rating has improved!"),t("Pioneering Pilots' Guild"))
		end
	elseif PlayerDamagedShips[ship] then
		Character.persistent.player.assistcount = Character.persistent.player.assistcount + 1
	end
end

local onShipHit = function (ship, attacker)
	if attacker:isa('Ship') and attacker:IsPlayer() then
		PlayerDamagedShips[ship]=true
	end
end

Event.Register("onShipDestroyed",onShipDestroyed)
--Commented out pending issue #887
--Event.Register("onShipHit",onShipHit)
