-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local EquipType = require 'EquipType'
local Game		= require 'Game'
local ShipDef   = require 'ShipDef'

local utils = require 'utils'

--==============================================================================

-- Class: LaserType
--
-- LaserType is the (somewhat misnamed) base type for most projectile weapons.
--
---@class Equipment.LaserType : EquipType
---@field laser_stats table
---@field weapon_data table
local LaserType = EquipType:NewType("Equipment.LaserType")

function LaserType:Constructor(specs)
	local ls = specs.laser_stats

	-- NOTE: backwards-compatibility with old laser_stats definitions
	if ls then

		local projectile = {
			lifespan = ls.lifespan,
			speed = ls.speed,
			damage = ls.damage,
			beam = ls.beam == 1,
			mining = ls.mining == 1,
			length = ls.length,
			width = ls.width,
			color = Color(ls.rgba_r, ls.rgba_g, ls.rgba_b, ls.rgba_a),
		}

		self.weapon_data = {
			rpm = 60 / ls.rechargeTime,
			heatPerShot = ls.heatrate or 0.01,
			cooling = ls.coolrate or 0.01,
			overheat = 1.0,
			projectile = projectile,
			numBarrels = 1 + ls.dual
		}

	end
end

---@param ship Ship
---@param slot HullConfig.Slot
function LaserType:OnInstall(ship, slot)
	EquipType.OnInstall(self, ship, slot)

	ship:GetComponent('GunManager'):MountWeapon(slot.id, self.weapon_data)
end

---@param ship Ship
---@param slot HullConfig.Slot
function LaserType:OnRemove(ship, slot)
	EquipType.OnRemove(self, ship, slot)

	ship:GetComponent('GunManager'):UnmountWeapon(slot.id)
end

--==============================================================================

-- Class: HyperdriveType
--
-- HyperdriveType is responsible for most logic related to computing a hyperspace jump.
-- The actual logic for executing the jump is delegated to Ship, but the drive is responsible
-- for checking that a jump is possible and deducting fuel.
--
-- Note that no attempt is made to model multiple hyperdrives, including backup drives;
-- redundancy in hyperdrives should be achieved by repairing the existing drive or installing
-- a replacement drive from the ship's stores while underway.
--
---@class Equipment.HyperdriveType : EquipType
---@field fuel CommodityType
---@field byproduct CommodityType?
local HyperdriveType = EquipType:NewType("Equipment.HyperdriveType")

HyperdriveType.storedFuel = 0.0

-- Static factors
HyperdriveType.factor_eff = 55
HyperdriveType.factor_time = 1.0
HyperdriveType.fuel_resv_size = 1.0

-- Travel time exponents
local range_exp = 1.6
local mass_exp = 1.2
-- Fuel use exponents
local fuel_use_exp = 0.9
local fuel_use_exp_inv = 1.0 / 0.9

---@param ship Ship
---@param extraMass number Additional mass beyond current lading to include in calculation
function HyperdriveType:GetEfficiencyTerm(ship, extraMass)
	return self.factor_eff / (ship.staticMass + ship.fuelMassLeft + extraMass)^(3/5) * (1 + self.capabilities.hyperclass) / self.capabilities.hyperclass
end

-- Function: GetMaximumRange
--
-- Returns the maximum range of the hyperdrive under their current lading state of the ship.
---@param ship Ship
function HyperdriveType:GetMaximumRange(ship)
	-- Account for the extra mass needed to reach full fuel state
	local E = self:GetEfficiencyTerm(ship, self.fuel_resv_size - self.storedFuel)
	return (self.fuel_resv_size * E)^fuel_use_exp_inv
end

-- Function: GetFuelUse
--
-- Returns the fuel consumed by the drive in attempting a jump of the specified distance.
---@param ship Ship
---@param distance number Distance to jump, in light years
---@return number fuelUsed Amount of fuel used, in tons
function HyperdriveType:GetFuelUse(ship, distance)
	return distance^fuel_use_exp / self:GetEfficiencyTerm(ship, 0)
end

-- Function: GetDuration
--
-- Returns the duration taken for a jump of the specified distance.
---@param ship Ship
---@param distance number Distance to jump, in light years
---@return number duration Duration of the jump, in seconds
function HyperdriveType:GetDuration(ship, distance, range_max)
	local mass = ship.staticMass + ship.fuelMassLeft
	return 86400 * 0.36 * (distance^range_exp * mass^mass_exp) / (425 * self.capabilities.hyperclass^2 * self.factor_time)
end

-- Function: GetRange
--
-- Return the current jump range of the ship given the current fuel state,
-- and the maximum jump range of the ship at its current lading if the drive were fully fueled.
---@param ship Ship
---@return number currentRange
---@return number maxRange
function HyperdriveType:GetRange(ship)
	local E, E1 = self:GetEfficiencyTerm(ship, 0), self:GetEfficiencyTerm(ship, self.fuel_resv_size - self.storedFuel)
	return (self.storedFuel * E)^fuel_use_exp_inv, (self.fuel_resv_size * E1)^fuel_use_exp_inv
end

-- Function: SetFuel
--
-- Update the amount of fuel stored in the drive's internal reservoir.
-- The amount of fuel must be equal to or less than the fuel_resv_size of the drive.
---@param ship Ship? The ship this hyperdrive is installed in, or nil if the hyperdrive is not yet installed
---@param newFuel number The fuel amount stored in the hyperdrive's reservoir, in tons
function HyperdriveType:SetFuel(ship, newFuel)
	assert(newFuel >= 0 and newFuel <= self.fuel_resv_size)

	local massDiff = newFuel - self.storedFuel
	self.storedFuel = newFuel
	self.mass = self.mass + massDiff

	if ship then
		ship:setprop("mass_cap", ship["mass_cap"] + massDiff)
		ship:UpdateEquipStats()

		ship:GetComponent('EquipSet'):NotifyListeners('modify', self)
	end
end

-- Function: GetMaxFuel
--
-- Return the maximum fuel capacity of the drive, in tons.
function HyperdriveType:GetMaxFuel()
	return self.fuel_resv_size
end

-- Function: CheckJump
--
-- Checks the viability of a potential jump from are source SystemPath to a destination SystemPath.
--
-- if the destination is reachable, returns: distance, fuel, duration
-- if the destination is out of range, returns: distance
-- if the specified jump is invalid, returns nil
---@param ship Ship
---@param source SystemPath
---@param destination SystemPath
function HyperdriveType:CheckJump(ship, source, destination)
	if ship:GetInstalledHyperdrive() ~= self or source:IsSameSystem(destination) then
		return nil
	end
	local distance = source:DistanceTo(destination)
	local max_range = self:GetRange(ship) -- takes fuel into account
	if distance > max_range then
		return distance
	end
	local fuel = self:GetFuelUse(ship, distance)

	local duration = self:GetDuration(ship, distance, max_range) -- same as above
	return distance, fuel, duration
end

-- Function: CheckDestination
--
-- Like HyperdriveType.CheckJump, but uses Game.system as the source system
--
-- if the destination is reachable, returns: distance, fuel, duration
-- if the destination is out of range, returns: distance
-- if the specified jump is invalid, returns nil
---@param ship Ship
---@param destination SystemPath
function HyperdriveType:CheckDestination(ship, destination)
	if not Game.system then
		return nil
	end
	return self:CheckJump(ship, Game.system.path, destination)
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

-- Function: HyperjumpTo
--
-- Perform safety checks and initiate a hyperjump to the destination system
---@param ship Ship
---@param destination SystemPath
function HyperdriveType:HyperjumpTo(ship, destination)
	-- First off, check that this is the primary engine.
	-- NOTE: this enforces the constraint that only one hyperdrive may be installed on a ship
	local engine = ship:GetInstalledHyperdrive()
	if engine ~= self then
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
	local warmup_time = 5 + self.capabilities.hyperclass*1.5

	local sounds
	if self.fuel.name == 'military_fuel' then
		sounds = HYPERDRIVE_SOUNDS_MILITARY
	else
		sounds = HYPERDRIVE_SOUNDS_NORMAL
	end

	return ship:InitiateHyperjumpTo(destination, warmup_time, duration, sounds), fuel_use, duration
end

-- Function: OnLeaveHyperspace
--
-- Handle cleanup after leaving hyperspace
---@param ship Ship
function HyperdriveType:OnLeaveHyperspace(ship)
	if ship:hasprop('nextJumpFuelUse') then
		local amount = ship['nextJumpFuelUse']
		ship:unsetprop('nextJumpFuelUse')

		self:SetFuel(ship, math.max(0, self.storedFuel - amount))

		if self.byproduct then
			local cargoMgr = ship:GetComponent('CargoManager')
			-- TODO: byproduct can be "bypassed" when jumping on a full cargo hold
			local byproductAmount = math.min(math.ceil(amount), cargoMgr:GetFreeSpace())
			cargoMgr:AddCommodity(self.byproduct, byproductAmount)
		end
	end
end

--==============================================================================

-- NOTE: "sensors" have no general-purpose code associated with the equipment type
---@class Equipment.SensorType : EquipType
local SensorType = EquipType:NewType("Equipment.SensorType")

--==============================================================================

-- NOTE: all code related to managing a body scanner is implemented in the ScanManager component
---@class Equipment.BodyScannerType : EquipType
---@field stats table
local BodyScannerType = SensorType:NewType("Equipment.BodyScannerType")

--==============================================================================

---@class Equipment.CabinType : EquipType
---@field passengers Character[]?
local CabinType = EquipType:NewType("Equipment.CabinType")

---@param passenger Character
function CabinType:AddPassenger(passenger)
	table.insert(self.passengers, passenger)
	self.icon_name = "equip_cabin_occupied"
end

---@param passenger Character
function CabinType:RemovePassenger(passenger)
	utils.remove_elem(self.passengers, passenger)
	if #self.passengers == 0 then
		self.icon_name = "equip_cabin_empty"
	end
end

function CabinType:HasPassenger(passenger)
	return utils.contains(self.passengers, passenger)
end

function CabinType:GetNumPassengers()
	return self.passengers and #self.passengers or 0
end

function CabinType:GetMaxPassengers()
	return self.capabilities.cabin
end

function CabinType:GetFreeBerths()
	return self.capabilities.cabin - (self.passengers and #self.passengers or 0)
end

function CabinType:OnInstall(ship, slot)
	EquipType.OnInstall(self, ship, slot)

	self.passengers = {}
end

function CabinType:OnRemove(ship, slot)
	EquipType.OnRemove(self, ship, slot)

	if #self.passengers > 0 then
		logWarning("Removing passenger cabin with passengers onboard!")
		ship:setprop("cabin_occupied_cap", ship["cabin_occupied_cap"] - #self.passengers)
	end
end

--==============================================================================

---@class Equipment.ThrusterType : EquipType
local ThrusterType = EquipType:NewType("Equipment.ThrusterType")

--==============================================================================

---@class Equipment.MissileType : EquipType
---@field missile_stats table
local MissileType = EquipType:NewType("Equipment.MissileType")

--==============================================================================

---@class Equipment.ShieldType : EquipType
local ShieldType = EquipType:NewType("Equipment.ShieldType")

--==============================================================================

---@class Equipment.CargoScoopType : EquipType
local CargoScoopType = EquipType:NewType("Equipment.CargoScoopType")

CargoScoopType.volumePerSize = 1.5
CargoScoopType.massPerSize = 1.5
CargoScoopType.pricePerCargo = 10
CargoScoopType.pricePerSize = 0.4

---@param config HullConfig
function CargoScoopType:SpecializeForShip(config)
	local def = ShipDef[config.id]
	if not def then return end

	local hullSlot = utils.best_score(config.slots, function(k, v)
		return v.type == "hull" and 1 or nil
	end)

	if not hullSlot then return end

	-- Effective ship size, clamped to S1
	local size = math.max(hullSlot.size, 1)

	local cargoPriceMod = def.cargo * self.pricePerCargo

	local sizeMassMod = (size - 1) * self.massPerSize * self.mass
	local sizePriceMod = (size - 1) * self.pricePerSize * self.price
	local sizeVolumeMod = (size - 1) * self.volumePerSize * self.volume

	self.volume = utils.round(self.volume + sizeVolumeMod, 0.5)
	self.mass = utils.round(self.mass + sizeMassMod, 0.1)
	self.price = self.price + utils.round(sizePriceMod + cargoPriceMod, 10.0)
end

--==============================================================================

return {
	EquipType		= EquipType,
	CargoScoopType	= CargoScoopType,
	LaserType		= LaserType,
	HyperdriveType	= HyperdriveType,
	SensorType		= SensorType,
	BodyScannerType	= BodyScannerType,
	CabinType       = CabinType,
	ThrusterType    = ThrusterType,
	MissileType     = MissileType,
	ShieldType		= ShieldType,
}
