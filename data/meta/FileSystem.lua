-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- This file implements type information about C++ classes for Lua static analysis
-- This is used in FileSyestem.lua whcih then extends that.

---@meta

---@class FileSystem
local FileSystem = {}

---@param path string The directory to read the contents of.
---@return string[] files  A list of files as full paths from the root
---@return string[] dirs   A list of dirs as full paths from the root
---
--- Example:
---  > local files, dirs = FileSystem.ReadDirectory("user://savefiles")
function FileSystem.ReadDirectory(path) end

--- Join the passed arguments into a path, correctly handling separators and .
--- and .. special dirs.
---
---@param arg string[]  A list of path elements to be joined
---@return string       The joined path elements
function FileSystem.JoinPath( ... ) end

---@param dir_name string The name of the folder to create
---@return boolean Success
function FileSystem.MakeDirectory( dir_name ) end

--- Wrapper for our patched io.open that ensures files are opened inside the sandbox.
--- Prefer using this to io.open
---
--- Files in the user folder can be read or written to
--- Files in the data folder are read only.
---
---
---
--- Example:
--- > f = FileSystem.Open( "user://my_file.txt", "w" )
--- > f:write( "file contents" )
--- > f:close()
--- 
---@param filename string   The name of the file to open, must start either user:// or data://
---@param mode string|nil   The mode to open the file in, defaults to read only. Only user location files can be written
---@return file             A lua io file
function FileSystem.Open( filename ) end

return FileSystem
