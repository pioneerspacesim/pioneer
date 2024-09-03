-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- This file implements type information about C++ classes for Lua static analysis

---@meta

---@class SystemPath
---
---@field sectorX integer
---@field sectorY integer
---@field sectorZ integer
---@field systemIndex integer?
---@field bodyIndex integer?

---@class SystemPath
local SystemPath = {}

---@param x integer
---@param y integer
---@param z integer
---@return SystemPath
---@overload fun(x: integer, y: integer, z: integer, si: integer, bi: integer): SystemPath
function SystemPath.New(x, y, z) end

---@param str string
---@return SystemPath?
function SystemPath.ParseString(str) end

--- Determine if two <SystemPath> objects point to objects in the same system.
---@param other SystemPath
---@return boolean
function SystemPath:IsSameSystem(other) end

--- Determine if two <SystemPath> objects point to objects in the same sector.
---@param other SystemPath
---@return boolean
function SystemPath:IsSameSector(other) end

--- Derive a SystemPath that points to the whole system.
---@return SystemPath
function SystemPath:SystemOnly() end

--- Derive a SystemPath that points to the whole sector.
---@return SystemPath
function SystemPath:SectorOnly() end

--- Calculate the distance between this and another system
---@param system SystemPath|StarSystem --not yet implemented
---@return number
function SystemPath:DistanceTo(system) end

---@return StarSystem
function SystemPath:GetStarSystem() end

---@return SystemBody
function SystemPath:GetSystemBody() end

---@return boolean
function SystemPath:IsSystemPath() end

---@return boolean
function SystemPath:IsBodyPath() end

return SystemPath
