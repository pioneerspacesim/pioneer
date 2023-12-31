-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- This file implements type information about C++ classes for Lua static analysis

---@meta

---
---@class Player : Ship
---

---@class Player : Ship
local Player = {}

-- Ensure the CoreImport field is visible to static analysis
package.core["Player"] = Player

-- TODO: document methods as required

return Player
