-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local utils = require 'utils'
local Helpers = require 'pigui.modules.new-game-window.helpers'

---@class NewGameWindow.GameParam
local GameParam = utils.class("pigui.modules.new-game-window.game-param")

setmetatable(GameParam, { __index = function (self, key) assert(false, tostring(self) .. ": unexpected key - " .. tostring(key)) end })

function GameParam:Constructor(name, path)
	assert(name)
	assert(path)
	self.name = name
	self.path = path
	self.lock = false
	self.value = nil
	self.hidden = false
	self.noName = false
end

function GameParam:isValid()                 assert(false, tostring(self.name) .. ":isValid() should be overridden.") end
function GameParam:draw()                    assert(false, tostring(self.name) .. ":draw() Should be overridden.") end
function GameParam:fromStartVariant(variant) assert(false, tostring(self.name) .. ":fromStartVariant() Should be overridden.") end

-- A versioned function that tries to read data from saveGame.
-- either returns the value of the parameter, or nil and an error string
--
-- If in the next version the GameParam.value does not change, but is just
-- taken from another place(s) in saveFile document, you can simply define a
-- new reader. For older saves, older readers will be used.
--
-- If the meaning and content of the GameParam.value changes, there are 2 ways
-- - either fix all existing old reader versions so that they generate a value
-- of a new structure, or create a "patch", a procedure that changes the older
-- document so that a new reader can read it. In the second case, all old
-- readers (since they are not compatible in value with the new reader), must
-- be removed from this array. Old readers can be reused when creating a patch.
GameParam.reader = Helpers.versioned {{
	version = 1,
	fnc = function(_)
		assert(false, "No saveGame readers")
	end
}}

---@param saveGame table
---@return string? errorString
function GameParam:fromSaveGame(saveGame)
	local value, errorString = self.reader(saveGame)
	if errorString then return errorString end
	self.value = value
end

function GameParam:isEmpty()
	return not self.value
end

function GameParam:random() end

return GameParam
