-- Copyright © 2008-2022 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- This file implements type information about Pioneer's customization of the
-- Lua global execution environment.

---@meta

--- Dump all relevant variables and parameters for the current lua stack trace.
---@param level number stack level to start the stack dump at
debug.dumpstack = function(level) end

--- Convert the angle `a` from degrees to radians
---@param a number
---@return number
math.deg2rad = function(a) end

--- Convert the angle `a` from radians to degrees
---@param a number
---@return number
math.rad2deg = function(a) end

--- CoreImports registry for directly accessing C++ class metatypes without
--- using the `require()` mechanism (e.g. to allow extending metatypes)
package.core = {}

--- Trigger a purge and reimport of the specified package.
--- References to the return value of the old package will not be replaced.
---@param name string? dot-qualified name of the package to trigger reimport for
package.reimport = function(name) end

--- Log the specified string at the Warning semantic level
function logWarning(string) end

--- Log the specified string at the Verbose semantic level
function logVerbose(string) end

-- ============================================================================

-- Document lua Constants as typed subclasses of string for API visibility sake

-- TODO: not all constant types are present here; please update this file as new
-- constant types are required

-- A <Constants.BodyType> string
---@class BodyType: string

-- A <Constants.BodySuperType> string
---@class BodySuperType: string

-- A <Constants.PhysicsObjectType> string
---@class PhysicsObjectType: string

--- PolitGovType string constant; EARTHCOLONIAL, EARTHDEMOC, EMPIRERULE, etc.
---@class PolitGovType: string

-- A <Constants.ShipAlertStatus> string
---@class ShipAlertStatus: string

-- A <Constants.ShipFlightState> string
---@class ShipFlightState: string

-- ============================================================================

-- Global Constants namespace
Constants = {}

---@type BodyType[]
Constants.BodyType = {}

---@type BodySuperType[]
Constants.BodySuperType = {}

---@type PhysicsObjectType[]
Constants.PhysicsObjectType = {}

---@type PolitGovType[]
Constants.PolitGovType = {}

---@type ShipAlertStatus[]
Constants.ShipAlertStatus = {}

---@type ShipFlightState[]
Constants.ShipFlightState = {}
