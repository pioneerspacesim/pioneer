-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
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
local Model = import("SceneGraph.Model")
local ModelSkin = import("SceneGraph.ModelSkin")
local Serializer = import("Serializer")
local Equipment = import("Equipment")

--
-- Class: SpaceStation
--

local equipmentStock = {}

local function updateEquipmentStock (station)
	if equipmentStock[station] then return end
	equipmentStock[station] = {}
	local hydrogen = Equipment.cargo.hydrogen
	for _,slot in pairs{"cargo","laser", "hyperspace", "misc"} do
		for key, e in pairs(Equipment[slot]) do
			if e:IsValidSlot("cargo") then      -- is cargo
				local min = e == hydrogen and 1 or 0 -- always stock hydrogen
				equipmentStock[station][e] = Engine.rand:Integer(min,100) * Engine.rand:Integer(1,100)
			else                                     -- is ship equipment
				equipmentStock[station][e] = Engine.rand:Integer(0,100)
			end
		end
	end
end

local equipmentPrice = {}

--
-- Method: GetEquipmentPrice
--
-- Get the price of an equipment or cargo item traded at this station
--
-- > price = station:GetEquipmentPrice(equip)
--
-- Parameters:
--
--   equip - the <Constants.EquipType> string for the equipment or cargo item
--
-- Returns:
--
--   price - the price of the equipment or cargo item
--
-- Availability:
--
--   alpha 10
--
-- Status:
--
--   experimental
--

function SpaceStation:GetEquipmentPrice (e)
	if not equipmentPrice[self] then equipmentPrice[self] = {} end
	if equipmentPrice[self][e] then
		return equipmentPrice[self][e]
	end
	local mul = e:IsValidSlot("cargo") and ((100 + Game.system:GetCommodityBasePriceAlterations(e)) / 100) or 1
	return mul * e.price
end

function SpaceStation:SetEquipmentPrice (e, v)
	if not equipmentPrice[self] then equipmentPrice[self] = {} end
	equipmentPrice[self][e] = v
end

--
-- Method: GetEquipmentStock
--
-- Get the quantity of an equipment or cargo item this station has available for trade
--
-- > stock = station:GetEquipmentStock(equip)
--
-- Parameters:
--
--   equip - the <Constants.EquipType> string for the equipment or cargo item
--
-- Returns:
--
--   stock - the amount available for trade
--
-- Availability:
--
--   201308
--
-- Status:
--
--   experimental
--
function SpaceStation:GetEquipmentStock (e)
	return equipmentStock[self][e] or 0
end

--
-- Method: AddEquipmentStock
--
-- Modify the quantity of an equipment or cargo item this station has available for trade
--
-- > station:AddEquipmentStock(equip, amount)
--
-- Parameters:
--
--   equip - the <Constants.EquipType> string for the equipment or cargo item
--
--   amount - the amount of the item to add (or subtract) from the station stock
--
-- Availability:
--
--   201308
--
-- Status:
--
--   experimental
--
function SpaceStation:AddEquipmentStock (e, stock)
	equipmentStock[self][e] = (equipmentStock[self][e] or 0) + stock
end



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
		def     = entry.def,
		skin    = entry.skin,
		pattern = entry.pattern,
		label   = entry.label
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
			def     = new.def,
			skin    = new.skin,
			pattern = new.pattern,
			label   = new.label,
		}
	end
	Event.Queue("onShipMarketUpdate", self, shipsOnSale[self])
end

local isPlayerShip = function (def) return def.tag == "SHIP" and def.basePrice > 0 end

local groundShips = utils.build_array(utils.filter(function (k,def) return isPlayerShip(def) and def.equipSlotCapacity.atmo_shield > 0 end, pairs(ShipDef)))
local spaceShips  = utils.build_array(utils.filter(function (k,def) return isPlayerShip(def) end, pairs(ShipDef)))

local function updateShipsOnSale (station)
	if not shipsOnSale[station] then shipsOnSale[station] = {} end
	local shipsOnSale = shipsOnSale[station]

	local toAdd, toRemove = 0, 0
	if #shipsOnSale == 0 then
		toAdd = Engine.rand:Integer(20)
	elseif Engine.rand:Integer(1) > 0 then
		toAdd = 1
	elseif #shipsOnSale > 0 then
		toRemove = 1
	else
		return
	end

	if toAdd > 0 then
		local avail = station.type == "STARPORT_SURFACE" and groundShips or spaceShips
		for i=1,toAdd do
			local def = avail[Engine.rand:Integer(1,#avail)]
			local model = Engine.GetModel(def.modelName)
			local pattern = model.numPatterns ~= 0 and Engine.rand:Integer(1,model.numPatterns) or nil
			addShipOnSale(station, {
				def     = def,
				skin    = ModelSkin.New():SetRandomColors(Engine.rand):SetDecal(def.manufacturer),
				pattern = pattern,
				label   = Ship.MakeRandomLabel(),
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
-- > ref = station:AddAdvert({
-- >     description = description,
-- >     icon        = icon,
-- >     onChat      = onChat,
-- >     onDelete    = onDelete,
-- >     isEnabled   = isEnabled,
-- > })
-- >
-- > -- Legacy form
-- > ref = station:AddAdvert(description, onChat, onDelete)
--
-- Parameters:
--
--   description - text to display in the bulletin board
--
--   icon - (option) filename of an icon to display alongside the advert
--
--   onChat - function to call when the ad is activated. The function is
--            passed three parameters: a <ChatForm> object for the ad
--            conversation display, the ad reference returned by <AddAdvert>
--            when the ad was created, and an integer value corresponding to
--            the action that caused the activation. When the ad is initially
--            selected from the bulletin board, this value is 0. Additional
--            actions (and thus values) are defined by the script via
--            <ChatForm.AddAction>.
--
--   onDelete - optional. function to call when the ad is removed from the
--              bulletin board. This happens when <RemoveAdvert> is called,
--              when the ad is cleaned up after
--              <ChatForm.RemoveAdvertOnClose> is called, and when the
--              <SpaceStation> itself is destroyed (eg the player leaves the
--              system).
--
--   isEnabled - optional. function to call to determine whether the advert is
--               enabled. Disabled adverts are shown in darker tone than enabled
--               ones. When not given, all adverts are considered enabled.
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
function SpaceStation:AddAdvert (description, onChat, onDelete)
	-- XXX legacy arg unpacking
	local args
	if (type(description) == "table") then
		args = description
	else
		args = {
			description = description,
			onChat      = onChat,
			onDelete    = onDelete,
		}
	end

	if not SpaceStation.adverts[self] then SpaceStation.adverts[self] = {} end
	local adverts = SpaceStation.adverts[self]
	nextRef = nextRef+1
	adverts[nextRef] = {
		description = args.description,
		icon        = args.icon,
		onChat      = args.onChat,
		onDelete    = args.onDelete,
		isEnabled   = args.isEnabled,
	}
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
	local onDelete = SpaceStation.adverts[self][ref].onDelete
	if onDelete then
		onDelete(ref)
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
		updateEquipmentStock(stations[i])
		updateShipsOnSale(stations[i])
		updateAdverts(stations[i])
	end
end
local function destroySystem ()
	equipmentStock = {}
	equipmentPrice = {}

	shipsOnSale = {}

	for station,ads in pairs(SpaceStation.adverts) do
		for ref,ad in pairs(ads) do
			station:RemoveAdvert(ref)
		end
	end
	SpaceStation.adverts = {}
end


local loaded_data

Event.Register("onGameStart", function ()
	if (loaded_data) then
		equipmentStock = loaded_data.equipmentStock
		equipmentPrice = loaded_data.equipmentPrice or {} -- handle missing in old saves
		for station,list in pairs(loaded_data.shipsOnSale) do
			shipsOnSale[station] = {}
			for i,entry in pairs(loaded_data.shipsOnSale[station]) do
				local def = ShipDef[entry.id]
				if (def) then
					shipsOnSale[station][i] = {
						def     = def,
						skin    = entry.skin,
						pattern = entry.pattern,
						label   = entry.label,
					}
				end
			end
		end
		loaded_data = nil
	end

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
Event.Register("onGameEnd", function ()
	destroySystem()

	-- XXX clean up for next game
	nextRef = 0
	equipmentStock = {}
	equipmentPrice = {}
	shipsOnSale = {}
end)


Serializer:Register("SpaceStation",
	function ()
		local data = {
			equipmentStock = equipmentStock,
			equipmentPrice = equipmentPrice,
			shipsOnSale = {},
		}
		for station,list in pairs(shipsOnSale) do
			data.shipsOnSale[station] = {}
			for i,entry in pairs(shipsOnSale[station]) do
				data.shipsOnSale[station][i] = {
					id      = entry.def.id,
					skin    = entry.skin,
					pattern = entry.pattern,
					label   = entry.label,
				}
			end
		end
		return data
	end,
	function (data)
		loaded_data = data
	end
)


return SpaceStation
