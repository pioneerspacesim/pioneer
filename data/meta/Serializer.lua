-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- This file implements type information about C++ modules for Lua static analysis

---@meta

---@class Serializer
local Serializer = {}

-- Registers a function pair to serialize per-module data across savegames.
--
-- The serializer function can only serialize a single table object, but may
-- store any serializable objects inside that table.
---@param key string
---@param serialize fun(): table
---@param unserialize fun(t: table)
function Serializer:Register(key, serialize, unserialize) end

-- Registers a class type for serialization across savegames.
--
-- Objects of the passed class will be serialized via the Serialize / Unserialize
-- class methods if referred to by other serialized data.
---@param key string
---@param class table
function Serializer:RegisterClass(key, class) end

-- Register a table as a "persistent" value. All references to the saved
-- instance of that table will be transparently replaced across savegames to
-- maintain table instance identity.
---@param key string
---@param value table
function Serializer:RegisterPersistent(key, value) end

return Serializer
