-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- This file implements type information about C++ modules for Lua static analysis

---@meta

---@class ShipDef
---@field id string
---@field name string
---@field shipClass string
---@field manufacturer string
---@field modelName string
---@field cockpitName string
---@field tag ShipTypeTag
---@field roles string[]
--
---@field angularThrust number
---@field linearThrust table<ShipTypeThruster, number>
---@field linAccelerationCap table<ShipTypeThruster, number>
---@field effectiveExhaustVelocity number
---@field thrusterFuelUse number -- deprecated
---@field frontCrossSec number
---@field sideCrossSec number
---@field topCrossSec number
---@field atmosphericPressureLimit number
--
---@field capacity number
---@field cargo integer
---@field hullMass number
---@field fuelTankMass number
---@field basePrice number
---@field minCrew integer
---@field maxCrew integer
---@field hyperdriveClass integer
---@field equipSlotCapacity table -- deprecated
---
---@field raw table The entire ShipDef JSON object as a Lua table

---@type table<string, ShipDef>
local ShipDef = {}

return ShipDef
