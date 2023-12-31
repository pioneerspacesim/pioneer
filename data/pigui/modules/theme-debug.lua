-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = require 'pigui'
local debugView = require 'pigui.views.debug'

local colors = ui.theme.colors
local styleColors = ui.theme.styleColors

local sortedColors, sortedStyleColors = {}, {}
for name in pairs(colors) do table.insert(sortedColors, name) end
for name in pairs(styleColors) do table.insert(sortedStyleColors, name) end
table.sort(sortedColors)
table.sort(sortedStyleColors)

debugView.registerTab('debug-theme-colors', function()
	if not ui.beginTabItem("Theme Colors") then return end
	ui.text("Palette Colors")
	ui.separator()
	for _, name in ipairs(sortedStyleColors) do
		local changed, ncolor = ui.colorEdit(name, styleColors[name], { "NoAlpha" })
		-- if we're changing semantic colors, we want to update all uses of the color object
		if changed then styleColors[name](ncolor.r, ncolor.g, ncolor.b) end
	end

	ui.newLine()
	ui.text("Semantic Colors")
	ui.separator()
	for _, name in ipairs(sortedColors) do
		local changed, color = ui.colorEdit(name, colors[name])
		if changed then colors[name] = color end
	end
	ui.endTabItem()
end)
