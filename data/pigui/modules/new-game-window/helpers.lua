-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txtl

local lui = require 'Lang'.GetResource("ui-core")

local Helpers = {}

-- Given a number (version), select the most suitable function from the array
-- and run it with the arguments passed. The smallest version is selected, no
-- less than given. Or the largest if all versions in the array are less than
-- given. Functions are assumed to be sorted in ascending order of versions
---@param version number
---@param fncList table array of tables { version: number, fnc: function }
---@param ... any
---@return any ...
local function runVersioned(version, fncList, ...)
	local i = #fncList
	while i > 1 and fncList[i].version > version do
		i = i - 1
	end
	return fncList[i].fnc(...)
end


--
-- Function: Helpers.versioned
--
-- Creates a versioned function. When called, depending on
-- Helpers.implicitVersion, the appropriate version is launched.
--
-- Example:
--
-- > Helpers.isPickled = Helpers.versioned {{
-- > 	version = 89,
-- > 	fnc = function(tbl)
-- > 		return tbl and tbl.table
-- > 	end
-- > }}
--
-- Parameters:
--
-- fncList - table, array of tables { version: number, fnc: function }
--
-- Returns:
--
-- value - function
--
---@param fncList table array of tables { version: number, fnc: function }
---@return function value
function Helpers.versioned(fncList)
	return function(...)
		assert(Helpers.implicitVersion)
		return runVersioned(Helpers.implicitVersion, fncList, ...)
	end
end


--
-- Function: Helpers.getByPath
--
-- Looks for an element in the table, sequentially obtaining indices from the
-- provided string and checking for their presence.
--
-- If the key looks like $N, return the N-th value from pairs(table). In fact,
-- it is used as $1 when there is only one element in the table, but the key
-- there is a complex table.
-- If the key starts with #, it is read as a number
--
-- Example:
--
-- > myTable = { section = { subsection = { key1 = 'value1', key2 = 'value2' }}}
-- > value, errorString = Helpers.getByPath(myTable, 'section/subsection/key2')
-- > assert(errorString == nil)
-- > assert(value == 'value2')
--
-- Parameters:
--
--   tbl - table
--   path - string|table, see example
--   verify - function?
--
-- Returns:
--
--  value - any?
--  errorString - string?
--
---@param tbl table
---@param path string|table see example
---@param verify function?
---@return any? value
---@return string? errorString
function Helpers.getByPath(tbl, path, verify)
	assert(Helpers.implicitVersion)
	-- list of paths - return on first successful
	if type(path) == 'table' then
		local value
		local wholeErrorString = ""
		local sep = ""
		for _, item in ipairs(path) do
			local errorString
			value, errorString = Helpers.getByPath(tbl, item, verify)
			if value then return value end
			wholeErrorString = wholeErrorString .. sep .. errorString
			sep = ",\n    "
		end
		return nil, wholeErrorString
	end

	local currentPath = ""
	local sep = ""
	for token in string.gmatch(path, "[^/]+") do
		currentPath = currentPath .. sep .. token
		sep = '/'
		if token:sub(1, 1) == '$' then
			local i = tonumber(token:sub(2, -1))
			for _, v in pairs(tbl) do
				if i == 1 then
					tbl = v
					break
				end
				i = i - 1
			end
			if i ~= 1 then return nil, string.format(lui.CANT_FIND_PATH, currentPath) end
		elseif token:sub(1, 1) == '#' then
			tbl = tbl[tonumber(token:sub(2, -1))]
		else
			tbl = tbl[token]
		end
		if not tbl then return nil, string.format(lui.CANT_FIND_PATH, currentPath) end
	end
	if verify and not verify(tbl) then
		return nil, string.format(lui.PATH_FOUND_BUT_VERIFICATION_FAILED, path)
	end
	return tbl
end


---@param saveGame table
---@return table?
---@return string? errorString
local function findPlayerShip(saveGame)
	local bodies, errorString = Helpers.getByPath(saveGame, "space/bodies")
	local playerShip
	local bodyTypePlayer = saveGame.version > 86 and 4 or 5
	if errorString then return nil, errorString end
	for _, body in ipairs(bodies) do
		if body.body_type == bodyTypePlayer then
			if playerShip then
				return nil, lui.MORE_THAN_ONE_PLAYERS_SHIP_FOUND_IN_SPACE
			else
				playerShip = body
			end
		end
	end
	if playerShip then return playerShip end
	return nil, lui.CANT_FIND_PLAYERS_SHIP_IN_SPACE
end


--
-- Function: Helpers.getPlayerShipParameter
--
-- Finds the player's ship in the saved space and pulls out a parameter from it
-- according to the specified path.
--
-- Example:
--
-- > local shipID, errorString = Helpers.getPlayerShipParameter(saveGame, "model_body/model_name")
--
-- Parameters:
--
--   saveGame - table
--   paramPath - string
--
-- Returns:
--
--   value - any?
--   errorString - string?, if it is not nil, an error occurred
--
---@param saveGame table
---@param paramPath string
---@return any? value
---@return string? errorString if it is not nil, an error occurred
function Helpers.getPlayerShipParameter(saveGame, paramPath)
	local playerShip, errorString = findPlayerShip(saveGame)
	if errorString then return nil, errorString end
	assert(playerShip)
	local param
	param, errorString = Helpers.getByPath(playerShip, paramPath)
	if errorString then return nil, errorString end
	return param
end

--
-- Function: Helpers.getLuaClass
--
-- Retrieves the value of the 'class' field from the metatable. It should have
-- been there when the save was unpickled.
--
-- Example:
--
-- > local entryType = Helpers.getLuaClass(entry)
--
-- Parameters:
--
--   tbl - table
--
-- Returns:
--
--   typename - string?
--
---@param tbl table
---@return string? typename
function Helpers.getLuaClass(tbl)
	local typeTable = getmetatable(tbl)
	if typeTable then return typeTable.class end
end

return Helpers
