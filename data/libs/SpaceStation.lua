-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

---@class SpaceStation : ModelBody
---@field techLevel integer
local SpaceStation = package.core['SpaceStation']

local Economy     = require 'Economy'
local Event       = require 'Event'
local Rand        = require 'Rand'
local Space       = require 'Space'
local utils       = require 'utils'
local ShipDef     = require 'ShipDef'
local Engine      = require 'Engine'
local Timer       = require 'Timer'
local Game        = require 'Game'
local Ship        = require 'Ship'
local Model       = require 'SceneGraph.Model'
local ModelSkin   = require 'SceneGraph.ModelSkin'
local Serializer  = require 'Serializer'
local Equipment   = require 'Equipment'
local Commodities = require 'Commodities'
local Faction     = require 'Faction'
local Lang        = require 'Lang'

local l = Lang.GetResource("ui-core")

--
-- Class: SpaceStation
--

function SpaceStation.GetTechLevel(systemBody)

	local rand = Rand.New(systemBody.seed .. '-techLevel')
	local techLevel = rand:Integer(1, 6) + rand:Integer(0,6)
	local system = systemBody.path:GetStarSystem()

	if system.faction ~= nil and system.faction.hasHomeworld and system.faction.homeworld == systemBody.parent.path then
		techLevel = math.max(techLevel, 6) -- bump it upto at least 6 if it's a homeworld like Earth
	end
	-- cap the techlevel lower end based on the planets population
	techLevel = math.max(techLevel, math.min(math.floor(systemBody.parent.population * 0.5), 11))
	return techLevel;
end

function SpaceStation:Constructor()
	local techLevel = SpaceStation.GetTechLevel(self.path:GetSystemBody())

	self:setprop("techLevel", techLevel)
end

-- visited keeps track of which stations we have docked with and have had
-- extended info (BBS adverts, ship ads, equipment stock info) generated for
local visited = {}
local equipmentStock = {}

-- transientMarket is a cache for commodity markets initialized in unvisited stations
-- it's used to reduce the impact of looking up goods in stations that haven't been generated yet
local transientMarket = utils.automagic()

local ensureStationData

-- create a transient entry for this station's equipment stock
local function createEquipmentStock (station)
	assert(station and station:exists())
	if equipmentStock[station] then error("Attempt to create station equipment stock twice!") end
	equipmentStock[station] = {}

	for _,slot in pairs{"laser", "hyperspace", "misc"} do
		for key, e in pairs(Equipment[slot]) do
			equipmentStock[station][e] = Engine.rand:Integer(0,100)
		end
	end
end

-- Create a transient entry for this station's commodity stocks and seed it with
-- commodity stock information from persistent data
local function createCommodityStock (station)
	local market = Economy.CreateStationMarket(station:GetSystemBody())

	-- Station Market data is persistent across saves and will be created
	-- before the transientMarket table, so we want to import the data from the
	-- station market if it exists
	for key, data in pairs(market.commodities) do
		transientMarket[station][key] = data
	end
end

-- ============================================================================

local equipmentPrice = {}

--
-- Method: GetEquipmentPrice
--
-- Get the price of an equipment item traded at this station
--
-- > price = station:GetEquipmentPrice(equip)
--
-- Parameters:
--
--   equip - the <Constants.EquipType> string for the equipment item
--
-- Returns:
--
--   price - the price of the equipment item
--
-- Availability:
--
--   alpha 10
--
-- Status:
--
--   stable
--
function SpaceStation:GetEquipmentPrice (e)
	assert(self:exists())

	if equipmentPrice[self] then
		return equipmentPrice[self][e] or e.price
	end

	return e.price
end

--
-- Method: SetEquipmentPrice
--
-- Set the price of an equipment item traded at this station
--
-- > station:SetEquipmentPrice(equip, price)
--
-- Parameters:
--
--   equip - the <Constants.EquipType> string for the equipment item
--
--   price - the new price of the equipment item
--
-- Availability:
--
--   alpha 10
--
-- Status:
--
--   stable
--
function SpaceStation:SetEquipmentPrice (e, price)
	assert(self:exists())
	if not equipmentPrice[self] then equipmentPrice[self] = {} end
	equipmentPrice[self][e] = price
end

--
-- Method: GetEquipmentStock
--
-- Get the quantity of an equipment item this station has available for trade
--
-- > stock = station:GetEquipmentStock(equip)
--
-- Parameters:
--
--   equip - the <Constants.EquipType> string for the equipment item
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
--   stable
--
function SpaceStation:GetEquipmentStock (e)
	assert(self:exists())
	return equipmentStock[self] and equipmentStock[self][e] or 0
end

--
-- Method: AddEquipmentStock
--
-- Modify the quantity of an equipment item this station has available for trade
--
-- > station:AddEquipmentStock(equip, amount)
--
-- Parameters:
--
--   equip - the <Constants.EquipType> string for the equipment item
--
--   amount - the amount of the item to add (or subtract) from the station stock
--
-- Availability:
--
--   201308
--
-- Status:
--
--   stable
--
function SpaceStation:AddEquipmentStock (e, stock)
	assert(self:exists())
	ensureStationData(self)
	assert(equipmentStock[self])

	equipmentStock[self][e] = (equipmentStock[self][e] or 0) + stock
end

-- ============================================================================

-- track commodity prices for stations present in the current system
local commodityPrice = utils.automagic()

--
-- Method: GetCommodityMarket
--
-- Get commodity market data for a commodity traded at this station.
--
-- > price = station:GetCommodityMarket(itemType)
--
-- Parameters:
--
--   itemType - the <CommodityType> of the commodity item in question
--
-- Returns:
--
--   market - the market information for the specified commodity in
--            { stock, demand, pricemod } triplet form
--
-- Availability:
--
--   January 2023
--
-- Status:
--
--   stable
--
---@param itemType CommodityType
---@return table market
function SpaceStation:GetCommodityMarket(itemType)
	assert(self:exists())

	local market = transientMarket[self]
	local sBody = assert(self:GetSystemBody())

	local comm = market[itemType.name]
	if not comm then
		logVerbose("Creating commodity market for {}: {}" % { self:GetLabel(), itemType.name })
		comm = Economy.CreateStationCommodityMarket(sBody, itemType.name)
		Economy.UpdateCommodityPriceMod(sBody, itemType.name, comm)

		market[itemType.name] = comm
	end

	return comm
end

--
-- Method: GetCommodityPrice
--
-- Get the price of a commodity item traded at this station
--
-- > price = station:GetCommodityPrice(itemType)
--
-- Parameters:
--
--   itemType - the <CommodityType> of the commodity item in question
--
-- Returns:
--
--   price - the price of the commodity item
--
-- Availability:
--
--   June 2022
--
-- Status:
--
--   stable
--
---@param itemType CommodityType
---@return number price
function SpaceStation:GetCommodityPrice(itemType)
	assert(self:exists())

	-- TODO: this cache exists to allow special-case pricing for commodities from events
	-- This should ideally be handled with a queue of temporary modifiers which
	-- adjust supply/demand/pricemod and avoid this cache entirely
	local price = commodityPrice[self][itemType.name]
	if price then return price end

	-- determine the commodity price modifier for the market conditions
	-- NOTE: the commodity price cache is not written to so that prices can update as stock changes
	-- This is not the best design, and should be re-thought if/when a market event queue is established
	local commodityMarket = self:GetCommodityMarket(itemType)
	return Economy.GetMarketPrice(itemType.price, commodityMarket[3])
end

--
-- Method: SetCommodityPrice
--
-- Set the price of a commodity item traded at this station
--
-- > station:SetCommodityPrice(itemType, price)
--
-- Parameters:
--
--   itemType - the <CommodityType> of the commodity item in question
--
--   price - the new price of the commodity item
--
-- Availability:
--
--   June 2022
--
-- Status:
--
--   stable
--
---@param itemType CommodityType
---@param price number
function SpaceStation:SetCommodityPrice(itemType, price)
	assert(self:exists())

	commodityPrice[self][itemType.name] = price
end

--
-- Method: GetCommodityStock
--
-- Get the quantity of a cargo item this station has available for trade
--
-- > stock = station:GetCommodityStock(itemType)
--
-- Parameters:
--
--   itemType - the <CommodityType> of the commodity item in question
--
-- Returns:
--
--   stock - the amount available for trade
--
-- Availability:
--
--   June 2022
--
-- Status:
--
--   stable
--
---@param itemType CommodityType
---@return integer stock
function SpaceStation:GetCommodityStock(itemType)
	assert(self:exists())

	return self:GetCommodityMarket(itemType)[1]
end

--
-- Method: GetCommodityDemand
--
-- Get the quantity of a cargo item this station wants to purchase
--
-- > demand = station:GetCommodityDemand(itemType)
--
-- Parameters:
--
--   itemType - the <CommodityType> of the commodity item in question
--
-- Returns:
--
--   demand - the amount the station is willing to buy
--
-- Availability:
--
--   January 2023
--
-- Status:
--
--   stable
--
---@param itemType CommodityType
---@return integer stock
function SpaceStation:GetCommodityDemand(itemType)
	return self:GetCommodityMarket(itemType)[2]
end

--
-- Method: AddCommodityStock
--
-- Modify the quantity of a cargo item this station has available for trade.
-- This function assumes this is taking place as part of a gameplay action,
-- so it modifies stock + demand levels based on the `amount` parameter.
--
-- > station:AddCommodityStock(itemType, amount)
--
-- Parameters:
--
--   itemType - a <CommodityType> cargo item
--
--   amount - the amount of the item to add (or subtract) from the station stock
--
-- Availability:
--
--   June 2022
--
-- Status:
--
--   stable
--
---@param itemType CommodityType
---@param amount integer
function SpaceStation:AddCommodityStock(itemType, amount)
	assert(self:exists())
	ensureStationData(self)

	local market = self:GetCommodityMarket(itemType)

	if amount < 0 then
		-- Buying from market - reduce commodity stock
		market[1] = math.max(market[1] + amount, 0)
	else
		-- Selling to market - reduce commodity demand
		market[2] = math.max(market[2] - amount, 0)
	end

	Economy.UpdateCommodityPriceMod(assert(self:GetSystemBody()), itemType.name, market)
end

--
-- Method: SetCommodityStock
--
-- Modify the stock and demand values of a cargo item this station has
-- available for trade. This function does not update the commodity price.
--
-- > station:SetCommodityStock(itemType, newStock, newDemand)
--
-- Parameters:
--
--   itemType - a <CommodityType> cargo item
--
--   stock - optional, the new stock number for the commodity type
--   demand - optional, the new demand number for the commodity type
--
-- Availability:
--
--   January 2023
--
-- Status:
--
--   stable
--
---@param itemType CommodityType
---@param stock integer? new commodity stock number
---@param demand integer? new commodity demand number
function SpaceStation:SetCommodityStock(itemType, stock, demand)
	assert(self:exists())
	ensureStationData(self)

	local market = self:GetCommodityMarket(itemType)

	market[1] = stock  or market[1]
	market[2] = demand or market[2]
end

-- ============================================================================

local shipsOnSale = {}

function SpaceStation:GetShipsOnSale ()
	assert(self:exists())
	if not shipsOnSale[self] then shipsOnSale[self] = {} end
	return shipsOnSale[self]
end

local function addShipOnSale (station, entry)
	if not shipsOnSale[station] then shipsOnSale[station] = {} end
	table.insert(shipsOnSale[station], entry)
end

function SpaceStation:AddShipOnSale (entry)
	assert(self:exists())
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
	assert(self:exists())
	local num = findShipOnSale(self, entry)
	if num > 0 then
		removeShipOnSale(self, num)
		Event.Queue("onShipMarketUpdate", self, shipsOnSale[self])
	end
end

function SpaceStation:ReplaceShipOnSale (old, new)
	assert(self:exists())
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


-- Dynamics of ship adverts in ShipMarket --
--------------------------------------------
-- N(t) = Number of ads, lambda = decay constant:
--    d/dt N(t) = prod - lambda * N
-- and equilibrium:
--    dN/dt = 0 = prod - lambda * N_equil
-- and solution (if prod=0), with N_0 initial number:
--    N(t) = N_0 * exp(-lambda * t)
-- with tau = half life, i.e. N(tau) = 0.5*N_0 we get:
--    0.5*N_0 = N_0*exp(-lambda * tau)
-- else, with production:
--   N(t) = prod/lambda - N_0*exp(-lambda * t)
-- We want to have N_0 = N_equil, since ShipMarket should spawn in equilibrium

-- Average number of ship for sale for station
local function N_equilibrium(station)
	local pop = station.path:GetSystemBody().parent.population -- E.g. Earth=7, Mars=0.3
	local pop_bonus = 9 * math.log(pop*0.45 + 1)       -- something that gives resonable result
	if station.type == "STARPORT_SURFACE" then
		pop_bonus = pop_bonus * 1.5
	end

	return 2 + pop_bonus
end

-- add num random ships for sale to station ShipMarket
local function addRandomShipAdvert(station, num)
	for i=1,num do
		local avail = station.type == "STARPORT_SURFACE" and groundShips or spaceShips
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

local function updateShipsOnSale (station)
	if not shipsOnSale[station] then shipsOnSale[station] = {} end
	local shipsOnSale = shipsOnSale[station]

	local tau = 7*24                              -- half life of a ship advert in hours
	local lambda = 0.693147 / tau                 -- removal probability= ln(2) / tau
	local prod = N_equilibrium(station) * lambda  -- creation probability

	-- remove with decay rate lambda. Call ONCE/hour for each ship advert in station
	for ref,ad in pairs(shipsOnSale) do
		if Engine.rand:Number(0,1) < lambda then  -- remove one random ship (sold)
			removeShipOnSale(station, Engine.rand:Integer(1,#shipsOnSale))
		end
	end

	-- spawn a new ship adverts, call for each station
	if Engine.rand:Number(0,1) <= prod then
		addRandomShipAdvert(station, 1)
	end

	if prod > 1 then print("Warning: ShipMarket not in equilibrium") end

	Event.Queue("onShipMarketUpdate", station, shipsOnSale)
end


--
-- Attribute: lawEnforcedRange
--
--   The distance, in meters, at which a station upholds the law,
--   (is 50 km for all at the moment)
--
-- Availability:
--
--   2015 September
--
-- Status:
--
--   experimental
--
SpaceStation.lawEnforcedRange = 50000


local police = {}

--
-- Method: LaunchPolice
--
-- Launch station police
--
-- > station:LaunchPolice(targetShip)
--
-- Parameters:
--
--   targetShip - the ship to intercept
--
-- Availability:
--
--   2015 September
--
-- Status:
--
--   experimental
--
function SpaceStation:LaunchPolice(targetShip)
	if not targetShip then error("Ship targeted invalid") end

	-- if no police created for this station yet:
	if not police[self] then
		police[self] = {}
		-- decide how many to create
		local lawlessness = Game.system.lawlessness
		local maxPolice = math.min(9, self.numDocks)
		local numberPolice = math.ceil(Engine.rand:Integer(1,maxPolice)*(1-lawlessness))
		local shiptype = ShipDef[Game.system.faction.policeShip]

		-- create and equip them
		while numberPolice > 0 do
			local policeShip = Space.SpawnShipDocked(shiptype.id, self)
			if policeShip == nil then
				return
			else
				numberPolice = numberPolice - 1
				--policeShip:SetLabel(Game.system.faction.policeName) -- this is cool, but not translatable right now
				policeShip:SetLabel(l.POLICE)
				policeShip:AddEquip(Equipment.laser.pulsecannon_dual_1mw)
				policeShip:AddEquip(Equipment.misc.atmospheric_shielding)
				policeShip:AddEquip(Equipment.misc.laser_cooling_booster)

				table.insert(police[self], policeShip)
			end
		end
	end

	for _, policeShip in pairs(police[self]) do
		-- if docked
		if policeShip.flightState == "DOCKED" then
			policeShip:Undock()
		end
		-- if not shot down
		if policeShip:exists() then
			policeShip:AIKill(targetShip)
		end
	end
end


--
-- Method: LandPolice
--
-- Clear any target assigned and land flying station police.
--
-- > station:LandPolice()
--
--
-- Availability:
--
--   2015 September
--
-- Status:
--
--   experimental
--
function SpaceStation:LandPolice()
	-- land command issued before creation of police
	if not police[self] then return end

	for _, policeShip in pairs(police[self]) do
		if not (policeShip.flightState == "DOCKED") and policeShip:exists() then
			policeShip:CancelAI()
			policeShip:AIDockWith(self)
		end
	end
end


--
-- Group: Methods
--

SpaceStation.lockedAdvert = nil
SpaceStation.advertLockCount = 0
SpaceStation.removeOnReleased = false
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
--   icon - optional, filename of an icon to display alongside the advert.
--          Defaults to bullet (data/icons/bbs/default).
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
	assert(self:exists())
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
	adverts[nextRef] = args

	args.__ref = nextRef
	args.title = args.title or ""

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
	assert(self:exists())
	if not SpaceStation.adverts[self] then return end
	if SpaceStation.lockedAdvert == ref then
		SpaceStation.removeOnReleased = true
		return
	end
	local onDelete = SpaceStation.adverts[self][ref].onDelete
	if onDelete then
		onDelete(ref)
	end
	SpaceStation.adverts[self][ref] = nil
	Event.Queue("onAdvertRemoved", self, ref)
end

--
-- Method: LockAdvert
--
-- > station:LockAdvert(ref)
--
-- Only one advert may be locked at a time. Locked adverts are not removed by
-- RemoveAdvert until they are unlocked.
--
-- Parameters:
--
--   ref - the advert reference number returned by <AddAdvert>
--
-- Availability:
--
--  September 2014
--
-- Status:
--
--  experimental
--
function SpaceStation:LockAdvert (ref)
	assert(self:exists())
	if (SpaceStation.advertLockCount > 0) then
		assert(SpaceStation.lockedAdvert == ref, "Attempt to lock ref "..ref
		.."disallowed."
		.." Ref "..(SpaceStation.lockedAdvert or "nil").." is already locked "
		..SpaceStation.advertLockCount.." times.")
		SpaceStation.advertLockCount = SpaceStation.advertLockCount + 1
		return
	end
	assert(SpaceStation.lockedAdvert == nil, "Attempt to lock ref "..ref
		.." disallowed."
		.." Ref "..(SpaceStation.lockedAdvert or "nil").." is already locked.")
	SpaceStation.lockedAdvert = ref
	SpaceStation.removeOnReleased = false
	SpaceStation.advertLockCount = 1
end

local function updateAdverts (station)
	if not SpaceStation.adverts[station] then
		logWarning("SpaceStation.lua: updateAdverts called for station that hasn't been visited")
	else
		Event.Queue("onUpdateBB", station)
	end
end

--
-- Method: UnlockAdvert
--
-- > station:UnlockAdvert(ref)
--
-- Releases the preserved advert. There must be an advert preserved at the
-- time. If RemoveAdvert(ref) was called with the preserved advert's ref while
-- the advert was preserved, it is now removed.
--
-- Parameters:
--
--   ref - the advert reference number returned by <AddAdvert>
--
-- Availability:
--
--  September 2014
--
-- Status:
--
--  experimental
--
function SpaceStation:UnlockAdvert (ref)
	assert(SpaceStation.lockedAdvert == ref, "Attempt to unlock ref "..ref
		.." disallowed."
		.." Ref "..(SpaceStation.lockedAdvert or "nil").." is locked"
		..SpaceStation.advertLockCount.." times. Unlock this"
		.." first.")
	if (SpaceStation.advertLockCount > 1) then
		SpaceStation.advertLockCount = SpaceStation.advertLockCount - 1
		return
	end
	SpaceStation.lockedAdvert = nil
	SpaceStation.advertLockCount = 0
	if SpaceStation.removeOnReleased then
		self:RemoveAdvert(ref)
	end
end

local function updateSystem ()
	local stations = Space.GetBodies("SpaceStation")
	for i, station in ipairs(stations) do
		-- updateStationMarket(station)
		Economy.UpdateStationMarket(station:GetSystemBody())

		if visited[station] then
			updateShipsOnSale(station)
			updateAdverts(station)
		end
	end
end

local function createStationData (station)
	SpaceStation.adverts[station] = {}
	shipsOnSale[station] = {}
	visited[station] = true

	createEquipmentStock(station)
	createCommodityStock(station)

	local shipAdsToSpawn = Engine.rand:Poisson(N_equilibrium(station))
	addRandomShipAdvert(station, shipAdsToSpawn)

	Event.Queue("onCreateBB", station)
end

ensureStationData = function (station)
	if not visited[station] or not equipmentStock[station] then
		logWarning("Creating station data for station " .. station.label .. " before onShipDocked event is processed for that station")
		logVerbose(debug.dumpstack(2))

		createStationData(station)
	end
end

local function destroySystem ()
	equipmentStock = {}
	equipmentPrice = {}

	commodityPrice = utils.automagic()
	transientMarket = utils.automagic()

	visited = {}

	police = {}

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
		commodityPrice = utils.automagic(loaded_data.commodityPrice)

		visited = loaded_data.visited or {}
		police = loaded_data.police

		for station,_ in pairs(visited) do
			createCommodityStock(station)
		end

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

	local station = Game.player:GetDockedWith()

	if station and station:isa("SpaceStation") and not visited[station] then
		createStationData(station)
	end

	Timer:CallEvery(3600, updateSystem)
end)

Event.Register("onShipDocked", function (ship, station)
	if ship ~= Game.player then return end

	if not visited[station] then
		createStationData(station)
	else
		Economy.UpdateStationMarket(station:GetSystemBody())
	end
end)

Event.Register("onLeaveSystem", function (ship)
	if ship ~= Game.player then return end
	destroySystem()
end)

Event.Register("onShipDestroyed", function (ship, _)
	for _,local_police in pairs(police) do
		for k,police_ship in pairs(local_police) do
			if (ship == police_ship) then
				table.remove(local_police, k)
			end
		end
	end
end)

Event.Register("onGameEnd", function ()
	destroySystem()

	-- XXX clean up for next game
	nextRef = 0

	Economy.OnGameEnd()
end)


Serializer:Register("SpaceStation",
	function ()
		local data = {
			equipmentStock = equipmentStock,
			equipmentPrice = equipmentPrice,
			commodityPrice = commodityPrice,
			visited = visited,
			police = police,  --todo fails if a police ship is killed
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
