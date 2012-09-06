local VERSION = 1 -- Integer versioning; bump this up if the saved game format changes.

local loaded

local spawnShips = function ()
	local population = Game.system.population

	if population == 0 then
		return
	end

	local stations = Space.GetBodies(function (body) return body:isa("SpaceStation") end)
	if #stations == 0 then
		return
	end

	local shiptypes = ShipType.GetShipTypes('STATIC_SHIP')
	if #shiptypes == 0 then return end

	--[[
	assuming these are huge supply ships and not your run-of-the-mill
	traders and not warships or whatever, we'll do it like this:

	- first one is free
	- one ship per billion up to 4 billion
	- one ship per 5 billion after that
	]]

	local num_bulk_ships = 1
	while population > 1 do
		if num_bulk_ships < 4 then
			population = population-1
			num_bulk_ships = num_bulk_ships+1
		elseif population > 5 then
			population = population-5
			num_bulk_ships = num_bulk_ships+1
		else
			break
		end
	end

	for i=1, num_bulk_ships do
	local station = stations[Engine.rand:Integer(1,#stations)]
		Space.SpawnShipParked(shiptypes[Engine.rand:Integer(1,#shiptypes)], station)
	end
end

local onEnterSystem = function (player)
	if not player:IsPlayer() then return end

	spawnShips()
end

local onGameStart = function ()
	if loaded == nil then
		spawnShips()
	end
	loaded = nil
end

local serialize = function ()
    return {
		VERSION = VERSION,
	}
end

local unserialize = function (data)
	loaded = true
	if data.VERSION then
		if data.VERSION < VERSION then
			print('Old BulkShips data loaded, converting...')
			-- No upgrade code yet
			print(('BulkShips data converted to internal version {newversion}'):interp({newversion=VERSION}))
			return
		end
		if data.VERSION > VERSION then
			error(([[BulkShips load error - saved game is more recent than installed files
			Saved game internal version: {saveversion}
			Installed internal version: {ourversion}]]):interp({saveversion=data.VERSION,ourversion=VERSION}))
		end
	else
		-- Hopefully, a few engine save-game bumps from now,
		-- there will be no instance where this is acceptable,
		-- and we can error() out of here.
		print('Pre-versioning BulkShips data loaded')
	end
end

Event.Register("onEnterSystem", onEnterSystem)
Event.Register("onGameStart", onGameStart)

Serializer:Register("BulkShips", serialize, unserialize)
