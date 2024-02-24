-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Json = require 'Json'
local Lang = require 'Lang'
local lui = Lang.GetResource("ui-core")
local Helpers = require 'pigui.modules.new-game-window.helpers'

local Recovery = {}

-- patch
--    from - number (version)
--    to - number (version)
--    apply(gameDoc) -> errorString? - function that modifies whole gameDoc
--
--    it is assumed that the patches are arranged in the array in ascending
--    order of versions, and do not intersect
Recovery.Patches = {}

-- no patches yes

-- Fix up all contained table references
--
-- The order and location a table value definition is serialized in is
-- non-deterministic and any JSON value encoding a reference to a specific
-- table value may potentially be the one in which that table is serialized to.
local fixupDoc = Helpers.versioned {{
	version = 89,
	fnc = function(gameDoc)

		local refs = {}

		local typeTables = {}

		local function unpickle(tbl)
			local result = {}
			tbl = tbl.table
			for i = 1, #tbl, 2 do
				result[tbl[i]] = tbl[i + 1]
			end
			return result
		end

		-- get refs and unpickle 1 level
		local function getRefs(tbl)
			if type(tbl) ~= 'table' then return end
			if tbl.ref and tbl.table and not refs[tbl.ref] then
				refs[tbl.ref] = unpickle(tbl)
			end
			for k, v in pairs(tbl) do
				getRefs(k)
				getRefs(v)
			end
		end

		local function putRefs(tbl, cache)
			if type(tbl) ~= 'table' then return tbl end

			if cache[tbl] then return cache[tbl] end
			if cache[tbl.ref] then return cache[tbl.ref] end

			local result = {}
			if tbl.ref then
				if tbl.lua_class then
					local typeTable = typeTables[tbl.lua_class]
					if not typeTable then
						typeTable = { class = tbl.lua_class }
						typeTables[tbl.lua_class] = typeTable
					end
					setmetatable(result, typeTable)
				end
				cache[tbl.ref] = result
				tbl = refs[tbl.ref]
			else
				cache[tbl] = result
			end

			for k, v in pairs(tbl) do
				result[ putRefs(k, cache) ] = putRefs(v, cache)
			end
			return result
		end

		-- circular references are possible after putRefs
		local function countRefs(tbl, cache)
			if type(tbl) ~= 'table' then return 0 end
			if cache[tbl] then return 0 end
			cache[tbl] = true
			local counter = 0
			if tbl.ref and not tbl.table then
				counter = counter + 1
			end
			for k, v in pairs(tbl) do
				counter = counter + countRefs(v, cache) + countRefs(k, cache)
			end
			return counter
		end

		getRefs(gameDoc)
		local countBefore = countRefs(gameDoc, {})
		gameDoc = putRefs(gameDoc, {})
		local countAfter = countRefs(gameDoc, {})

		if countAfter ~= 0 then
			print("fixupDoc failed: " .. tostring(countAfter) .. " of " .. tostring(countBefore) .. " refs were not resolved.")
			return nil
		end

		return gameDoc
	end
}}

-- the function returns the full game save document patched to the supported
-- version or a string with an error
function Recovery.loadDoc(saveName)
	local gameDoc = Json.LoadSaveFile(saveName)
	if not gameDoc then
		return lui.COULD_NOT_LOAD_GAME .. " (" .. tostring(saveName) .. ")"
	end
	local version = gameDoc.version
	if not version then
		return lui.COULD_NOT_READ_THE_SAVE_VERSION
	end
	for _, patch in ipairs(Recovery.Patches) do
		if patch.from >= version then
			local errorString = patch.apply(gameDoc)
			if errorString then
				return lui.ERROR_APPLYING_PATCH ..  " (" ..  tostring(patch.from) .. " -> " .. tostring(patch.to) .. "): " .. errorString
			end
		end
	end
	Helpers.implicitVersion = gameDoc.version
	gameDoc = fixupDoc(gameDoc)
	if not gameDoc then
		return lui.COULD_NOT_LOAD_GAME .. " (" .. tostring(saveName) .. ")"
	end
	return gameDoc
end

return Recovery
