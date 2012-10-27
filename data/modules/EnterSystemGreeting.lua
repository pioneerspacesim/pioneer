-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local loaded

local welcomeHome = function ()
	local faction = Game.system.faction

	if faction == nil then
		Comms.ImportantMessage('No faction controls this system... take it whilst it\'s hot!')
	else
		Comms.ImportantMessage('This star system is under ' .. faction.name .. ' control.')
	end
end

local onEnterSystem = function (player)
	if not player:IsPlayer() then return end

	welcomeHome()
end

local onGameStart = function ()
	if loaded == nil then
		welcomeHome()
	end
	loaded = nil
end

local serialize = function ()
	return true
end

local unserialize = function (data)
	loaded = true
end

Event.Register("onEnterSystem", onEnterSystem)
Event.Register("onGameStart", onGameStart)

Serializer:Register("EnterSystemGreeting", serialize, unserialize)
