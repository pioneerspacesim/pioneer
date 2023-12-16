-- Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local EquipTypes = require 'EquipType'

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

---@return EquipType.UI.Stats[]
function EquipType:GetDetailedStats()
	local equipHealth = 1
	local powerDraw = 0

	return {
		{ le.EQUIPMENT_INTEGRITY, icons.repairs, equipHealth, format_integrity },
		{ le.STAT_VOLUME, icons.square, self.volume, ui.Format.Volume, true },
		{ le.STAT_WEIGHT, icons.hull, self.mass, format_mass, true },
		{ le.STAT_POWER_DRAW, icons.ecm, powerDraw, format_power, true }
	}
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

local LaserType = EquipTypes.LaserType

local format_rpm = function(v) return string.format("%d RPM", 60 / v) end
local format_speed = function(v) return string.format("%.1f%s", v, lc.UNIT_METERS_PER_SECOND) end

function LaserType:GetDetailedStats()
	local out = self:Super().GetDetailedStats(self)

	table.insert(out, {
		le.SHOTS_PER_MINUTE,
		icons.comms, -- PLACEHOLDER
		self.laser_stats.rechargeTime,
		format_rpm,
		true -- lower is better
	})

	table.insert(out, {
		le.DAMAGE_PER_SHOT,
		icons.ecm_advanced,
		self.laser_stats.damage,
		format_power
	})

	table.insert(out, {
		le.PROJECTILE_SPEED,
		icons.forward,
		self.laser_stats.speed,
		format_speed
	})

	return out
end

--==============================================================================

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
