-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = require 'pigui'

local debugView = {
	tabs = {}
}

local childWindowFlags = ui.WindowFlags { "NoSavedSettings", "NoBackground" }

local function drawTab(tab, i, delta)
	local icon = tab.icon or ui.theme.icons.alert_generic
	local icon_str = "{}##{}" % { ui.get_icon_glyph(icon), i }

	local active = false

	ui.tabItem(icon_str, tab.label, function()
		active = true

		ui.child(tab.label, Vector2(0, 0), childWindowFlags, function()
			tab.draw(tab, delta)
		end)
	end)

	return active
end

---@class Debug.DebugTab
---@field icon integer Icon of the debug tab in the tab bar
---@field label string? Tooltip for the tab item
---@field show function? Return true if the tab should be visible
---@field draw function Render the contents of the debug tab
---@field module string? Module name to hot reload, automatically filled

-- Register a new tab to the debug view
---@param name string
---@param tab Debug.DebugTab
function debugView.registerTab(name, tab)
	assert(type(tab) == "table", "Must pass a DebugTab table to debugView.registerTab()")

	tab.label = tab.label or ""
	tab.module = tab.module or package.modulename(2)

	local index = debugView.tabs[name] or #debugView.tabs + 1
	debugView.tabs[index] = tab
	debugView.tabs[name] = index

	table.sort(debugView.tabs, function(a, b)
		return (a.priority and not b.priority) or (a.priority and b.priority and a.priority < b.priority) or (a.priority == b.priority and a.label < b.label)
	end)
end

function debugView.drawTabs(delta)
	for i, tab in ipairs(debugView.tabs) do
		if not tab.show or tab.show() then
			if drawTab(tab, i, delta) then

				if ui.ctrlHeld() and ui.isKeyReleased(string.byte 'r') then
					print("Hot reloading module " .. tab.module)
					package.reimport(tab.module)
				end

			end
		end
	end

end

ui.registerHandler('debug-tabs', debugView.drawTabs)

return debugView
