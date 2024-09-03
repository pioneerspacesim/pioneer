-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local Space = require 'Space'
local Event = require 'Event'
local Comms = require 'Comms'
local Timer = require 'Timer'
local Lang = require 'Lang'
local utils = require 'utils'

local l = Lang.GetResource("module-system")

local exploreSystem = function (system)
	Comms.Message(l.GETTING_SENSOR_DATA)
	local starports = #Space.GetBodies("SpaceStation")
	local major_bodies = #Space.GetBodies("TerrainBody")
	local bodies
	if major_bodies == 1 then
		bodies = l.BODY
	else
		bodies = l.BODIES
	end
	Timer:CallAt(Game.time+major_bodies, function ()
		system:Explore()
		Comms.Message(l.EXPLORED_SYSTEM:interp({bodycount=major_bodies, bodies=bodies}))
	end)
	if starports > 0 then
		local bases
		if starports == 1 then
			bases = l.BASE
		else
			bases = l.BASES
		end
		Timer:CallAt(Game.time+major_bodies+starports, function ()
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
