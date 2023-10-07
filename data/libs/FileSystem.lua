---@class FileSystem : FileSystemBase
local FileSystem = package.core["FileSystem"]

--- Wrapper for our patched io.open that ensures files are opened inside the sandbox.
--- Prefer using this to io.open
---
--- Files in the user folder can be read or written to
--- Files in the data folder are read only.
---
---
---
--- Example:
--- > f = FileSystem.Open( "USER", "my_file.txt", "w" )
--- > f:write( "file contents" )
--- > f:close()
--- 
---@param root string       A FileSystemRoot constant. Can be either "DATA" or "USER"
---@param filename string   The name of the file to open, relative to the root
---@param mode string|nil   The mode to open the file in, defaults to read only
---@return file             A lua io file
function FileSystem.Open( root, filename, mode )
    if not mode then mode = "r" end

    return io.open( filename, mode, root )
end

return FileSystem
