-- Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local SpaceStation = import_core("SpaceStation")
local Event = import("Event")
local Rand = import("Rand")
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
local Faction = import("Faction")
local Lang = import("Lang")
local l = Lang.GetResource("ui-core")

--
-- Class: SpaceStation
--


function SpaceStation:Constructor()
	-- Use a variation of the space station seed itself to ensure consistency
	local rand = Rand.New(util.hash_random(self.seed .. '-techLevel', 2^31)-1)
	local techLevel = rand:Integer(1, 6) + rand:Integer(0,6)
	self:setprop("techLevel", techLevel)
end


local equipmentStock = {}

local function updateEquipmentStock (station)
	assert(station and station:exists())
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
	assert(self:exists())
	if not equipmentPrice[self] then equipmentPrice[self] = {} end
	if equipmentPrice[self][e] then
		return equipmentPrice[self][e]
	end
	local mul = e:IsValidSlot("cargo") and ((100 + Game.system:GetCommodityBasePriceAlterations(e)) / 100) or 1
	return mul * e.price
end

function SpaceStation:SetEquipmentPrice (e, v)
	assert(self:exists())
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
	assert(self:exists())
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
	assert(self:exists())
	equipmentStock[self][e] = (equipmentStock[self][e] or 0) + stock
end



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

local function createShipMarket (station)
	shipsOnSale[station] = {}

	local shipAdsToSpawn = Engine.rand:Poisson(N_equilibrium(station))
	addRandomShipAdvert(station, shipAdsToSpawn)
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
--   (is 100 km for all at the moment)
--
-- Availability:
--
--   2015 September
--
-- Status:
--
--   experimental
--
SpaceStation.lawEnforcedRange = 100000


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
				policeShip:AddEquip(Equipment.cargo.hydrogen, 1)

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
	-- XXX this should really just be a single event
	-- XXX don't create for stations we haven't visited
	if not SpaceStation.adverts[station] then
		SpaceStation.adverts[station] = {}
		Event.Queue("onCreateBB", station)
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
	local stations = Space.GetBodies(function (b) return b.superType == "STARPORT" end)
	for i=1,#stations do
		updateEquipmentStock(stations[i])
		updateShipsOnSale(stations[i])
		updateAdverts(stations[i])
	end
end

local function createSystem()
	local stations = Space.GetBodies(function (b) return b.superType == "STARPORT" end)
	for i=1,#stations do
		createShipMarket(stations[i])
	end
end

local function destroySystem ()
	equipmentStock = {}
	equipmentPrice = {}

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
		police = loaded_data.police
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

	createSystem()
	updateSystem()
	Timer:CallEvery(3600, updateSystem)
end)
Event.Register("onEnterSystem", function (ship)
	if ship ~= Game.player then return end
	createSystem()
	updateSystem()
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
	equipmentStock = {}
	equipmentPrice = {}
	police = {}
	shipsOnSale = {}
end)


Serializer:Register("SpaceStation",
	function ()
		local data = {
			equipmentStock = equipmentStock,
			equipmentPrice = equipmentPrice,
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
