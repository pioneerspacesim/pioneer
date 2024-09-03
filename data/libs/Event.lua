-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

--
-- Interface: Event
--
-- The majority of the work done by a Pioneer Lua module is in response to
-- events. The typical structure of a module will be to define a number of
-- event handler functions and then register them to receive a specific types
-- of event.
--
-- When events occur within the game, such as a ship docking or the player
-- coming under attack, an event is added to the event queue. At the end of
-- each physics frame, queued events are processed by calling the handlers
-- registered with for each type of event.
--
-- An event usually has one or more parameters attached to it that describe
-- the details of the event. For example, the <onShipDocked> event has two
-- parameters, the <Ship> that docked, and the <SpaceStation> it docked with.
--
-- Many of these events are triggered as a result of something happening to a
-- <Ship>. The same events are used when something happens to a <Player>
-- (which is also a <Ship>). Call the <Ship.IsPlayer> method on the <Ship> if
-- your module needs to know the difference.
--

local Engine = require 'Engine'
local utils  = require 'utils'

local do_callback_normal = function (cb, ev)
	cb.func(table.unpack(ev))
end
local do_callback_timed = function (cb, ev)
	local d = debug.getinfo(cb)

	local tstart = Engine.ticks
	cb.func(table.unpack(ev))
	local tend = Engine.ticks

	print(string.format("DEBUG: %s %dms %s:%d", ev.name, tend-tstart, d.source, d.linedefined))
end

---@class EventBase
---@field callbacks table
---@field debugger table
local Event = utils.inherits(nil, "Event")

function Event:Register(name, module, func)

	for _, cb in ipairs(self.callbacks[name]) do
		-- Update registered callback
		if cb.module == module then
			logWarning("Module {} overwriting event callback {}" % { module, func })
			cb.func = func
			return
		end
	end

	table.insert(self.callbacks[name], { module = module, func = func })
end

function Event:Deregister(name, module, func)
	for i, cb in ipairs(self.callbacks[name]) do
		if cb.module == module and cb.func == func then
			table.remove(self.callbacks[name], i)
			return
		end
	end
end

function Event:_Emit()
	while #self > 0 do
		local ev = table.remove(self, 1)
		local executor = self.debugger[ev.name] or do_callback_normal

		for _, cb in ipairs(self.callbacks[ev.name]) do
			executor(cb, ev)
		end
	end
end

function Event:_Dump()
	for ev, list in pairs(self.callbacks) do
		print("event: " .. ev)

		for _, v in ipairs(list) do
			print("\t{module} -> {func}" % v)
		end
	end
end

-- Note: this function is written this way to preserve the existing
-- interface to Event.Register(name, callback) as well as provide helpers
-- to automatically acquire module names when registering callbacks
Event.New = function()
	---@class EventQueue : EventBase
	local self = setmetatable({}, Event.meta)
	local super = Event

	self.callbacks = utils.automagic()
	self.debugger = {}

	--
	-- Function: Register
	--
	-- Register a function with a specific type of event. When an event with
	-- the named type is processed, the function will be called.
	--
	-- > Event.Register(name, function)
	--
	-- Parameters:
	--
	--   name - the name (type) of the event
	--
	--   function - function to call when an event of the named type is processed.
	--              The function will recieve a copy of the parameters attached to
	--              the event.
	--
	--
	-- Example:
	--
	-- > Event.Register("onEnterSystem", function (ship)
	-- >     print("welcome to "..Game.system.name..", "..ship.label)
	-- > end)
	--
	-- Availability:
	--
	--   alpha 26
	--
	-- Status:
	--
	--   stable
	--
	self.Register = function (name, cb)
		super.Register(self, name, package.modulename(2), cb)
	end

	--
	-- Function: Deregister
	--
	-- Deregisters a function from an event type. The funtion will no longer
	-- receive events of the named type.
	--
	-- If the function is not registered this method does nothing.
	--
	-- > Event.Deregister(name, function)
	--
	-- Parameters:
	--
	--   name - the name (type) of the event
	--
	--   function - a function that was previously connected to this queue with
	--              <Connect>
	--
	-- Availability:
	--
	--   alpha 26
	--
	-- Status:
	--
	--   stable
	--
	self.Deregister = function (name, cb)
		super.Deregister(self, name, package.modulename(2), cb)
	end

	--
	-- Function: Queue
	--
	-- Add an event to the queue of pending events. The event will be
	-- distributed to the handlers when the queue is processed.
	--
	-- > Event.Queue(name, ...)
	--
	-- Parameters:
	--
	--   name - the name (type) of the event
	--
	--   ... - zero or more arguments to be passed to the handlers
	--
	-- Example:
	--
	-- > Event.Queue("onEnterSystem", ship)
	--
	-- Availability:
	--
	--   alpha 26
	--
	-- Status:
	--
	--   stable
	--
	self.Queue = function (name, ...)
		table.insert(self, { name = name, ... })
	end

	--
	-- Function: DebugTimer
	--
	-- Enables the function timer for this event type. When enabled the console
	-- will display the amount of time that each handler for this event type
	-- takes to run.
	--
	-- > Event.DebugTimer(name, enabled)
	--
	-- Parameters:
	--
	--   name - name (type) of the event
	--
	--   enabled - a true value to enable the timer, or a false value to
	--             disable it.
	--
	-- Availability:
	--
	--   alpha 26
	--
	-- Status:
	--
	--   debug
    --

    self.DebugTimer = function (name, enabled)
		do_callback[name] = enabled and do_callback_timed or do_callback_normal
	end

	-- internal method, called from C++
	self._Clear = function ()
		for i = #self, 1, -1 do
			self[i] = nil
		end
	end

	-- internal method, called from C++
	self._Emit = function ()
		super._Emit(self)
	end

	return self
end

--
-- Event: onGameStart
--
-- Triggered when the game is first initialised.
--
-- > local onGameStart = function () ... end
-- > Event.Register("onGameStart", onGameStart)
--
-- onGameStart is triggered just after the physics <Body> objects (including
-- the <Player>) are placed.
--
-- This is a good place to add equipment to the Player ship or spawn objects
-- in the space. You should also initialise your module data for the game
-- here.
--
-- Availability:
--
--   alpha 10
--
-- Status:
--
--   stable
--

--
-- Event: onGameEnd
--
-- Triggered when game is finished.
--
-- > local onGameEnd = function () ... end
-- > Event.Register("onGameEnd", onGameEnd)
--
-- Triggered just before the physics <Body> objects (include the <Player>) are
-- destroyed and Pioneer returns to the main menu.
--
-- You should clean up any game data here. Remember that your module will be
-- re-used if the player begins another game. You probably don't want any data
-- from a previous game to leak into a new one.
--
-- Availability:
--
--   alpha 10
--
-- Status:
--
--   stable
--

--
-- Event: onEnterSystem
--
-- Triggered when a ship enters a system after hyperspace.
--
-- > local onEnterSystem = function (ship) ... end
-- > Event.Register("onEnterSystem", onEnterSystem)
--
-- This is the place to spawn pirates and other attack ships to give the
-- illusion that the ship was followed through hyperspace.
--
-- Note that this event is *not* triggered at game start.
--
-- Parameters:
--
--   ship - the <Ship> that entered the system
--
-- Availability:
--
--   alpha 10
--
-- Status:
--
--   stable
--

--
-- Event: onLeaveSystem
--
-- Triggered immediately before ship leaves a system and enters hyperspace.
--
-- > local onLeaveSystem = function (ship) ... end
-- > Event.Register("onLeaveSystem", onLeaveSystem)
--
-- If the ship was the player then all physics <Body> objects are invalid after
-- this method returns.
--
-- Parameters:
--
--   ship - the <Ship> that left the system
--
-- Availability:
--
--   alpha 10
--
-- Status:
--
--   stable
--

--
-- Event: onSystemExplored
--
-- Triggered when a system has just been explored by the player.
--
-- > local onSystemExplored = function (system) ... end
-- > Event.Register("onSystemExplored", onSystemExplored)
--
-- Parameters:
--
--   system - the <StarSystem> that has just been explored
--
-- Availability:
--
--   October 2014
--
-- Status:
--
--   experimental
--

--
-- Event: onFrameChanged
--
-- Triggered as a dynamic <Body> moves between frames of reference.
--
-- > local onFrameChanged = function (body) ... end
-- > Event.Register("onFrameChanged", onFrameChanged)
--
-- Details of the new frame itself can be obtained from the body's
-- <Body.frameBody> and <Body.frameRotating> attributes.
--
-- Parameters:
--
--   body - the dynamic <Body> that changed frames
--
-- Availability:
--
--   alpha 12
--
-- Status:
--
--   experimental
--


--
-- Event: onShipCreated
--
-- Triggered when a ship is pushed to Lua and its constructor is called.
-- This event may be used to start timers related to the ship or add optional
-- components to the ship object.
--
-- > local onShipCreated = function (ship) ... end
-- > Event.Register("onShipCreated", onShipCreated)
--
-- Parameters:
--
--   ship - the ship that was created
--
-- Availability:
--
--   June 2022
--
-- Status:
--
--   stable
--

--
-- Event: onShipDestroyed
--
-- Triggered when a ship is destroyed by gunfire or collision.
-- This event will not be triggered when a ship is simply deleted from space.
--
-- > local onShipDestroyed = function (ship, attacker) ... end
-- > Event.Register("onShipDestroyed", onShipDestroyed)
--
-- Parameters:
--
--   ship - the ship that was destroyed
--
--   attacker - the <Body> that was responsible for reducing the ship's hull
--   strength to 0. Usually a ship that fired lasers or missiles, but may be
--   another <Body> (eg <Planet> or <SpaceStation>) if the ship was destroyed
--   by a collision.
--
-- Availability:
--
--   alpha 10
--
-- Status:
--
--   stable
--

--
-- Event: onShipHit
--
-- Triggered when a ship is hit by laser fire or a missile.
--
-- > local onShipHit = function (ship, attacker) ... end
-- > Event.Register("onShipHit", onShipHit)
--
-- Parameters:
--
--   ship - the <Ship> that was hit
--
--   attacker - the <Ship> that fired the laser or missile
--
-- Availability:
--
--   alpha 10
--
-- Status:
--
--   stable
--

--
-- Event: onShipFiring
--
-- Triggered when a ship is firing its weapons.
--
-- > local onShipFiring = function (ship) ... end
-- > Event.Register("onShipFiring", onShipFiring)
--
-- Parameters:
--
--   ship - the <Ship> that is firing its weapons
--
-- Availability:
--
--   2014 May
--
-- Status:
--
--   experimental
--

--
-- Event: onShipCollided
--
-- Triggered when a ship collides with an object.
--
-- > local onShipCollided = function (ship, other) ... end
-- > Event.Register("onShipCollided", onShipCollided)
--
-- If the ship collides with a city building on a planet it will register as
-- as collision with the planet itself.
--
-- If the ship "collides" with a missile, <onShipHit> will be triggered
-- instead of <onShipCollided>.
--
-- If the ship hit another <Ship>, this event will be triggered twice, once
-- for each ship.
--
-- Parameters:
--
--   ship - the <Ship> that was hit
--
--   other - the <Body> that the ship collided with
--
-- Availability:
--
--   alpha 10
--
-- Status:
--
--   stable
--

--
-- Event: onShipDocked
--
-- Triggered when a ship docks with a space station.
--
-- > local onShipDocked = function (ship, station) ... end
-- > Event.Register("onShipDocked", onShipDocked)
--
-- Parameters:
--
--   ship - the <Ship> that docked
--
--   station - the <SpaceStation> the ship docked with
--
-- Availability:
--
--   alpha 10
--
-- Status:
--
--   stable
--

--
-- Event: onShipUndocked
--
-- Triggered when a ship undocks with a space station.
--
-- > local onShipUndocked = function (ship, station) ... end
-- > Event.Register("onShipUndocked", onShipUndocked)
--
-- Parameters:
--
--   ship - the <Ship> that docked
--
--   station - the <SpaceStation> the ship undocked with
--
-- Availability:
--
--   alpha 10
--
-- Status:
--
--   stable
--

--
-- Event: onShipLanded
--
-- Triggered when a ship performs a surface landing
-- (not on a spaceport).
--
-- > local onShipLanded = function (ship, body) ... end
-- > Event.Register("onShipLanded", onShipLanded)
--
-- Parameters:
--
--   ship - the <Ship> that landed
--
--   body - the <Body> the ship landed on
--
-- Availability:
--
--   alpha 13
--
-- Status:
--
--   experimental
--

--
-- Event: onShipTakeOff
--
-- Triggered when a ship takes off from a surface
-- (not from a spaceport).
--
-- > local onShipTakeOff = function (ship, body) ... end
-- > Event.Register("onShipTakeOff", onShipTakeOff)
--
-- Parameters:
--
--   ship - the <Ship> that took off
--
--   body - the <Body> the ship took off from
--
-- Availability:
--
--   alpha 13
--
-- Status:
--
--   experimental
--

--
-- Event: onShipAlertChanged
--
-- Triggered when a ship's alert status changes.
--
-- > local onShipAlertChanged = function (ship, alert) ... end
-- > Event.Register("onShipAlertChanged", onShipAlertChanged)
--
-- Parameters:
--
--   ship - the <Ship> that changed status
--
--   alert - the new <Constants.ShipAlertStatus>
--
--  Availability:
--
--    alpha 10
--
--  Status:
--
--    stable
--

--
-- Event: onJettison
--
-- Triggered when a ship jettisons a cargo item.
--
-- > local onJettison = function (ship, cargo) ... end
-- > Event.Register("onJettison", onJettison)
--
-- Parameters:
--
--   ship - the <Ship> that jettisoned the cargo item
--
--   cargo - the <CargoBody> now drifting in space
--
-- Availability:
--
--   alpha 10
--
-- Status:
--
--   experimental
--

--
-- Event: onCargoUnload
--
-- Triggered when the player unloads a cargo item while docked or landed.
--
-- > local onUnload = function (body, cargoType) ... end
-- > Event.Register("onCargoUnload", onCargoUnload)
--
-- Parameters:
--
--   body - the <Body> the <Player> was docked with (a <SpaceStation>) or landed on (a <Planet>)
--
--   cargoType - <EquipType> of the unloaded cargo
--
-- Availability:
--
--   alpha 18
--
-- Status:
--
--   experimental
--

--
-- Event: onAICompleted
--
-- Triggered when a ship AI completes
--
-- > local onAICompleted = function (ship, error) ... end
-- > Event.Register("onAICompleted", onAICompleted)
--
-- Parameters:
--
--   ship - the <Ship>
--
--   error - the <Constants.ShipAIError>
--
-- Availability:
--
--   alpha 10
--
-- Status:
--
--   stable
--

--
-- Event: onCreateBB
--
-- Triggered when a space station bulletin board is created.
--
-- > local onCreateBB = function (station) ... end
-- > Event.Register("onCreateBB", onCreateBB)
--
-- The usual function of a <onCreateBB> event handler is to call
-- <SpaceStation.AddAdvert> to populate the bulletin board with ads.
--
-- This event is triggered the first time a <Ship> docks with the station or
-- is placed directly into the station (eg by <Space.SpawnShipDocked> or at
-- game start)
--
-- Parameters:
--
--   station - the <SpaceStation> the bulletin board is being created for
--
-- Availability:
--
--   alpha 10
--
-- Status:
--
--   stable
--

--
-- Event: onUpdateBB
--
-- Triggered every 1-2 hours of game time to update the bulletin board.
--
-- > local onUpdateBB = function (station) ... end
-- > Event.Register("onUpdateBB", onUpdateBB)
--
-- The usual function of a <onUpdateBB> event handler is to call
-- <SpaceStation.AddAdvert> and <SpaceStation.RemoveAdvert> to update the
-- bulletin board.
--
-- This event is triggered at a random game time 1-2 hours after it was
-- previously called. This period is subject to change; script authors should
-- not rely on it.
--
-- <onUpdateBB> will never be triggered for a station that has not previously
-- had <onCreateBB> triggered for it.
--
-- Parameters:
--
--   station - the <SpaceStation> the bulletin board is being updated for
--
-- Availability:
--
--   alpha 10
--
-- Status:
--
--   stable
--

--
-- Event: onShipTypeChanged
--
-- Triggered when a ship's type changes (eg player buys a new ship).
--
-- > local onShipTypeChanged = function (ship) ... end
-- > Event.Register("onShipTypeChanged", onShipTypeChanged)
--
-- Parameters:
--
--   ship - the <Ship> whose type just changed
--
-- Availability:
--
--   alpha 32
--
-- Status:
--
--   experimental
--

--
-- Event: onShipEquipmentChanged
--
-- Triggered when a ship's equipment set changes.
--
-- > local onShipEquipmentChanged = function (ship, equipType) ... end
-- > Event.Register("onShipEquipmentChanged", onShipEquipmentChanged)
--
-- Parameters:
--
--   ship - the <Ship> whose equipment just changed
--
--   equipType - The <EquipType> item that was added or removed,
--   or nil if the change involved multiple types of equipment
--
-- Availability:
--
--   alpha 15
--
-- Status:
--
--   experimental
--

--
-- Event: onShipFuelChanged
--
-- Triggered when a ship's fuel status changes.
--
-- > local onShipFuelChanged = function (ship, fuelStatus) ... end
-- > Event.Register("onShipFuelChanged", onShipFuelChanged)
--
-- Parameters:
--
--   ship - the <Ship> whose fuel status just changed
--
--   fuelStatus - the new <Constants.PropulsionFuelStatus>
--
-- Availability:
--
--   alpha 20
--
-- Status:
--
--   experimental
--

--
-- Event: onShipScoopFuel
--
-- Triggered when a ship has stayed in the atmosphere of a star or gas giant
-- and a unit of fuel should be scooped into the players' cargo hold.
--
-- > local onShipScoopFuel = function (ship, body) ... end
-- > Event.Register("onShipScoopFuel", onShipScoopFuel)
--
-- Parameters:
--
--   ship - the <Ship> which just scooped fuel
--
--   body - the <SystemBody> the fuel was scooped from
--
-- Availability:
--
--   June 2022
--
-- Status:
--
--   experimental
--

--
-- Event: onShipScoopCargo
--
-- Triggered when a ship attempts to scoop up a cargo container.
--
-- Parameters:
--
--   ship - the <Ship> which attempted to scoop a cargo item
--
--   success - was the cargo container successfully scooped?
--
--   cargoType - the <CommodityType> contained in the cargo item
--
-- Availability:
--
--   June 2022
--
-- Status:
--
--   experimental
--

--
-- Event: onGamePaused
--
-- Triggered when the game is paused.
--
-- > local onGamePaused = function () ... end
-- > Event.Register("onGamePaused", onGamePaused)
--
-- Availability:
--
--   September 2014
--
-- Status:
--
--   experimental
--

--
-- Event: onGameResumed
--
-- Triggered when the game time accel (Game::GetTimeAccel) transitions from
-- TIMEACCEL_PAUSED to any other value.
--
-- > local onGameResumed = function () ... end
-- > Event.Register("onGameResumed", onGameResumed)
--
-- Availability:
--
--   September 2014
--
-- Status:
--
--   experimental
--

return Event.New()
