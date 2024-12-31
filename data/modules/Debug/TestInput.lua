-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- This file tests the Lua input API

--[[
local Input = require 'Input'

local action_disable = Input.RegisterActionBinding("TestInputDisable", "Debug.TestInput", { activator = { key = Input.keys["return"] } })

local frame = Input.CreateInputFrame("TestInputFrame", true)
frame:AddAction(action_disable)

action_disable:OnPressed(function()
	frame:RemoveFromStack()

	print("Test!")
end)

require 'Event'.Register("onGameStart", function()
	frame:AddToStack()
end)
--]]
