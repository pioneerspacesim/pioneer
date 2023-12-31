-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local utils = require 'utils'

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
function GameParam:fromSaveGame(saveGame)    assert(false, tostring(self.name) .. ":fromSave() Should be overridden.") end
function GameParam:fromStartVariant(variant) assert(false, tostring(self.name) .. ":fromStartVariant() Should be overridden.") end


function GameParam:setLock(value)
	self.lock = value
end

function GameParam:isEmpty()
	return not self.value
end

function GameParam:random() end

return GameParam
