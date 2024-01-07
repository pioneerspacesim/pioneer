-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- This file implements type information about Pioneer's customization of the
-- Lua global execution environment.

---@meta

-- Load in auto-generated constants
require 'Constants.lua'

--- Dump all relevant variables and parameters for the current lua stack trace.
---@param level number stack level to start the stack dump at
debug.dumpstack = function(level) end

--- Convert the angle `a` from degrees to radians
---@param a number
---@return number
math.deg2rad = function(a) end

--- Convert the angle `a` from radians to degrees
---@param a number
---@return number
math.rad2deg = function(a) end

--- CoreImports registry for directly accessing C++ class metatypes without
--- using the `require()` mechanism (e.g. to allow extending metatypes)
package.core = {}

--- Trigger a purge and reimport of the specified package.
--- References to the return value of the old package will not be replaced.
---@param name string? dot-qualified name of the package to trigger reimport for
package.reimport = function(name) end

--- Get the module name of the file where this function is called
---@return string moduleName
package.thisModule = function() end

--- Get the module name of the function N stack levels above package.modulename
--- in the execution stack. Note: tail calls may affect the accuracy of this
--- function.
---@param index integer number of stack levels above package.modulename(), minimum 1
---@return string moduleName
package.modulename = function(index) end

--- Log the specified string at the Warning semantic level
function logWarning(string) end

--- Log the specified string at the Verbose semantic level
function logVerbose(string) end
