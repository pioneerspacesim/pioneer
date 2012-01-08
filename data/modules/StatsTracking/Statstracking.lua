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
		PersistentCharacters.player.killcount = PersistentCharacters.player.killcount + 1
		PlayerDamagedShips[ship]=nil
	elseif PlayerDamagedShips[ship] then
		PersistentCharacters.player.assistcount = PersistentCharacters.player.assistcount + 1
	end
end

local onShipHit = function (ship, attacker)
	if attacker:isa('Ship') and attacker:IsPlayer() then
		PlayerDamagedShips[ship]=true
	end
end

EventQueue.onShipDestroyed:Connect(onShipDestroyed)
--EventQueue.onShipHit:Connect(onShipHit)
