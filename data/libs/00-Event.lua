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

local pending = {}
local callbacks = {}
local do_callback = {}

local do_callback_normal = function (cb, p)
	cb(table.unpack(p.event))
end
local do_callback_timed = function (cb, p)
	local d = debug.getinfo(cb)

	local tstart = Engine.ticks
	cb(table.unpack(p.event))
	local tend = Engine.ticks

	print(string.format("DEBUG: %s %dms %s:%d", p.name, tend-tstart, d.source, d.linedefined))
end

Event = {
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
	Register = function (name, cb)
		if not callbacks[name] then callbacks[name] = {} end
		callbacks[name][cb] = cb;
        if not do_callback[name] then do_callback[name] = do_callback_normal end
	end,

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
	Deregister = function (name, cb)
		if not callbacks[name] then return end
		callbacks[name][cb] = nil
	end,

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
	Queue = function (name, ...)
		table.insert(pending, { name = name, event = {...} })
	end,

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

    DebugTimer = function (name, enabled)
		do_callback[name] = enabled and do_callback_timed or do_callback_normal
	end,

	-- internal method, called from C++
	_Clear = function ()
		pending = {}
	end,

	-- internal method, called from C++
	_Emit = function ()
		while #pending > 0 do
			local p = table.remove(pending, 1)
			if callbacks[p.name] then
				for cb,_ in pairs(callbacks[p.name]) do
					do_callback[p.name](cb, p)
				end
			end
		end
	end
}

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
-- Event: onShipDestroyed
--
-- Triggered when a ship is destroyed.
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
-- Event: onShipFlavourChanged
--
-- Triggered when a ship's type, registration or graphical flavour changes.
--
-- > local onShipFlavourChanged = function (ship) ... end
-- > Event.Register("onShipFlavourChanged", onShipFlavourChanged)
--
-- Parameters:
--
--   ship - the <Ship> whose type or graphical flavour just changed
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
--   equipType - the string ID of the <EquipType> that was added or removed,
--   or 'NONE' if the change involved multiple types of equipment
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
--   fuelStatus - the new <Constants.ShipFuelStatus>
--
-- Availability:
--
--   alpha 20
--
-- Status:
--
--   experimental
--

-- XXX document SongFinished
