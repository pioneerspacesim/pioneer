-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

--
-- Interface: Player
--
-- Functions for interacting with the Player.
--

---@class Player
local Player = package.core["Player"]

local Engine = require 'Engine'
local Event = require 'Event'
local Game = require 'Game'

local onEnterSystem = function (ship)
	-- Return to game view when we exit hyperspace
	if Engine.GetResetViewOnHyperspaceExit() and Game.CurrentView() ~= "WorldView" then
		Game.SetView("WorldView")
	end
end

Event.Register("onEnterSystem", onEnterSystem)

return Player
