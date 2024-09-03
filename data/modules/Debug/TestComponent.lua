-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- This file tests adding and retrieving Lua BodyComponents to the player,
-- and is used to test serialization of body components.

local Event = require 'Event'
local Game = require 'Game'
local Serializer = require 'Serializer'
local utils = require 'utils'

local TestComponent = utils.inherits(nil, 'TestComponent')

function TestComponent.New()
	local self = setmetatable({}, TestComponent.meta)

	self.testStr = "I am a test string"
	self.testNumber = 1234

	return self
end

--[[
Serializer:RegisterClass('TestComponent', TestComponent)

Event.Register('onGameStart', function()
	print('TestComponent #onGameStart')
	local comp = Game.player:GetComponent('TestComponent')

	if comp then
		print("Loaded from save file, test component:")
		utils.print_r(comp)
	else
		print("Setting player component")
		comp = TestComponent.New()
		Game.player:SetComponent('TestComponent', comp)
		utils.print_r(Game.player:GetComponent('TestComponent'))
	end
end)
--]]
