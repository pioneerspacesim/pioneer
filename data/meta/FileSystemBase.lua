-- Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- This file implements type information about C++ classes for Lua static analysis
-- This is used in FileSyestem.lua whcih then extends that.

---@meta

---@class FileSystemBase
local FileSystemBase = {}

---@param root string A FileSystemRoot constant. Can be either "DATA" or "USER"
---@return string[] files  A list of files as full paths from the root
---@return string[] dirs   A list of dirs as full paths from the root
---
--- Example:
---  > local files, dirs = FileSystem.ReadDirectory(root, path)
function FileSystemBase.ReadDirectory(root, path) end

--- Join the passed arguments into a path, correctly handling separators and .
--- and .. special dirs.
---
---@param arg string[]  A list of path elements to be joined
---@return string       The joined path elements
function FileSystemBase.JoinPath( ... ) end

---@param dir_name string The name of the folder to create in the user directory
---@return boolean Success
function FileSystemBase.MakeUserDataDirectory( dir_name ) end

return FileSystemBase
