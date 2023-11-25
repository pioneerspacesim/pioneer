-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = require 'pigui'

local debugView = {
	tabs = {}
}

local function encode_icon(idx)
	idx = idx + 0xF000
	return string.char(
		0xe0 + bit32.rshift(idx, 12),
		0x80 + bit32.band(bit32.rshift(idx, 6), 0x3f),
		0x80 + bit32.band(idx, 0x3f))
end

local childWindowFlags = ui.WindowFlags { "NoSavedSettings", "NoBackground" }

local function drawTab(tab, i, delta)
	local icon = tab.icon or ui.theme.icons.alert_generic
	local label = tab.label or ""
	local icon_str = "{}##{}" % { encode_icon(icon), i }

	ui.tabItem(icon_str, label, function()
		ui.child(label, Vector2(0, 0), childWindowFlags, function()
			tab.draw(delta)
		end)
	end)
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
	if type(tab) == "function" then
		tab = {
			icon = ui.theme.icons.alert_generic,
			draw = tab
		}
	end

	if not tab.module then
		tab.module = package.modulename(2)
	end

	local index = debugView.tabs[name] or #debugView.tabs + 1
	debugView.tabs[index] = tab
	debugView.tabs[name] = index
end

function debugView.drawTabs(delta)
	for i, tab in ipairs(debugView.tabs) do
		if not tab.show or tab.show() then
			drawTab(tab, i, delta)

			if ui.ctrlHeld() and ui.isKeyReleased(string.byte 'r') then
				print("Hot reloading module " .. tab.module)
				package.reimport(tab.module)
			end
		end
	end
end

ui.registerHandler('debug-tabs', debugView.drawTabs)

return debugView
