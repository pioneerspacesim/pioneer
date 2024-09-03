-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- This file implements type information about C++ classes for Lua static analysis

---@meta

---
---@class Ship : ModelBody
---
--- Whether the frame this body is in is a rotating frame.
---@field frameRotating boolean
---
---@field shipId string
---@field shipName string
---@field equipSet EquipSet
---
---@field flightState ShipFlightState
---@field alertStatus ShipAlertStatus
---
---@field hullMassLeft number
---@field hullPercent number
---
---@field shieldMass number
---@field shieldMassLeft number
---
--- Remaining fuel as a number from 1..100
---@field fuel number
--- Remaining fuel mass in tons
---@field fuelMassLeft number
---
---@field usedCapacity number
---@field freeCapacity number
---
---@field usedCargo number
---@field totalCargo number
---
---@field staticMass number
---@field totalMass number
---
---@field hyperspaceRange number
---@field maxHyperspaceRange number
---

---@class Ship
local Ship = {}

-- Ensure the CoreImport field is visible to static analysis
package.core["Ship"] = Ship

---@return boolean
function Ship:IsPlayer() end

-- Returns a string describing the ship class
---@return string
function Ship:GetShipClass() end

-- Returns a string describing the ship type
---@return string
function Ship:GetShipType() end

-- Replaces the ship with a new ship of the specified type.
---@param type string
function Ship:SetShipType(type) end

-- Changes the ship's label text.
-- This is the text that appears beside the ship in the HUD.
---@param label string
function Ship:SetLabel(label) end

-- Changes the ship's name text.
---@param name string
function Ship:SetShipName(name) end

-- Get the current skin object of the ship.
---@return SceneGraph.ModelSkin
function Ship:GetSkin() end

-- Set the skin of the ship.
---@param skin SceneGraph.ModelSkin
function Ship:SetSkin(skin) end

-- Changes the pattern used for texturing the ship.
---@param pattern integer
function Ship:SetPattern(pattern) end

-- Return the current hyperspace destination.
---@return SystemPath path the destination system and body
---@return string name the name of the destination
function Ship:GetHyperspaceDestination() end

-- TODO: document further methods as they are used

-- Return the current hull temperature (0.0 - 1.0).
---@return number
function Ship:GetHullTemperature() end

-- Get a gun's temperature (0.0 - 1.0).
---@param gun integer
---@return number
function Ship:GetGunTemperature(gun) end

-- Return the remaining hull mass as a percent in the range 0..100
---@return number
function Ship:GetHullPercent() end

-- Sets the hull mass of the ship to the given percentage of its maximum.
---@param percent number
function Ship:SetHullPercent(percent) end

-- Return the remaining shield capacity as a percent in the range 0..100
---@return number
function Ship:GetShieldsPercent() end

-- Sets the thruster fuel tank of the ship to the given percentage of its maximum.
---@param percent number
function Ship:SetFuelPercent(percent) end
