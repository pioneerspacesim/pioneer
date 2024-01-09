-- Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- This file implements type information about C++ classes for Lua static analysis

---@meta

local Space = {}

package.core["Space"] = Space

-- Returns a list of bodies simulated in the current system, optionally filtered by type
---@generic T
---@param class `T` an optional body classname to filter the returned results by
---@return T[]
---@overload fun(): Body[]
function Space.GetBodies(class) end

-- Returns a list of bodies at most dist away from the given body, optionally filtered by type
---@generic T
---@param body Body the reference body
---@param dist number the maximum distance from the reference body to search
---@param filter `T` an optional body classname to filter the returned results by
---@return T[]
---@overload fun(body: Body, dist: number): Body[]
function Space.GetBodiesNear(body, dist, filter) end

return Space
