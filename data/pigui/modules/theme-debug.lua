-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = require 'pigui'
local debugView = require 'pigui.views.debug'

local colors = ui.theme.colors

local sortedColors = {}
for name in pairs(colors) do table.insert(sortedColors, name) end
table.sort(sortedColors)

debugView.registerTab('debug-theme-colors', function()
	if not ui.beginTabItem("Theme Colors") then return end
	for _, name in ipairs(sortedColors) do
		local changed, color = nil, colors[name]
		changed, color = ui.colorEdit(name, color, true)
		if changed then colors[name] = color end
	end
	ui.endTabItem()
end)
