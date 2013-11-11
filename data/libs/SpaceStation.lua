-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local SpaceStation = import_core("SpaceStation")
local Event = import("Event")
local Space = import("Space")
local utils = import("utils")
local ShipDef = import("ShipDef")
local Engine = import("Engine")
local Timer = import("Timer")
local Game = import("Game")
local Ship = import("Ship")
local ModelSkin = import("SceneGraph.ModelSkin")

--
-- Class: SpaceStation
--

local shipsOnSale = {}

function SpaceStation:GetShipsOnSale ()
	if not shipsOnSale[self] then shipsOnSale[self] = {} end
	return shipsOnSale[self]
end

local function addShipOnSale (station, entry)
	if not shipsOnSale[station] then shipsOnSale[station] = {} end
	table.insert(shipsOnSale[station], entry)
end

function SpaceStation:AddShipOnSale (entry)
	assert(entry.def)
	assert(entry.skin)
	assert(entry.label)
	addShipOnSale(self, {
		def   = entry.def,
		skin  = entry.skin,
		label = entry.label
	})
	Event.Queue("onShipMarketUpdate", self, shipsOnSale[self])
end

local function removeShipOnSale (station, num)
	if not shipsOnSale[station] then shipsOnSale[station] = {} end
	table.remove(shipsOnSale[station], num)
end

local function findShipOnSale (station, entry)
	if not shipsOnSale[station] then shipsOnSale[station] = {} end
	local num = 0
	for k,v in pairs(shipsOnSale[station]) do
		if v == entry then
			num = k
			break
		end
	end
	return num
end

function SpaceStation:RemoveShipOnSale (entry)
	local num = findShipOnSale(self, entry)
	if num > 0 then
		removeShipOnSale(self, num)
		Event.Queue("onShipMarketUpdate", self, shipsOnSale[self])
	end
end

function SpaceStation:ReplaceShipOnSale (old, new)
	assert(new.def)
	assert(new.skin)
	assert(new.label)
	local num = findShipOnSale(self, old)
	if num <= 0 then
		self:AddShipOnSale(new)
	else
		shipsOnSale[self][num] = {
			def   = new.def,
			skin  = new.skin,
			label = new.label,
		}
	end
	Event.Queue("onShipMarketUpdate", self, shipsOnSale[self])
end

local groundShips = utils.build_array(utils.filter(function (k,def) return def.tag == "SHIP" and def.equipSlotCapacity.ATMOSHIELD > 0 end, pairs(ShipDef)))
local spaceShips  = utils.build_array(utils.filter(function (k,def) return def.tag == "SHIP" end, pairs(ShipDef)))

local function updateShipsOnSale (station)
	if not shipsOnSale[station] then shipsOnSale[station] = {} end
	local shipsOnSale = shipsOnSale[station]

	local toAdd, toRemove = 0, 0
	if #shipsOnSale == 0 then
		toAdd = Engine.rand:Integer(20)
	elseif Engine.rand:Integer(2) > 0 then
		toAdd = 1
	elseif #shipsOnSale > 0 then
		toRemove = 1
	else
		return
	end

	if toAdd > 0 then
		local avail = station.type == "STARPORT_SURFACE" and groundShips or spaceShips
		for i=1,toAdd do
			addShipOnSale(station, {
				def   = avail[Engine.rand:Integer(1,#avail)],
				skin  = ModelSkin.New():SetRandomColors(Engine.rand),
				label = Ship.MakeRandomLabel(),
			})
		end
	end

	if toRemove > 0 then
		removeShipOnSale(station, Engine.rand:Integer(1,#shipsOnSale))
	end

	Event.Queue("onShipMarketUpdate", station, shipsOnSale)
end


--
-- Group: Methods
--


SpaceStation.adverts = {}

--
-- Method: AddAdvert
--
-- Add an advertisement to the station's bulletin board
--
-- > ref = station:AddAdvert(description, chatfunc, deletefunc)
--
-- Parameters:
--
--   description - text to display in the bulletin board
--
--   chatfunc - function to call when the ad is activated. The function is
--              passed three parameters: a <ChatForm> object for the ad
--              conversation display, the ad reference returned by <AddAdvert>
--              when the ad was created, and an integer value corresponding to
--              the action that caused the activation. When the ad is initially
--              selected from the bulletin board, this value is 0. Additional
--              actions (and thus values) are defined by the script via
--              <ChatForm.AddAction>.
--
--   deletefunc - optional. function to call when the ad is removed from the
--                bulletin board. This happens when <RemoveAdvert> is called,
--                when the ad is cleaned up after
--                <ChatForm.RemoveAdvertOnClose> is called, and when the
--                <SpaceStation> itself is destroyed (eg the player leaves the
--                system).
--
-- Return:
--
--   ref - an integer value for referring to the ad in the future. This value
--         will be passed to the ad's chat function and should be passed to
--         <RemoveAdvert> to remove the ad from the bulletin board.
--
-- Example:
--
-- > local ref = station:AddAdvert(
-- >     "FAST SHIP to deliver a package to the Epsilon Eridani system.",
-- >     function (ref, opt) ... end,
-- >     function (ref) ... end
-- > )
--
-- Availability:
--
--   alpha 10
--
-- Status:
--
--   stable
--
local nextRef = 0
function SpaceStation:AddAdvert (description, chatFunc, deleteFunc)
	if not SpaceStation.adverts[self] then SpaceStation.adverts[self] = {} end
	local adverts = SpaceStation.adverts[self]
	nextRef = nextRef+1
	adverts[nextRef] = { description, chatFunc, deleteFunc };
	Event.Queue("onAdvertAdded", self, nextRef)
	return nextRef
end

--
-- Method: RemoveAdvert
--
-- Remove an advertisement from the station's bulletin board
--
-- > station:RemoveAdvert(ref)
--
-- If the deletefunc parameter was supplied to <AddAdvert> when the ad was
-- created, it will be called as part of this call.
--
-- Parameters:
--
--   ref - the advert reference number returned by <AddAdvert>
--
-- Availability:
--
--  alpha 10
--
-- Status:
--
--  stable
--

function SpaceStation:RemoveAdvert (ref)
	if not SpaceStation.adverts[self] then return end
	local deleteFunc = SpaceStation.adverts[self][ref][3]
	if deleteFunc then
		deleteFunc(ref)
	end
	SpaceStation.adverts[self][ref] = nil
	Event.Queue("onAdvertRemoved", self, ref)
end


local function updateAdverts (station)
	-- XXX this should really just be a single event
	-- XXX don't create for stations we haven't visited
	if not SpaceStation.adverts[station] then
		SpaceStation.adverts[station] = {}
		Event.Queue("onCreateBB", station)
	else
		Event.Queue("onUpdateBB", station)
	end
end


local function updateSystem ()
	local stations = Space.GetBodies(function (b) return b.superType == "STARPORT" end)
	for i=1,#stations do
		updateShipsOnSale(stations[i])
		updateAdverts(stations[i])
	end
end
local function destroySystem ()
	SpaceStation.shipsOnSale = {}

	for station,ads in pairs(SpaceStation.adverts) do
		for ref,ad in pairs(ads) do
			station:RemoveAdvert(ref)
		end
	end
	SpaceStation.adverts = {}
end

Event.Register("onGameStart", function ()
	nextRef = 0
	updateSystem()
	Timer:CallEvery(3600, updateSystem)
end)
Event.Register("onEnterSystem", function (ship)
	if ship ~= Game.player then return end
	updateSystem()
end)

Event.Register("onLeaveSystem", function (ship)
	if ship ~= Game.player then return end
	destroySystem()
end)
Event.Register("onGameEnd", destroySystem)

return SpaceStation
