-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- This file implements type information about C++ classes for Lua static analysis

---@meta

---@class Vector2
---@field x number
---@field y number
---
---@operator add: Vector2
---@operator add(number): Vector2
---@operator sub: Vector2
---@operator sub(number): Vector2
---@operator mul: Vector2
---@operator mul(number): Vector2
---@operator div: Vector2
---@operator div(number): Vector2
---@operator unm: Vector2
---@operator call: Vector2

---@class Vector2
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
---
---@operator add: Vector3
---@operator add(number): Vector3
---@operator sub: Vector3
---@operator sub(number): Vector3
---@operator mul: Vector3
---@operator mul(number): Vector3
---@operator div: Vector3
---@operator div(number): Vector3
---@operator unm: Vector3
---@operator call: Vector3

---@class Vector3
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
