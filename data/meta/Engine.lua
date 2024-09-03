-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
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

return Engine
