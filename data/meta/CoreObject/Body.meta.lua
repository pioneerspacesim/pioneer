-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- This file implements type information about C++ classes for Lua static analysis

---@meta

---
---@class Body
---
---@field label string
---@field seed integer
---
--- The <SystemPath> that points to this body.
--- Only available for non-dynamic bodies.
---@field path? SystemPath
---
--- The type of the body, as a <Constants.BodyType> constant.
---@field type? BodyType
---
--- The supertype of the body, as a <Constants.BodySuperType> constant
---@field superType? BodySuperType
---
--- The non-dynamic body attached to the frame this dynamic body is in.
--- Only available for dynamic bodies.
---@field frameBody? Body
---
--- Whether the frame this body is in is a rotating frame.
--- Only available for dynamic bodies.
---@field frameRotating? boolean
---

---@class Body
local Body = {}

-- Ensure the CoreImport field is visible to static analysis
package.core["Body"] = Body

-- Check if this body reference is still valid
---@return boolean
function Body:exists() end

-- Check if this object is a specific type (e.g. Ship or SpaceStation)
---@param type string
---@return boolean
function Body:isa(type) end

-- Set the given body property to the passed value
---@param key string
---@param value any NOTE: functions, tables, and coroutines cannot be set as body properties
function Body:setprop(key, value) end

-- Check if the given property exists on the body
---@param key string
---@return boolean
function Body:hasprop(key) end

--- Get a C++ or Lua component object from the body if present.
---
--- Note: the caller should check the return value if there is a possibility if
--- the component may not be present on this Body.
---@generic T
---@param name `T`
---@return T
function Body:GetComponent(name) end

--- Set or clear the given Lua component on the body to the passed object.
---@generic T
---@param name `T`
---@param value T|nil
function Body:SetComponent(name, value) end

--- Determine if the body is a dynamic body.
---@return boolean
function Body:IsDynamic() end

--- Calculate the distance between two bodies
---@param otherBody Body
---@return number
function Body:DistanceTo(otherBody) end

--- Get the body's position relative to its parent frame.
---
--- If the parent is a TerrainBody, altitude will be the height above terrain or sea level in meters.
---@param boolean? terrainRelative
---@return number latitude the latitude of the body in radians
---@return number longitude the longitude of the body in radians
---@return number? altitude altitude above the ground or sea level in meters
function Body:GetGroundPosition(terrainRelative) end

--- Find the nearest object of a <Constants.PhysicsObjectType> type
---@param type PhysicsObjectType
---@return Body nearest
function Body:FindNearestTo(type) end

--- Get the body's velocity relative to another body as a Vector3
---@param other Body
---@return Vector3 velocity
function Body:GetVelocityRelTo(other) end

--- Get the body's position relative to another body as a Vector3
---@param other Body
---@return Vector3 position
function Body:GetPositionRelTo(other) end

--- Get the body's altitude relative to another body.
---
--- Returns height above terrain or sea level if the other body is a TerrainBody or
--- distance between bodies otherwise.
---@param other Body
---@param boolean? terrainRelative
---@return number altitude
function Body:GetAltitudeRelTo(other, terrainRelative) end

---@return number
function Body:GetPhysicalRadius() end

--- Return the atmospheric state of the given body in this body's atmosphere
---@param forBody Body
---@return number? pressure
---@return number? density
function Body:GetAtmosphericState(forBody) end

---@return string label
function Body:GetLabel() end

---@param other Body
---@return boolean
function Body:IsMoreImportantThan(other) end

---@return boolean
function Body:IsMoon() end

---@return boolean
function Body:IsPlanet() end

---@return boolean
function Body:IsShip() end

---@return boolean
function Body:IsHyperspaceCloud() end

---@return boolean
function Body:IsMissile() end

---@return boolean
function Body:IsStation() end

---@return boolean
function Body:IsSpaceStation() end

---@return boolean
function Body:IsGroundStation() end

---@return boolean
function Body:IsCargoContainer() end

---@return SystemBody?
function Body:GetSystemBody() end

---@return Vector3
function Body:GetVelocity() end

---@param vel Vector3
function Body:SetVelocity(vel) end

---@param angVel Vector3
function Body:SetAngVelocity(angVel) end
