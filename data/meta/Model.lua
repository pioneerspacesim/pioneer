-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- This file implements type information about C++ classes for Lua static analysis

---@meta

---@class SceneGraph.Model
---
---@field name string
---@field pattern integer Current pattern applied to the model
---@field numPatterns integer Number of patterns supported by the model

---@class SceneGraph.Model
local Model = {}

-- Set the pattern currently applied to the model
---@param idx integer
function Model:SetPattern(idx) end

-- Set debug flags used when rendering this model
---@param flags ModelDebugFlags[]
function Model:SetDebugFlags(flags) end
