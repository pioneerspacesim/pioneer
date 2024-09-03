-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- This file implements type information about C++ classes for Lua static analysis

---@meta

---
---@class ModelBody : Body
---
---@field model SceneGraph.Model
---

---@class ModelBody
local ModelBody = {}

-- Ensure the CoreImport field is visible to static analysis
package.core["ModelBody"] = ModelBody
