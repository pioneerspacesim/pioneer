-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Defs = require 'pigui.modules.new-game-window.defs'
local Crew = require 'pigui.modules.new-game-window.crew'
local Ship = require 'pigui.modules.new-game-window.ship'
local Location = require 'pigui.modules.new-game-window.location'
local Summary = require 'pigui.modules.new-game-window.summary'
local FlightLog = require 'pigui.modules.new-game-window.flight-log'

local Layout = {}

Layout.UpdateOrder = {

	Crew.Player.Char,
	Crew.Player.Money,
	Crew.Player.Reputation,
	Crew.Player.Kills,
	Crew,
	Ship.Type,
	Ship.Name,
	Ship.Label,
	Ship.Model,
	Ship.Cargo,
	Ship.Equip,
	Ship.Fuel,
	Location,
	Location.Time,
	FlightLog,
	Summary.Description
}

Layout.Tabs = { Summary, Crew, Ship, Location, FlightLog }

function Layout.updateLayout(contentRegion)

	Defs.updateLayoutValues(contentRegion)

	for _, tab in ipairs(Layout.Tabs) do
		if tab.updateLayout then tab:updateLayout() end
	end
end

function Layout.setLock(lock)
	for _, param in ipairs(Layout.UpdateOrder) do
		param.lock = lock
	end
end

return Layout
