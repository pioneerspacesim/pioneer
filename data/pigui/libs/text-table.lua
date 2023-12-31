-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = require 'pigui'
local pigui = require 'Engine'.pigui

-- Interface: textTable

local textTable = {}

-- Function: draw()
-- Draws a simple two-column table that expands to take up the available space.
function textTable.draw(t)
	for _, entry in ipairs(t) do
		if type(entry) == "table" then
			ui.text(entry[1])
			local width, textWidth = ui.getContentRegion().x, ui.calcTextSize(entry[2]).x
			ui.sameLine(math.max(width - textWidth, width / 2 + ui.getItemSpacing().x), 0)
			ui.textWrapped(entry[2])
		elseif type(entry) == "string" then
			ui.spacing()
			ui.text(entry)
			ui.separator()
		elseif entry == false then
			ui.spacing()
		elseif type(entry) == "function" then
			entry()
		end
	end
end

-- Function: withHeading
-- Similar to draw(), this function draws a two-column table that takes up the
-- available space, but with a separate heading element.
function textTable.withHeading(name, font, t)
    ui.withFont(font.name, font.size, function()
        ui.text(name)
    end)

    textTable.draw(t)
end

-- Handle styling and drawing for a table row. Automatically advances columns.
local function drawRow(row, n)
	-- apply font/color to entire row if given
	local font = row.font and pigui:PushFont(row.font.name, row.font.size)
	local color = row.color and pigui.PushStyleColor('Text', row.color)

	for i = 1, n do
		local v = row[i]
		if type(v) == "string" or type(v) == "number" then
			ui.textWrapped(v)
		elseif type(v) == "table" then
			-- apply font/color to single cell if given
			local font = v.font and pigui:PushFont(v.font.name, v.font.size)
			local color = v.color and pigui.PushStyleColor('Text', v.color)

			ui.textWrapped(v[1])

			-- pop font/color
			if font then pigui:PopFont() end
			if color then pigui.PopStyleColor(1) end
		elseif type(v) == "function" then
			v()
		end
		ui.nextColumn()
	end

	-- pop font/color
	if font then pigui:PopFont() end
	if color then pigui.PopStyleColor(1) end
end

-- Function: drawTable()
-- Draw an arbitrary-column table
--
-- Parameters:
--	 nCols - the number of columns in the table.
--   t     - the list of rows to draw
function textTable.drawTable(nCols, widths, t)
	if not t then t, widths = widths, t end
	ui.columns(nCols)
	if widths then
		for i, v in ipairs(widths) do ui.setColumnWidth(i - 1, v) end
	end
	-- apply font/color to the entire table if given
	local font = t.font and pigui:PushFont(t.font.name, t.font.size)
	local color = t.color and pigui.PushStyleColor('Text', t.color)

	for k, v in ipairs(t) do
		drawRow(v, nCols)
		if t.separated and (k == 1 or not t.headerOnly) then ui.separator() end
	end

	-- pop font/color
	if font then pigui:PopFont() end
	if color then pigui.PopStyleColor(1) end
	ui.columns(1)
end

return textTable
