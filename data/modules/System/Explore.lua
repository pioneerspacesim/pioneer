-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = import("Game")
local Space = import("Space")
local Event = import("Event")
local Comms = import("Comms")
local Timer = import("Timer")
local Lang = import("Lang")

local l = Lang.GetResource("module-system")

local exploreSystem = function (system)
	system:Explore()
	local starports = #Space.GetBodies(function (body) return body.superType == 'STARPORT' end)
	local major_bodies = #Space.GetBodies(function (body) return body.superType and body.superType ~= 'STARPORT' and body.superType ~= 'NONE' end)
	local bodies
	if major_modies == 1 then
		bodies = l.BODY
	else
		bodies = l.BODIES
	end
	Comms.Message(l.EXPLORING_SYSTEM:interp({bodycount=major_bodies, bodies=bodies}))
	if starports > 0 then
		local bases
		if starports == 1 then
			bases = l.BASE
		else
			bases = l.BASES
		end
		Timer:CallAt(Game.time+5, function ()
				Comms.ImportantMessage(l.DISCOVERED_HIDDEN_BASES:interp({portcount=starports, bases=bases}))
			end)
	end
end

local onEnterSystem = function (player)
	if not player:IsPlayer() then return end

	if not Game.system.explored then
		exploreSystem(Game.system)
	end
end

Event.Register("onEnterSystem", onEnterSystem)
