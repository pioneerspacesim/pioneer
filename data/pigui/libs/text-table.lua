-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = import 'pigui/pigui.lua'

local textTable = {}

function textTable.draw(t)
	for _, entry in ipairs(t) do
		if type(entry) == "table" then
			ui.text(entry[1])
			ui.sameLine(ui.getContentRegion().x - ui.calcTextSize(entry[2]).x)
			ui.text(entry[2])
		elseif type(entry) == "string" then
			ui.spacing()
			ui.text(entry)
			ui.separator()
		elseif entry == false then
			ui.spacing()
		end
	end
end

function textTable.withHeading(name, font, t)
    ui.withFont(font.name, font.size, function()
        ui.text(name)
    end)

    textTable.draw(t)
end

return textTable
