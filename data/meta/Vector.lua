-- Copyright © 2008-2022 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- This file implements type information about C++ classes for Lua static analysis

---@meta

---@class Vector2
---@field x number
---@field y number
local Vector2 = {}

---@param x number
---@param y number
---@return Vector2
---@overload fun(x: number): Vector2
function _G.Vector2(x, y) end

---@return Vector2
function Vector2:normalized() end

---@return Vector2
function Vector2:normalised() end

---@return number
function Vector2:lengthSqr() end

---@return number
function Vector2:length() end

---@param angle number
---@return Vector2
function Vector2:rotate(angle) end

---@return number
function Vector2:angle() end

--- Rotate the vector by 90 degrees left
---@return Vector2
function Vector2:left() end

--- Rotate the vector by 90 degrees right
---@return Vector2
function Vector2:right() end

-- ============================================================================

---@class Vector3
---@field x number
---@field y number
---@field z number
local Vector3 = {}

---@param x number
---@return Vector3
---@overload fun(x: number, y: number, z: number): Vector3
---@overload fun(vec2: Vector2, z: number?): Vector3
function _G.Vector3(x) end

---@return Vector3
function Vector3:normalized() end

---@return Vector3
function Vector3:normalised() end

---@return number
function Vector3:lengthSqr() end

---@return number
function Vector3:length() end

---@param vec Vector3
---@return Vector3
function Vector3:cross(vec) end

---@param vec Vector3
---@return number
function Vector3:dot(vec) end
