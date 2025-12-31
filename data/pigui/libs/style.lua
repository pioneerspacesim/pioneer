-- Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- Convenience helper to manage ImGui style manipulation

local pigui = require 'Engine'.pigui
local utils = require 'utils'

--
-- Class: UI.Style
--
-- Bundles up a set of ImGui color and style var overrides, allowing them to be
-- applied in a single call without recreating the table every frame.
--
-- May eventually internally store a C++ object representing the style overrides
-- in a more efficient manner with less runtime cost to push/pop the changes to
-- the global style.
--
-- Note that runtime modification of the contained styles is not allowed; all
-- styles should be set during the clone() call.
--
---@class UI.Style
local Style = utils.proto("UI.Style")

Style.colors = {}
Style.vars = {}

---@param mixin { colors: table<string, Color>, vars: table<string, number|Vector2> }
---@return UI.Style
function Style:clone(mixin)
	---@class UI.Style
	local new = { __index = self }

	new.colors = table.merge(table.copy(self.colors), mixin.colors)
	new.vars = table.merge(table.copy(self.vars), mixin.vars)

	new.num_colors = utils.count(new.colors)
	new.num_vars = utils.count(new.vars)

	return setmetatable(new, new)
end

function Style:withStyle(fun)
	self:push()
	fun()
	self:pop()
end

function Style:push()
	for key, val in pairs(self.colors) do
		pigui.PushStyleColor(key, val)
	end

	for key, val in pairs(self.vars) do
		pigui.PushStyleVar(key, val)
	end
end

function Style:pop()
	pigui.PopStyleColor(self.num_colors)
	pigui.PopStyleVar(self.num_vars)
end

return Style
