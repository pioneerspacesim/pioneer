-- Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local utils = require 'utils'

-- Elm-style model-view-update pattern
-- The render function should treat self as immutable and instead process all
-- data updates through the message() system
---@class UI.Module
local Module = utils.class("UI.Module")

function Module:Constructor()
	self.__messages = {}
end

function Module:message(id, ...)
	table.insert(self.__messages, { id = id, args = table.pack(...) })
end

function Module:hookMessage(id, fun, receiver)
	local oldHandler = self[id]

	self[id] = function(self, ...)
		if oldHandler then oldHandler(self, ...) end
		return fun(receiver, ...)
	end
end

function Module:update()
	if #self.__messages == 0 then
		return
	end

	local messages = self.__messages
	self.__messages = {}

	for i, msg in ipairs(messages) do
		local msgHandler = self[msg.id]

		if not msgHandler then
			error(string.format("No message handler for %s on self %s", msg.id, getmetatable(self).class))
		end

		msgHandler(self, table.unpack(msg.args, 1, msg.args.n))
	end
end

function Module:render()
	return
end

return Module
