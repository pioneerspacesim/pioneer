-- Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- This file implements type information about C++ classes for Lua static analysis
-- TODO: this file is partially type-complete, please expand it as more types are added.

---@meta

---@class Engine
---
---@field rand Rand
---@field ticks number Number of milliseconds since Pioneer was started.
---@field time number Number of real-time seconds since Pioneer was started.
---@field frameTime number Length of the last frame in seconds.
---@field pigui unknown This is purposefully left untyped, type information is provided for the ui object
---@field version string

---@class Engine
local Engine = {}

-- TODO: add information about Engine methods

-- Get a model file by name
---@param name string
---@return SceneGraph.Model model
function Engine.GetModel(name) end

-- Get a boolean value from the settings file.
---@param key string
---@return boolean
function Engine.SettingsGetBool(key) end

-- Get an integer value from the settings file.
---@param key string
---@return integer
function Engine.SettingsGetInt(key) end

-- Get a floating-point number value from the settings file.
---@param key string
---@return number
function Engine.SettingsGetNumber(key) end

-- Get a string value from the settings file.
---@param key string
---@return string
function Engine.SettingsGetString(key) end

-- Set a boolean value to the settings file.
---@param key string
---@param value boolean
function Engine.SettingsSetBool(key, value) end

-- Set an integer value to the settings file.
---@param key string
---@param value integer
function Engine.SettingsSetInt(key, value) end

-- Set a number value to the settings file.
---@param key string
---@param value number
function Engine.SettingsSetNumber(key, value) end

-- Set a string value to the settings file.
---@param key string
---@param value string
function Engine.SettingsSetString(key, value) end

-- Write any pending settings changes to disk.
function Engine.SaveSettings() end

return Engine
