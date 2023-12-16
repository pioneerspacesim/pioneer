-- Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local EquipTypes = require 'EquipType'

local Lang = require 'Lang'

local utils = require 'utils'

local lc = Lang.GetResource("core")
local le = Lang.GetResource("equipment-core")

local ui = require 'pigui'
local icons = ui.theme.icons

local compare = function(n)
	if n > 0 then
		return ui.theme.styleColors.success_300
	elseif n == 0 then
		return ui.theme.colors.font
	else
		return ui.theme.styleColors.warning_300
	end
end

--==============================================================================

---@class EquipType
local EquipType = EquipTypes.EquipType

---@param other EquipType?
---@return table[]
function EquipType:GetDetailedStats(other)
	other = other or self

	local out = {}

	local equipHealth = 1
	table.insert(out, {
		le.EQUIPMENT_INTEGRITY, icons.repairs, string.format("%d%%", equipHealth * 100),
		compare(equipHealth - equipHealth)
	})

	table.insert(out, {
		le.STAT_VOLUME, icons.square, string.format("%.1f%s³", self.volume, lc.UNIT_METERS),
		compare(other.volume - self.volume)
	})

	table.insert(out, {
		le.STAT_WEIGHT, icons.hull, ui.Format.Mass(self.mass * 1000, 1),
		compare(other.mass - self.mass)
	})

	return out
end

---@return table[]
function EquipType:GetItemCardStats()
	return {
		{ icons.square, string.format("%.1f%s³", self.volume, lc.UNIT_METERS) },
		{ icons.hull, ui.Format.Mass(self.mass * 1000, 1) },
		{ icons.repairs, string.format("%d%%", 100) }
	}
end

--==============================================================================

local LaserType = EquipTypes.LaserType

function LaserType:GetDetailedStats(other)
	local out = self:Super().GetDetailedStats(self)
	other = other or self

	table.insert(out, {
		le.SHOTS_PER_MINUTE,
		icons.comms, -- PLACEHOLDER
		string.format("%d RPM", 60 / self.laser_stats.rechargeTime),
		compare(other.laser_stats.rechargeTime - self.laser_stats.rechargeTime)
	})

	table.insert(out, {
		le.DAMAGE_PER_SHOT,
		icons.ecm_advanced,
		string.format("%.1f KW", self.laser_stats.damage),
		compare(self.laser_stats.damage - other.laser_stats.damage)
	})

	table.insert(out, {
		le.PROJECTILE_SPEED,
		icons.forward,
		string.format("%.1f%s", self.laser_stats.speed, lc.UNIT_METERS_PER_SECOND),
		compare(self.laser_stats.speed - other.laser_stats.speed)
	})

	return out
end

--==============================================================================

local BodyScannerType = EquipTypes.BodyScannerType

function BodyScannerType:GetDetailedStats(other)
	local out = self:Super().GetDetailedStats(self)
	other = other or self

	table.insert(out, {
		le.SENSOR_RESOLUTION,
		icons.scanner,
		string.format("%s px", ui.Format.Number(self.stats.resolution, 0)),
		compare(self.stats.resolution - other.stats.resolution)
	})

	table.insert(out, {
		le.SENSOR_MIN_ALTITUDE,
		icons.altitude,
		ui.Format.Distance(self.stats.minAltitude),
		compare(other.stats.minAltitude - self.stats.minAltitude)
	})

	return out
end

--==============================================================================
