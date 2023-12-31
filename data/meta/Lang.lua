-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- This file implements type information about C++ classes for Lua static analysis

---@meta

local Lang = {}

--- Helper typedef to provide completion for the :get() method
---@type table<string, string>
local Resource = {}

---@param key string
---@return string
function Resource:get(key) end

--- Return a table of strings for the given language resource, optionally
--- in the specified language.
---@param name string
---@param langCode string?
function Lang.GetResource(name, langCode) return Resource end

return Lang
