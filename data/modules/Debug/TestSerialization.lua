-- Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Serializer = require 'Serializer'
local utils = require 'utils'

-- This file ensures unserializers that return different values are respected
-- by the lua serialization system - references to the original table should
-- be unpickled into the modified table returned by the original serializer.

-- Note: nested references to a table being deserialized will not be
-- automatically converted to references to the "replacement" table.

-- Disable this test until needed.
--[[

local testClass = utils.inherits(nil, 'TestClass')

local testInstance = testClass.New()

function testClass:Unserialize(data)
	return testInstance
end

local function serialize()
	return {
		a = testInstance,
		b = testInstance,
		c = testInstance
	}
end

local function unserialize(data)
	utils.print_r(data.a)
	utils.print_r(data.b)
	utils.print_r(data.c)

	print(data.a == data.b, data.a == data.c, data.b == data.c)

	print(testInstance)
	print(data.a == testInstance, data.b == testInstance, data.c == testInstance)
end

Serializer:RegisterClass('TestClass', testClass)
Serializer:Register('TestSerialization', serialize, unserialize)
--]]
