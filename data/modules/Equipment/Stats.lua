-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local EquipTypes = require '.Types'
local Lang = require 'Lang'

local utils = require 'utils'

local lc = Lang.GetResource("core")
local le = Lang.GetResource("equipment-core")

local ui = require 'pigui'
local icons = ui.theme.icons

--==============================================================================

---@class EquipType
local EquipType = EquipTypes.EquipType

---@alias EquipType.UI.Stats { [1]:string, [2]:ui.Icon, [3]:any, [4]:(fun(v: any):string), [5]:boolean? }

local format_integrity = function(v) return string.format("%d%%", v * 100) end
local format_mass = function(v) return ui.Format.Mass(v * 1000, 1) end
local format_power = function(v) return string.format("%.1f KW", v) end
local format_multiplier = function(v) return string.format("%.1fx", v) end

---@return EquipType.UI.Stats[]
function EquipType:GetDetailedStats()
	local equipHealth = 1
	local powerDraw = 0

	local stats = {
		{ le.EQUIPMENT_INTEGRITY, icons.repairs, equipHealth, format_integrity },
		{ le.STAT_VOLUME, icons.square, self.volume, ui.Format.Volume, true },
		{ le.STAT_WEIGHT, icons.hull, self.mass, format_mass, true },
		{ le.STAT_POWER_DRAW, icons.ecm, powerDraw, format_power, true }
	}

	if self.capabilities and self.capabilities.atmo_shield then
		table.insert(stats, { le.STAT_ATMO_SHIELD, icons.equip_atmo_shield_generator, self.capabilities.atmo_shield, format_multiplier })
	end

	return stats
end

---@return table[]
function EquipType:GetItemCardStats()
	return {
		{ icons.square, ui.Format.Volume(self.volume) },
		{ icons.hull, format_mass(self.mass) },
		{ icons.ecm, format_power(0) },
		{ icons.repairs, format_integrity(1) }
	}
end

--==============================================================================

---@class Equipment.LaserType
local LaserType = EquipTypes.LaserType

local format_rpm = function(v) return string.format("%d RPM", 60 / v) end
local format_speed = function(v) return string.format("%.1f%s", v, lc.UNIT_METERS_PER_SECOND) end

function LaserType:GetDetailedStats()
	local out = self:Super().GetDetailedStats(self)

	return table.append(out, {
		{
			le.SHOTS_PER_MINUTE,
			icons.comms, -- PLACEHOLDER
			self.laser_stats.rechargeTime,
			format_rpm,
			true -- lower is better
		},
		{
			le.DAMAGE_PER_SHOT,
			icons.ecm_advanced,
			self.laser_stats.damage,
			format_power
		},
		{
			le.PROJECTILE_SPEED,
			icons.forward,
			self.laser_stats.speed,
			format_speed
		}
	})
end

--==============================================================================

local format_efficiency = function(v) return string.format("%d%s", v, le.UNIT_HYPERDRIVE_EFFICIENCY) end

---@class Equipment.HyperdriveType
local HyperdriveType = EquipTypes.HyperdriveType

function HyperdriveType:GetDetailedStats()
	local out = self:Super().GetDetailedStats(self)

	return table.append(out, {
		{ le.STAT_HYPERDRIVE_RESERVOIR, icons.fuel, self.fuel_resv_size, format_mass },
		{ le.STAT_HYPERDRIVE_EFFICIENCY, icons.autopilot_fly_to, self.factor_eff, format_efficiency },
	})
end

--==============================================================================

local format_damage = function(v) return string.format("%d%s", v, le.UNIT_WARHEAD_DAMAGE) end

---@class Equipment.MissileType
local MissileType = EquipTypes.MissileType

function MissileType:GetDetailedStats()
	local out = self:Super().GetDetailedStats(self)

	return table.append(out, {
		{ le.STAT_MISSILE_RADIUS, icons.follow_pos, self.missile_stats.fuzeRadius, ui.Format.Distance },
		-- TODO: need a better icon for "explosive"
		{ le.STAT_MISSILE_WARHEAD, icons.galaxy_map, self.missile_stats.warheadSize, format_damage },
	})
end

---@return table[]
function MissileType:GetItemCardStats()
	return {
		{ icons.follow_pos, ui.Format.Distance(self.missile_stats.fuzeRadius) },
		-- TODO: need a better icon for "explosive"
		{ icons.galaxy_map, ui.Format.Mass(self.missile_stats.warheadSize, 0) },
		{ icons.hull, format_mass(self.mass) },
		{ icons.repairs, format_integrity(1) }
	}
end

--==============================================================================

---@class Equipment.BodyScannerType
local BodyScannerType = EquipTypes.BodyScannerType

local format_px = function(v) return string.format("%s px", ui.Format.Number(v, 0)) end

function BodyScannerType:GetDetailedStats()
	local out = self:Super().GetDetailedStats(self)

	table.insert(out, {
		le.SENSOR_RESOLUTION,
		icons.scanner,
		self.stats.resolution,
		format_px
	})

	table.insert(out, {
		le.SENSOR_MIN_ALTITUDE,
		icons.altitude,
		self.stats.minAltitude,
		ui.Format.Distance,
		true -- lower is better
	})

	return out
end

--==============================================================================

---@class Equipment.CabinType
local CabinType = EquipTypes.CabinType

function CabinType:GetDetailedStats()
	local out = self:Super().GetDetailedStats(self)

	table.insert(out, {
		le.OCCUPIED_BERTHS,
		icons.personal,
		self:GetNumPassengers(),
		tostring
	})

	table.insert(out, {
		le.PASSENGER_BERTHS,
		icons.personal,
		self.capabilities.cabin,
		tostring
	})

	return out
end

---@return table[]
function CabinType:GetItemCardStats()
	return {
		{ icons.personal, "{}/{}" % { self:GetNumPassengers(), self:GetMaxPassengers() } },
		{ icons.hull, format_mass(self.mass) },
		{ icons.ecm, format_power(0) },
		{ icons.repairs, format_integrity(1) }
	}
end

--==============================================================================

---@class Equipment.ThrusterType
local ThrusterType = EquipTypes.ThrusterType

function ThrusterType:GetDetailedStats()
	local stats = self:Super().GetDetailedStats(self)

	return table.append(stats, {
		{ le.STAT_THRUSTER_POWER, icons.equip_thrusters, self.capabilities.thruster_power, format_multiplier }
	})
end

--==============================================================================

---@class Equipment.ShieldType
local ShieldType = EquipTypes.ShieldType

function ShieldType:GetDetailedStats()
	local stats = self:Super().GetDetailedStats(self)

	return table.append(stats, {
		-- HACK: 10 is the value of TONS_HULL_PER_SHIELD in Ship.cpp
		{ le.STAT_SHIELD, icons.equip_shield_generator, self.capabilities.shield * 10, format_mass }
	})
end
