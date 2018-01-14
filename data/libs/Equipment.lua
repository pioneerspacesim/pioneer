-- Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local utils = import("utils")
local Game = import_core("Game")
local Serializer = import("Serializer")
local Lang = import("Lang")
local ShipDef = import("ShipDef")
local Timer = import("Timer")
local Space = import_core("Space")
local Comms = import("Comms")

local cargo
local laser
local hyperspace
local misc

--
-- Class: EquipType
--
-- A container for a ship's equipment.
--
-- Its constructor takes a table, the "specs". Mandatory fields are the following:
--  * l10n_key: the key to look up the name and description of
--          the object in a language-agnostic way
--  * l10n_resource: where to look up the aforementioned key. If not specified,
--          the system assumes "equipment-core"
--  * capabilities: a table of string->int, having at least "mass" as a valid key
--
-- All specs are copied directly within the object (even those I know nothing about),
-- but it is a shallow copy. This is particularly important for the capabilities, as
-- modifying the capabilities of one EquipType instance might modify them for other
-- instances if the same table was used for all (which is strongly discouraged by the
-- author, but who knows ? Some people might find it useful.)
--
--
local EquipType = utils.inherits(nil, "EquipType")

function EquipType.New (specs)
	local obj = {}
	for i,v in pairs(specs) do
		obj[i] = v
	end
	if not obj.l10n_resource then
		obj.l10n_resource = "equipment-core"
	end
	local l = Lang.GetResource(obj.l10n_resource)
	obj.volatile = {
		description = l:get(obj.l10n_key.."_DESCRIPTION") or "",
		name = l[obj.l10n_key] or ""
	}
	setmetatable(obj, EquipType.meta)
	if type(obj.slots) ~= "table" then
		obj.slots = {obj.slots}
	end
	return obj
end

function EquipType:Serialize()
	local tmp = EquipType.Super().Serialize(self)
	local ret = {}
	for k,v in pairs(tmp) do
		if type(v) ~= "function" then
			ret[k] = v
		end
	end

	if debug.dmodeenabled() and self.slots == "cargo" then
		for _,v in pairs(cargo) do
			if (v.l10n_key == self.key and v.l10n_resource == self.l10n_resource) then
				assert(v == self)
				break
			end
		end
	end

	ret.volatile = nil
	return ret
end

function EquipType.Unserialize(data)
	obj = EquipType.Super().Unserialize(data)
	setmetatable(obj, EquipType.meta)
	if not obj.l10n_resource then
		obj.l10n_resource = "equipment-core"
	end
	local l = Lang.GetResource(obj.l10n_resource)
	obj.volatile = {
		description = l:get(obj.l10n_key.."_DESCRIPTION") or "",
		name = l[obj.l10n_key] or ""
	}
	return obj
end

--
-- Group: Methods
--

--
-- Method: GetDefaultSlot
--
--  returns the default slot for this equipment
--
-- Parameters:
--
--  ship (optional) - if provided, tailors the answer for this specific ship
--
-- Return:
--
--  slot_name - A string identifying the slot.
--
function EquipType:GetDefaultSlot(ship)
	return self.slots[1]
end

--
-- Method: IsValidSlot
--
--  tells whether the given slot is valid for this equipment
--
-- Parameters:
--
--  slot - a string identifying the slot in question
--
--  ship (optional) - if provided, tailors the answer for this specific ship
--
-- Return:
--
--  valid - a boolean qualifying the validity of the slot.
--
function EquipType:IsValidSlot(slot, ship)
	for _, s in ipairs(self.slots) do
		if s == slot then
			return true
		end
	end
	return false
end

function EquipType:GetName()
	return self.volatile.name
end

function EquipType:GetDescription()
	return self.volatile.description
end

local function __ApplyMassLimit(ship, capabilities, num)
	if num <= 0 then return 0 end
	-- we need to use mass_cap directly (not, eg, ship.freeCapacity),
	-- because ship.freeCapacity may not have been updated when Install is called
	-- (see implementation of EquipSet:Set)
	local avail_mass = ShipDef[ship.shipId].capacity - (ship.mass_cap or 0)
	local item_mass = capabilities.mass or 0
	if item_mass > 0 then
		num = math.min(num, math.floor(avail_mass / item_mass))
	end
	return num
end

local function __ApplyCapabilities(ship, capabilities, num, factor)
	if num <= 0 then return 0 end
	factor = factor or 1
	for k,v in pairs(capabilities) do
		local full_name = k.."_cap"
		local prev = (ship:hasprop(full_name) and ship[full_name]) or 0
		ship:setprop(full_name, (factor*v*num)+prev)
	end
	return num
end

function EquipType:Install(ship, num, slot)
	local caps = self.capabilities
	num = __ApplyMassLimit(ship, caps, num)
	return __ApplyCapabilities(ship, caps, num, 1)
end

function EquipType:Uninstall(ship, num, slot)
	return __ApplyCapabilities(ship, self.capabilities, num, -1)
end

-- Base type for weapons
local LaserType = utils.inherits(EquipType, "LaserType")
function LaserType:Install(ship, num, slot)
	if num > 1 then num = 1 end -- FIXME: support installing multiple lasers (e.g., in the "cargo" slot?)
	if LaserType.Super().Install(self, ship, 1, slot) < 1 then return 0 end
	local prefix = slot..'_'
	for k,v in pairs(self.laser_stats) do
		ship:setprop(prefix..k, v)
	end
	return 1
end

function LaserType:Uninstall(ship, num, slot)
	if num > 1 then num = 1 end -- FIXME: support uninstalling multiple lasers (e.g., in the "cargo" slot?)
	if LaserType.Super().Uninstall(self, ship, 1) < 1 then return 0 end
	local prefix = (slot or "laser_front").."_"
	for k,v in pairs(self.laser_stats) do
		ship:unsetprop(prefix..k)
	end
	return 1
end

-- Single drive type, no support for slave drives.
local HyperdriveType = utils.inherits(EquipType, "HyperdriveType")

HyperdriveType.GetMaximumRange = function (self, ship)
	return self.capabilities.power / (ship.staticMass + ship.fuelMassLeft)
end

-- range_max is as usual optional
HyperdriveType.GetDuration = function (self, ship, distance, range_max)
	range_max = range_max or self:GetMaximumRange(ship)
	local hyperclass = self.capabilities.hyperclass
	local hyperspeed = self.capabilities.speed
	return 0.36*distance^2/(range_max*hyperclass) * (86400*math.sqrt(ship.staticMass + ship.fuelMassLeft)) * (100/hyperspeed)
end

-- range_max is optional, distance defaults to the maximal range.
HyperdriveType.GetFuelUse = function (self, ship, distance, range_max)
	range_max = range_max or self:GetMaximumRange(ship)
	local distance = distance or range_max
	local maxfuel = self.capabilities.maxfuel
	return math.clamp(math.ceil(maxfuel*distance / range_max), 1, maxfuel);
end

-- if the destination is reachable, returns: distance, fuel, duration
-- if the destination is out of range, returns: distance
-- if the specified jump is invalid, returns nil
HyperdriveType.CheckJump = function (self, ship, source, destination)
	if ship:GetEquip('engine', 1) ~= self or source:IsSameSystem(destination) then
		return nil
	end
	local distance = source:DistanceTo(destination)
	local max_range = self:GetMaximumRange(ship) -- takes fuel into account
	if distance > max_range then
		return distance
	end
	local fuel = self:GetFuelUse(ship, distance, max_range) -- specify range_max to avoid unnecessary recomputing.

	local duration = self:GetDuration(ship, distance, max_range) -- same as above
	return distance, fuel, duration
end

-- like HyperdriveType.CheckJump, but uses Game.system as the source system
-- if the destination is reachable, returns: distance, fuel, duration
-- if the destination is out of range, returns: distance
-- if the specified jump is invalid, returns nil
HyperdriveType.CheckDestination = function (self, ship, destination)
	if not Game.system then
		return nil
	end
	return self:CheckJump(ship, Game.system.path, destination)
end

-- Give the range for the given remaining fuel
-- If the fuel isn't specified, it takes the current value.
HyperdriveType.GetRange = function (self, ship, remaining_fuel)
	local range_max = self:GetMaximumRange(ship)
	local fuel_max = self.capabilities.maxfuel
	remaining_fuel = remaining_fuel or ship:CountEquip(self.fuel)

	if fuel_max <= remaining_fuel then
		return range_max, range_max
	end
	local range = range_max*remaining_fuel/fuel_max

	while range > 0 and self:GetFuelUse(ship, range, range_max) > remaining_fuel do
		range = range - 0.05
	end

	-- range is never negative
	range = math.max(range, 0)
	return range, range_max
end

local HYPERDRIVE_SOUNDS_NORMAL = {
	warmup = "Hyperdrive_Charge",
	abort = "Hyperdrive_Abort",
	jump = "Hyperdrive_Jump",
}

local HYPERDRIVE_SOUNDS_MILITARY = {
	warmup = "Hyperdrive_Charge_Military",
	abort = "Hyperdrive_Abort_Military",
	jump = "Hyperdrive_Jump_Military",
}

HyperdriveType.HyperjumpTo = function (self, ship, destination)
	-- First off, check that this is the primary engine.
	local engines = ship:GetEquip('engine')
	local primary_index = 0
	for i,e in ipairs(engines) do
		if e == self then
			primary_index = i
			break
		end
	end
	if primary_index == 0 then
		-- wrong ship
		return "WRONG_SHIP"
	end
	local distance, fuel_use, duration = self:CheckDestination(ship, destination)
	if not distance then
		return "OUT_OF_RANGE"
	end
	if not fuel_use then
		return "INSUFFICIENT_FUEL"
	end
	ship:setprop('nextJumpFuelUse', fuel_use)
	-- previously warmup was calculated for all drives as: 5 + hyperclass * 1.5
	local warmup_time = self.capabilities.warmup

	local sounds
	if self.fuel == cargo.military_fuel then
		sounds = HYPERDRIVE_SOUNDS_MILITARY
	else
		sounds = HYPERDRIVE_SOUNDS_NORMAL
	end

	return ship:InitiateHyperjumpTo(destination, warmup_time, duration, sounds), fuel_use, duration
end

HyperdriveType.OnEnterHyperspace = function (self, ship)
	if ship:hasprop('nextJumpFuelUse') then
		local amount = ship.nextJumpFuelUse
		ship:RemoveEquip(self.fuel, amount)
		if self.byproduct then
			ship:AddEquip(self.byproduct, amount)
		end
		ship:unsetprop('nextJumpFuelUse')
	end
end

local SensorType = utils.inherits(EquipType, "SensorType")

function SensorType:BeginAcquisition(callback)
	self:ClearAcquisition()
	self.callback = callback
	if self:OnBeginAcquisition() then
		self.state = "RUNNING"
		self.stop_timer = false
		Timer:CallEvery(1, function()
			return self:ScanProgress()
		end)
	end
	self:DoCallBack()
end

function SensorType:ScanProgress()
	if self.stop_timer == true then
		return true
	end
	if self:IsScanning() then
		self:OnProgress()
		if self:IsScanning() then
			self.stop_timer = false
		end
	elseif self.state == "PAUSED" then
		self.stop_timer = false
	elseif self.state == "DONE" then
		self.stop_timer = true
	end
	self:DoCallBack()
	return self.stop_timer
end

function SensorType:PauseAcquisition()
	if self:IsScanning() then
		self.state = "PAUSED"
	end
	self:DoCallBack()
end

function SensorType:UnPauseAcquisition()
	if self.state == "PAUSED" then
		self.state = "RUNNING"
	end
	self:DoCallBack()
end

function SensorType:ClearAcquisition()
	self:OnClear()
	self.state = "DONE"
	self.stop_timer = true
	self:DoCallBack()
	self.callback = nil
end

function SensorType:GetLastResults()
	return self.progress
end

-- gets called from C++ to set the MeterBar value
-- must return a number
function SensorType:GetProgress()
	if type(self.progress) == "number" then
		return self.progress
	else
		return 0
	end
end

function SensorType:IsScanning()
	return self.state == "RUNNING" or self.state == "HALTED"
end

function SensorType:DoCallBack()
	if self.callback then self.callback(self.progress, self.state) end
end

local BodyScannerType = utils.inherits(SensorType, "BodyScannerType")

function BodyScannerType:OnBeginAcquisition()
	local closest_planet = Game.player:FindNearestTo("PLANET")
	if closest_planet then
		local altitude = self:DistanceToSurface(closest_planet)
		if altitude and altitude < self.max_range then
			self.target_altitude = altitude
			self.target_body_path = closest_planet.path
			local l = Lang.GetResource(self.l10n_resource)
			Comms.Message(l.STARTING_SCAN.." "..string.format('%6.3f km',self.target_altitude/1000))
			return true
		end
	end
	return false
end

function BodyScannerType:OnProgress()
	local l = Lang.GetResource(self.l10n_resource)
	local target_body = Space.GetBody(self.target_body_path.bodyIndex)
	if target_body and target_body:exists() then
		local altitude = self:DistanceToSurface(target_body)
		local distance_diff = math.abs(altitude - self.target_altitude)
		local percentual_diff = distance_diff/self.target_altitude
		if percentual_diff <= self.bodyscanner_stats.scan_tolerance then
			if self.state == "HALTED" then
				Comms.Message(l.SCAN_RESUMED)
				self.state = "RUNNING"
			end
			self.progress = self.progress + self.bodyscanner_stats.scan_speed
			if self.progress > 100 then
				self.state = "DONE"
				self.progress = {body=target_body.path, altitude=self.target_altitude}
				Comms.Message(l.SCAN_COMPLETED)
			end
		else -- strayed out off range
			if self.state == "RUNNING" then
				local lower_limit = self.target_altitude-(percentual_diff*self.target_altitude)
				local upper_limit = self.target_altitude+(percentual_diff*self.target_altitude)
				Comms.Message(l.OUT_OF_SCANRANGE.." "..string.format('%6.3f km',lower_limit/1000).." - "..string.format('%6.3f km',upper_limit/1000))
			end
			self.state = "HALTED"
		end
	else -- we lost the target body
		self:ClearAcquisition()
	end
end

function BodyScannerType:OnClear()
	self.target_altitude = 0
	self.progress = 0
end

function BodyScannerType:DistanceToSurface(body)
	return  select(3,Game.player:GetGroundPosition(body)) -- altitude
end

-- Constants: EquipSlot
--
-- Equipment slots. Every equipment or cargo type has a corresponding
-- "slot" that it fits in to. Each slot has an independent capacity.
--
-- cargo - any cargo (commodity) item
-- engine - hyperdrives and military drives
-- laser_front - front attachment point for lasers and plasma accelerators
-- laser_rear - rear attachment point for lasers and plasma accelerators
-- missile - missile
-- ecm - ecm system
-- radar - radar
-- target_scanner - target scanner
-- hypercloud - hyperspace cloud analyser
-- hull_autorepair - hull auto-repair system
-- energy_booster - shield energy booster unit
-- atmo_shield - atmospheric shielding
-- cabin - cabin
-- shield - shield
-- scoop - scoop used for scooping things (cargo, fuel/hydrogen)
-- laser_cooler - laser cooling booster
-- cargo_life_support - cargo bay life support
-- autopilot - autopilot
-- trade_computer - commodity trade analyzer computer module

commoditylist = {
	{ 'air_processors',			"industry",		 150 },
	{ 'animal_meat',			"agriculture",	  80 },
	{ 'battle_weapons',			"industry", 	3500 },
	{ 'carbon_ore',				"mining", 		   7.5 },
	{ 'computers',				"industry", 	 350 },
	{ 'consumer_goods',			"industry", 	 250 },
	{ 'farm_machinery',			"industry", 	 175 },
	{ 'fertilizer',				"industry",		  10 },
	{ 'fruit_and_veg',			"agriculture",	  35 },
	{ 'grain',					"agriculture",	  15 },
	{ 'hand_weapons',			"industry",		 750 },
	{ 'hydrogen', 				"mining", 		   3 },
	{ 'industrial_machinery',	"industry",		 150 },
	{ 'liquid_oxygen',			"mining", 		   6.5 },
	{ 'liquor',					"agriculture",	  75 },
	{ 'live_animals',			"agriculture",	 200 },
	{ 'medicines',				"industry", 	 550 },
	{ 'metal_alloys',			"industry",		  50 },
	{ 'metal_ore',				"mining", 		  10 },
	{ 'military_fuel',			"industry", 	  42 },
	{ 'mining_machinery',		"industry", 	 125 },
	{ 'narcotics',				"agriculture",	 800 },
	{ 'nerve_gas',				"industry", 	1500 },
	{ 'plastics',				"industry", 	  25 },
	{ 'precious_metals',		"mining", 		2000 },
	{ 'radioactives',			"industry", 	 -15.5 },
	{ 'robots',					"industry", 	 400 },
	{ 'rubbish',				"industry", 	  -5.5 },
	{ 'slaves',					"agriculture",	5000 },
	{ 'textiles',				"industry", 	  25 },
	{ 'water',					"mining", 		   1 }
}

cargo = {}

for _,com in pairs(commoditylist) do
	local tblk = com[1]
	local econ = com[2]
	local val = com[3] --, econ, val = com
	print(tblk,econ,val)
	local langkey = string.upper(tblk)
	cargo[tblk] = EquipType.New({
		l10n_key = langkey, l10n_resource = "cargo", slots="cargo", icon_name = tblk,
		capabilities={mass=1}, economy_type = econ, purchasable=true, price = val
	})
end

-- lua has problems with strings in fields where numbers are expected
local MILTECH = 999

misc = {}
misc.missile_unguided = EquipType.New({
	l10n_key="MISSILE_UNGUIDED", slots="missile",
	capabilities={mass=1, missile=1, durability=1000000},
	icon_name="missile_unguided", missile_type="missile_unguided",
	purchasable=true, tech_level=1, stock=2000, stockmod=200, price=30
})
misc.missile_guided = EquipType.New({
	l10n_key="MISSILE_GUIDED", slots="missile",
	capabilities={mass=1, durability=1000000},
	icon_name="missile_guided", missile_type="missile_guided",
	purchasable=true, tech_level=5, stock=1000, stockmod=0, price=50
})
misc.missile_smart = EquipType.New({
	l10n_key="MISSILE_SMART", slots="missile",
	capabilities={mass=1, durability=1000000},
	icon_name="missile_smart", missile_type="missile_smart",
	purchasable=true, tech_level=10, stock=500, stockmod=0, price=95
})
misc.missile_naval = EquipType.New({
	l10n_key="MISSILE_NAVAL", slots="missile",
	capabilities={mass=1, durability=1000000},
	icon_name="missile_naval", missile_type="missile_naval",
	purchasable=true, tech_level=MILTECH, stock=350, stockmod=0, price=160,
})
misc.atmospheric_shielding = EquipType.New({
	l10n_key="ATMOSPHERIC_SHIELDING", slots="atmo_shield",
	capabilities={mass=1, atmo_shield=1, durability=1000000},
	purchasable=true, tech_level=3, stock=100, stockmod=0, price=200
})
misc.ecm_basic = EquipType.New({
	l10n_key="ECM_BASIC", slots="ecm", 
	capabilities={mass=2, ecm_power=2, ecm_recharge=5, durability=1000000},
	ecm_type = 'ecm',
	purchasable=true, tech_level=9, stock=100, stockmod=0, price=6000
})
misc.ecm_advanced = EquipType.New({
	l10n_key="ECM_ADVANCED", slots="ecm",
	capabilities={mass=2, ecm_power=3, ecm_recharge=5, durability=1000000},
	ecm_type = 'ecm_advanced',
	purchasable=true, tech_level=MILTECH, stock=100, stockmod=0, price=15200
})
misc.radar = EquipType.New({
	l10n_key="RADAR", slots="radar",
	capabilities={mass=1, radar=1, durability=1000000},
	purchasable=true, tech_level=3, stock=100, stockmod=0, price=680
})
misc.cabin = EquipType.New({
	l10n_key="UNOCCUPIED_CABIN", slots="cabin",
	capabilities={mass=1, cabin=1, durability=1000000},
	purchasable=true, tech_level=1, stock=100, stockmod=0, price=1350
})
misc.cabin_occupied = EquipType.New({
	l10n_key="PASSENGER_CABIN", slots="cabin",
	capabilities={mass=1, durability=1000000},
	purchasable=false, tech_level=1, stock=0, stockmod=0, price=0
})
misc.shield_generator = EquipType.New({
	l10n_key="SHIELD_GENERATOR", slots="shield",
	capabilities={mass=4, shield=1, durability=1000000},
	purchasable=true, tech_level=8, stock=100, stockmod=0, price=2500
})
misc.laser_cooling_booster = EquipType.New({
	l10n_key="LASER_COOLING_BOOSTER", slots="laser_cooler",
	capabilities={mass=1, laser_cooler=2, durability=1000000},
	purchasable=true, tech_level=8, stock=100, stockmod=0, price=380
})
misc.cargo_life_support = EquipType.New({
	l10n_key="CARGO_LIFE_SUPPORT", slots="cargo_life_support",
	capabilities={mass=1, cargo_life_support=1, durability=1000000},
	purchasable=true, tech_level=2, stock=100, stockmod=0, price=700
})
misc.autopilot = EquipType.New({
	l10n_key="AUTOPILOT", slots="autopilot",
	capabilities={mass=1, set_speed=1, autopilot=1, durability=1000000},
	purchasable=true, tech_level=1, stock=100, stockmod=0, price=1400
})
misc.target_scanner = EquipType.New({
	l10n_key="TARGET_SCANNER", slots="target_scanner",
	capabilities={mass=1, target_scanner_level=1, durability=1000000},
	purchasable=true, tech_level=9, stock=100, stockmod=0, price=900
})
misc.advanced_target_scanner = EquipType.New({
	l10n_key="ADVANCED_TARGET_SCANNER", slots="target_scanner", 
	capabilities={mass=1, target_scanner_level=2, durability=1000000},
	purchasable=true, tech_level=MILTECH, stock=100, stockmod=0, price=1200
})
misc.fuel_scoop = EquipType.New({
	l10n_key="FUEL_SCOOP", slots="scoop", 
	capabilities={mass=6, fuel_scoop=3, durability=1000000},
	purchasable=true, tech_level=4, stock=100, stockmod=0, price=3500
})
misc.cargo_scoop = EquipType.New({
	l10n_key="CARGO_SCOOP", slots="scoop", 
	capabilities={mass=7, cargo_scoop=1, durability=1000000},
	purchasable=true, tech_level=5, stock=100, stockmod=0, price=3900
})
misc.multi_scoop = EquipType.New({
	l10n_key="MULTI_SCOOP", slots="scoop",
	capabilities={mass=9, cargo_scoop=1, fuel_scoop=2, durability=1000000},
	purchasable=true, tech_level=9, stock=100, stockmod=0, price=12000
})
misc.hypercloud_analyzer = EquipType.New({
	l10n_key="HYPERCLOUD_ANALYZER", slots="hypercloud", 
	capabilities={mass=1, hypercloud_analyzer=1, durability=1000000},
	purchasable=true, tech_level=10, stock=100, stockmod=0, price=1500
})
misc.shield_energy_booster = EquipType.New({
	l10n_key="SHIELD_ENERGY_BOOSTER", slots="energy_booster",
	capabilities={mass=8, shield_energy_booster=1, durability=1000000},
	purchasable=true, tech_level=11, stock=100, stockmod=0, price=10000
})
misc.hull_autorepair = EquipType.New({
	l10n_key="HULL_AUTOREPAIR", slots="hull_autorepair", 
	capabilities={mass=40, hull_autorepair=1, durability=1000000},
	purchasable=true, tech_level=MILTECH, stock=100, stockmod=0, price=16000
})
misc.thrusters_basic = EquipType.New({
	l10n_key="THRUSTERS_BASIC", slots="thruster", 
	capabilities={mass=0, thruster_power=1, durability=1000000},
	icon_name="thrusters_basic",
	purchasable=true, tech_level=1, stock=100, stockmod=0, price=3000
})
misc.thrusters_medium = EquipType.New({
	l10n_key="THRUSTERS_MEDIUM", slots="thruster", 
	capabilities={mass=0, thruster_power=2, durability=1000000},
	icon_name="thrusters_medium",
	purchasable=true, tech_level=1, stock=100, stockmod=0, price=6500
})
misc.thrusters_best = EquipType.New({
	l10n_key="THRUSTERS_BEST", slots="thruster",
	capabilities={mass=0, thruster_power=3, durability=1000000},
	icon_name="thrusters_best",
	purchasable=true, tech_level=1, stock=100, stockmod=0, price=14000
})
misc.trade_computer = EquipType.New({
	l10n_key="TRADE_COMPUTER", slots="trade_computer",
	capabilities={mass=0, trade_computer=1, durability=1000000},
	purchasable=true, tech_level=9, stock=100, stockmod=0, price=400
})
misc.planetscanner = BodyScannerType.New({
	l10n_key = 'PLANETSCANNER', slots="sensor", 
	capabilities={mass=1,sensor=1, durability=1000000},
	bodyscanner_stats={scan_speed=3, scan_tolerance=0.05},
	icon_on_name="body_scanner_on", icon_off_name="body_scanner_off",
	max_range=100000000, target_altitude=0, state="HALTED", progress=0,
	purchasable=false, tech_level=1, stock=100, stockmod=0, price=15000
})

-- calculate power: 625*hyperclass^2
-- price for solfed drives calculated as mass * 1750 + slight variation
-- corello class price = solfed * 3 + slight variation
hyperspace = {}
hyperspace.hyperdrive_1 = HyperdriveType.New({
	l10n_key="DRIVE_CLASS1", fuel=cargo.hydrogen, slots="engine",
	capabilities={mass=4, hyperclass=1, speed=100, warmup=7, durability=1000000,
	power=625, maxfuel=1 },
	purchasable=true, tech_level=3, stock=75, stockmod=0, price=6500 -- slightly underpriced
})
hyperspace.hyperdrive_2 = HyperdriveType.New({
	l10n_key="DRIVE_CLASS2", fuel=cargo.hydrogen, slots="engine",
	capabilities={mass=10, hyperclass=2, speed=100, warmup=8, durability=1000000,
	power=2500, maxfuel=4 },
	purchasable=true, tech_level=4, stock=64, stockmod=0, price=17000 --slightly underpriced
})
-- corello corporation hyperdrive, "better quality, lighter, 20% shorter traveling time, shorter warmup, but quality costs"
hyperspace.corello_class2 = HyperdriveType.New({
	l10n_key="DRIVE_CLASS2_CORELLO", fuel=cargo.hydrogen, slots="engine",
	capabilities={mass=9, hyperclass=2, speed=120, warmup=7, durability=1200000, -- slightly higher durability (+20%)
	power=2500, maxfuel=4 },
	purchasable=true, tech_level=7, stock=3, stockmod=0, price=51500
})
hyperspace.hyperdrive_3 = HyperdriveType.New({
	l10n_key="DRIVE_CLASS3", fuel=cargo.hydrogen, slots="engine",
	capabilities={mass=20, hyperclass=3, speed=100, warmup=9, durability=1000000,
	power=5625, maxfuel=9 },
	purchasable=true, tech_level=4, stock=58, stockmod=0, price=34500 -- slightly underpriced
})
hyperspace.hyperdrive_4 = HyperdriveType.New({
	l10n_key="DRIVE_CLASS4", fuel=cargo.hydrogen, slots="engine",
	capabilities={mass=40, hyperclass=4, speed=100, warmup=11, durability=1000000,
	power=10000, maxfuel=16 },
	purchasable=true, tech_level=5, stock=45, stockmod=0, price=69000 -- slightly underpriced
})
hyperspace.hyperdrive_5 = HyperdriveType.New({
	l10n_key="DRIVE_CLASS5", fuel=cargo.hydrogen, slots="engine",
	capabilities={mass=120, hyperclass=5, speed=100, warmup=13, durability=1000000,
	power=15625, maxfuel=25 },
	purchasable=true, tech_level=5, stock=37, stockmod=0, price=198500 -- underpriced
})
hyperspace.hyperdrive_6 = HyperdriveType.New({
	l10n_key="DRIVE_CLASS6", fuel=cargo.hydrogen, slots="engine",
	capabilities={mass=225, hyperclass=6, speed=100, warmup=14, durability=1000000,
	power=22500, maxfuel=36 },
	purchasable=true, tech_level=6, stock=20, stockmod=0, price=397000 -- overpriced
})
hyperspace.hyperdrive_7 = HyperdriveType.New({
	l10n_key="DRIVE_CLASS7", fuel=cargo.hydrogen, slots="engine",
	capabilities={mass=400, hyperclass=7, speed=100, warmup=16, durability=1000000,
	power=30625, maxfuel=49 },
	purchasable=true, tech_level=8, stock=14, stockmod=0, price=675000 -- underpriced
})
hyperspace.hyperdrive_8 = HyperdriveType.New({
	l10n_key="DRIVE_CLASS8", fuel=cargo.hydrogen, slots="engine",
	capabilities={mass=580, hyperclass=8, speed=100, warmup=17, durability=1000000,
	power=40000, maxfuel=64 },
	purchasable=true, tech_level=9, stock=10, stockmod=0, price=985000 -- underpriced
})
hyperspace.hyperdrive_9 = HyperdriveType.New({
	l10n_key="DRIVE_CLASS9", fuel=cargo.hydrogen, slots="engine",
	capabilities={mass=740, hyperclass=9, speed=100, warmup=19, durability=1000000,
	power=50625, maxfuel=81 },
	purchasable=true, tech_level=10, stock=6, stockmod=0, price=1295000 -- exact
})
-- price of military drives: mass * 4750 + (class^2 * 10000) + slight variation
hyperspace.hyperdrive_mil1 = HyperdriveType.New({
	l10n_key="DRIVE_MIL1", fuel=cargo.military_fuel, byproduct=cargo.radioactives, slots="engine",
	capabilities={mass=3, hyperclass=1, speed=160, warmup=5, durability=1000000,
	power=625, maxfuel=1 },
	purchasable=true, tech_level=10, stock=12, stockmod=0, price=24500
})
hyperspace.hyperdrive_mil2 = HyperdriveType.New({
	l10n_key="DRIVE_MIL2", fuel=cargo.military_fuel, byproduct=cargo.radioactives, slots="engine",
	capabilities={mass=8, hyperclass=2, speed=160, warmup=5, durability=1000000,
	power=2500, maxfuel=4 },
	purchasable=true, tech_level=MILTECH, stock=1, stockmod=0, price=76500
})
hyperspace.hyperdrive_mil3 = HyperdriveType.New({
	l10n_key="DRIVE_MIL3", fuel=cargo.military_fuel, byproduct=cargo.radioactives, slots="engine",
	capabilities={mass=16, hyperclass=3, speed=160, warmup=6, durability=1000000,
	power=5625, maxfuel=9 },
	purchasable=true, tech_level=11, stock=8, stockmod=0, price=168000
})
hyperspace.hyperdrive_mil4 = HyperdriveType.New({
	l10n_key="DRIVE_MIL4", fuel=cargo.military_fuel, byproduct=cargo.radioactives, slots="engine",
	capabilities={mass=30, hyperclass=4, speed=160, warmup=7, durability=1000000,
	power=10000, maxfuel=16 },
	purchasable=true, tech_level=12, stock=5, stockmod=0, price=298000
})
hyperspace.hyperdrive_mil5 = HyperdriveType.New({
	l10n_key="DRIVE_MIL5", fuel=cargo.military_fuel, byproduct=cargo.radioactives, slots="engine",
	capabilities={mass=53, hyperclass=5, speed=160, warmup=7, durability=1000000,
	power=15625, maxfuel=25 },
	purchasable=false, tech_level=MILTECH, stock=1, stockmod=0, price=485500
})
hyperspace.hyperdrive_mil6 = HyperdriveType.New({
	l10n_key="DRIVE_MIL6", fuel=cargo.military_fuel, byproduct=cargo.radioactives, slots="engine",
	capabilities={mass=78, hyperclass=6, speed=160, warmup=8, durability=1000000,
	power=22500, maxfuel=36 },
	purchasable=false, tech_level=MILTECH, stock=1, stockmod=0, price=736500
})
hyperspace.hyperdrive_mil7 = HyperdriveType.New({
	l10n_key="DRIVE_MIL7", fuel=cargo.military_fuel, byproduct=cargo.radioactives, slots="engine",
	capabilities={mass=128, hyperclass=7, speed=160, warmup=9, durability=1000000,
	power=30625, maxfuel=49 },
	purchasable=false, tech_level=MILTECH, stock=1, stockmod=0, price=1121000
})
hyperspace.hyperdrive_mil8 = HyperdriveType.New({
	l10n_key="DRIVE_MIL8", fuel=cargo.military_fuel, byproduct=cargo.radioactives, slots="engine",
	capabilities={mass=196, hyperclass=8, speed=160, warmup=10, durability=1000000,
	power=40000, maxfuel=64 },
	purchasable=false, tech_level=MILTECH, stock=1, stockmod=0, price=1471000
})
hyperspace.hyperdrive_mil9 = HyperdriveType.New({
	l10n_key="DRIVE_MIL9", fuel=cargo.military_fuel, byproduct=cargo.radioactives, slots="engine",
	capabilities={mass=285, hyperclass=9, speed=160, warmup=10, durability=1000000,
	power=50625, maxfuel=81 },
	purchasable=false, tech_level=MILTECH, stock=1, stockmod=0, price=2368000
})

laser = {}
laser.pulsecannon_1mw = LaserType.New({
	l10n_key="PULSECANNON_1MW", 
	capabilities={mass=1, durability=1000000},
	slots = {"laser_front", "laser_rear"}, laser_stats = {
		lifespan=8, speed=1000, damage=1000, rechargeTime=0.25, length=30,
		width=5, dual=0, mining=0, rgba_r = 255, rgba_g = 51, rgba_b = 51, rgba_a = 255	},
	purchasable=true, tech_level=3, stock=100, stockmod=0, price=600
})
laser.pulsecannon_dual_1mw = LaserType.New({
	l10n_key="PULSECANNON_DUAL_1MW",
	capabilities={mass=4, durability=1000000},
	slots = {"laser_front", "laser_rear"}, laser_stats = {
		lifespan=8, speed=1000, damage=1000, rechargeTime=0.25, length=30,
		width=5, dual=1, mining=0, rgba_r = 255, rgba_g = 51, rgba_b = 51, rgba_a = 255	},
	purchasable=true, tech_level=4, stock=100, stockmod=0, price=1100
})
laser.pulsecannon_2mw = LaserType.New({
	l10n_key="PULSECANNON_2MW",
	capabilities={mass=3, durability=1000000},
	slots = {"laser_front", "laser_rear"}, laser_stats = {
		lifespan=8, speed=1000, damage=2000, rechargeTime=0.25, length=30,
		width=5, dual=0, mining=0, rgba_r = 255, rgba_g = 127, rgba_b = 51, rgba_a = 255 },
	purchasable=true, tech_level=5, stock=100, stockmod=0, price=1000
})
laser.pulsecannon_rapid_2mw = LaserType.New({
	l10n_key="PULSECANNON_RAPID_2MW",
	capabilities={mass=7, durability=1000000},
	slots = {"laser_front", "laser_rear"}, laser_stats = {
		lifespan=8, speed=1000, damage=2000, rechargeTime=0.13, length=30,
		width=5, dual=0, mining=0, rgba_r = 255, rgba_g = 127, rgba_b = 51, rgba_a = 255 },
	purchasable=true, tech_level=5, stock=100, stockmod=0, price=1800
})
laser.pulsecannon_4mw = LaserType.New({
	l10n_key="PULSECANNON_4MW",
	capabilities={mass=10, durability=1000000},
	slots = {"laser_front", "laser_rear"}, laser_stats = {
		lifespan=8, speed=1000, damage=4000, rechargeTime=0.25, length=30,
		width=5, dual=0, mining=0, rgba_r = 255, rgba_g = 255, rgba_b = 51, rgba_a = 255 },
	purchasable=true, tech_level=6, stock=100, stockmod=0, price=2200
})
laser.pulsecannon_10mw = LaserType.New({
	l10n_key="PULSECANNON_10MW",
	capabilities={mass=30, durability=1000000},
	slots = {"laser_front", "laser_rear"}, laser_stats = {
		lifespan=8, speed=1000, damage=10000, rechargeTime=0.25, length=30,
		width=5, dual=0, mining=0, rgba_r = 51, rgba_g = 255, rgba_b = 51, rgba_a = 255	},
	purchasable=true, tech_level=7, stock=100, stockmod=0, price=4900
})
laser.pulsecannon_20mw = LaserType.New({
	l10n_key="PULSECANNON_20MW",
	capabilities={mass=65, durability=1000000},
	slots = {"laser_front", "laser_rear"}, laser_stats = {
		lifespan=8, speed=1000, damage=20000, rechargeTime=0.25, length=30,
		width=5, dual=0, mining=0, rgba_r = 0.1, rgba_g = 51, rgba_b = 255, rgba_a = 255 },
	purchasable=true, tech_level=MILTECH, stock=100, stockmod=0, price=12000
})
laser.miningcannon_17mw = LaserType.New({
	l10n_key="MININGCANNON_17MW",
	capabilities={mass=10, durability=1000000},
	slots = {"laser_front", "laser_rear"}, laser_stats = {
		lifespan=8, speed=1000, damage=17000, rechargeTime=2, length=30,
		width=5, dual=0, mining=1, rgba_r = 51, rgba_g = 127, rgba_b = 0, rgba_a = 255 },
	purchasable=true, tech_level=8, stock=100, stockmod=0, price=10600
})
laser.small_plasma_accelerator = LaserType.New({
	l10n_key="SMALL_PLASMA_ACCEL",
	capabilities={mass=22, durability=1000000},
	slots = {"laser_front", "laser_rear"}, laser_stats = {
		lifespan=8, speed=1000, damage=50000, rechargeTime=0.3, length=42,
		width=7, dual=0, mining=0, rgba_r = 51, rgba_g = 255, rgba_b = 255, rgba_a = 255 },
	purchasable=true, tech_level=10, stock=100, stockmod=0, price=120000
})
laser.large_plasma_accelerator = LaserType.New({
	l10n_key="LARGE_PLASMA_ACCEL",
	capabilities={mass=50, durability=1000000},
	slots = {"laser_front", "laser_rear"}, laser_stats = {
		lifespan=8, speed=1000, damage=100000, rechargeTime=0.3, length=42,
		width=7, dual=0, mining=0, rgba_r = 127, rgba_g = 255, rgba_b = 255, rgba_a = 255 },
	purchasable=true, tech_level=12, stock=100, stockmod=0, price=390000
})

local equipment = {
	cargo=cargo,
	laser=laser,
	hyperspace=hyperspace,
	misc=misc,
	LaserType=LaserType,
	HyperdriveType=HyperdriveType,
	EquipType=EquipType,
	SensorType=SensorType,
	BodyScannerType=BodyScannerType
}

local serialize = function()
	local ret = {}
	for _,k in ipairs{"cargo","laser", "hyperspace", "misc"} do
		local tmp = {}
		for kk, vv in pairs(equipment[k]) do
			tmp[kk] = vv
		end
		ret[k] = tmp
	end
	return ret
end

local unserialize = function (data)
	for _,k in ipairs{"cargo","laser", "hyperspace", "misc"} do
		local tmp = equipment[k]
		for kk, vv in pairs(data[k]) do
			tmp[kk] = vv
		end
	end
end

Serializer:Register("Equipment", serialize, unserialize)
Serializer:RegisterClass("LaserType", LaserType)
Serializer:RegisterClass("EquipType", EquipType)
Serializer:RegisterClass("HyperdriveType", HyperdriveType)
Serializer:RegisterClass("SensorType", SensorType)
Serializer:RegisterClass("BodyScannerType", BodyScannerType)

return equipment
