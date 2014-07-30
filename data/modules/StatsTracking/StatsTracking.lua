-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Event = import("Event")
local Character = import("Character")
local Comms = import("Comms")
local Lang = import("Lang")

local l = Lang.GetResource("module-statstracking")

-- Stats-gathering module. Initially, gathers kill statistics for the player.
-- Can (and should) be expanded in the future to gather other information.

-- The information gathered here is stored in the player's character sheet.
-- This is globally available to all Lua scripts. Retrieval methods should
-- be implemented as part of Characters.lua.

-- This is used for tracking which ships were damaged by the player, so that
-- we can award assists as well as kills. One day, assists might contribute to
-- the combat rating.
local PlayerDamagedShips = {}


local onShipDestroyed = function (ship, attacker)
	if attacker:isa('Ship') and attacker:IsPlayer() then
		-- Increment player's kill count
		Character.persistent.player.killcount = Character.persistent.player.killcount + 1
		PlayerDamagedShips[ship]=nil
		if Character.persistent.player.killcount == 1 or (Character.persistent.player.killcount < 256 and Character.persistent.player.killcount % 16 == 0) or (Character.persistent.player.killcount % 256 == 0) then
			-- On the first kill, every 16th kill until 256, and every 256th
			-- kill thereafter
			Comms.Message(l.WELL_DONE_COMMANDER_YOUR_COMBAT_RATING_HAS_IMPROVED,l.PIONEERING_PILOTS_GUILD)
		end
		Event.Queue("onReputationChanged", Character.persistent.player.reputation, Character.persistent.player.killcount - 1,
			Character.persistent.player.reputation, Character.persistent.player.killcount)
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
