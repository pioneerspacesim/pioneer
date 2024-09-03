-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- This file implements type information about C++ classes for Lua static analysis

---@meta

---@class StarSystem
---
---@field name string
---@field other_names string[]
---@field path SystemPath
---
--- The lawlessness value for the system, 0 for peaceful, 1 for raging hordes of pirates
---@field lawlessness number
--- The population of this system, in billions of people
---@field population number
--- The <Faction> that controls this system
---@field faction unknown #TODO: add type information for LuaFaction
--- The government type used in the system
---@field govtype PolitGovType
--- Has this system been explored before?
---@field explored boolean
---
---@field numberOfStars integer
---@field numberOfStations integer
---@field numberOfBodies integer
---@field rootSystemBody SystemBody
---
--- Translated description for the system
---@field shortDescription string
--- Translated description of the system's government type
---@field govDescription string
--- Translated description of the system's economic type
---@field econDescription string

---@class StarSystem
local StarSystem = {}

---@return SystemPath[]
function StarSystem:GetStationPaths() end

---@return SystemPath[]
function StarSystem:GetBodyPaths() end

---@return SystemBody[]
function StarSystem:GetStars() end

---@return boolean[]
function StarSystem:GetJumpable() end

---@param name string Commodity identifier
---@return number
function StarSystem:GetCommodityBasePriceAlterations(name) end

---@param name string Commodity identifier
---@return boolean
function StarSystem:IsCommodityLegal(name) end

---@param range number distance from this system to search, in light years
---@param filter fun(ss: StarSystem): boolean predicate to filter returned starsystems
---@return StarSystem[]
function StarSystem:GetNearbySystems(range, filter) end

--- Calculate the distance between this and another system
---@param other StarSystem|SystemPath
---@return number dist distance between the two systems in light years
function StarSystem:DistanceTo(other) end

-- Export a generated star system to a Lua file for further customization
function StarSystem:ExportToLua() end

--- Set the star system to be explored by the player
---@param time number? optional time at which the system was explored
function StarSystem:Explore(time) end
