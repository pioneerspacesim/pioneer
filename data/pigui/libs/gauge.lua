-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

---@class ui
local ui = require 'pigui.baseui'
local Vector2 = _G.Vector2

local gauge_show_percent = true
ui.gauge_height = ui.rescaleUI(25, Vector2(1600, 900))
ui.gauge_width = ui.rescaleUI(275, Vector2(1600, 900))

--
-- Function: ui.gauge
--
-- ui.gauge(position, value, unit, format, minimum, maximum, icon,
--          color, tooltip, width, height, formatFont, percentFont)
--
-- Display a gauge at the given screen position.
--
-- +-----+------+-----------------------------+
-- | XX% | Icon |[Format unit     *bar*]      |
-- +-----+------+-----------------------------+
--
--
-- Example:
--
-- >
--
-- Parameters:
--
--   position    - Vector2, screen position
--   value       - number, value to display (clamped between minimum and maximum)
--   unit        - string, a unit value to display after the value text. Only shown if format is not nil
--   format      - string, a format string used to display the value text. A value of nil will hide the value text
--   minimum     - number, the minimum value (0%)
--   maximum     - number, the maximum value (100%)
--   icon        - number, id of an icon to display next to the guage bar
--   color       - Color, color of the gauge bar
--   tooltip     - string, tooltip shown on mouseover
--   width       - number, width of the gauge
--   height      - number, height of the gauge
--   formatFont  -
--   percentFont -
--
-- Returns:
--
--   nil
--
ui.gauge = function(position, value, unit, format, minimum, maximum, icon, color, tooltip, width, height, formatFont, percentFont)
	local percent = math.clamp((value - minimum) / (maximum - minimum), 0, 1)
	local uiPos = Vector2(position.x, position.y)
	local gauge_width = width or ui.gauge_width
	local gauge_height = height or ui.gauge_height
	ui.withFont(ui.fonts.pionillium.medium.name, ui.fonts.pionillium.medium.size, function()
		ui.addLine(uiPos, Vector2(uiPos.x + gauge_width, uiPos.y), ui.theme.colors.gaugeBackground, gauge_height, false)
		if gauge_show_percent then
			local one_hundred = ui.calcTextSize("100%")
			uiPos.x = uiPos.x + one_hundred.x * 1.2 -- 1.2 for a bit of slack
			ui.addStyledText(Vector2(uiPos.x, uiPos.y + gauge_height / 12), ui.anchor.right, ui.anchor.center, string.format("%i%%", percent * 100), ui.theme.colors.reticuleCircle, percentFont or ui.fonts.pionillium.medium, tooltip)
		end
		uiPos.x = uiPos.x + gauge_height * 1.2
		ui.addIcon(Vector2(uiPos.x - gauge_height / 2, uiPos.y), icon, ui.theme.colors.reticuleCircle, Vector2(gauge_height * 0.9, gauge_height * 0.9), ui.anchor.center, ui.anchor.center, tooltip)
		local w = (position.x + gauge_width) - uiPos.x
		ui.addLine(uiPos, Vector2(uiPos.x + w * percent, uiPos.y), color, gauge_height, false)

		formatFont = formatFont or ui.fonts.pionillium.small
		if value and format then
			ui.addFancyText(Vector2(uiPos.x + gauge_height/2, uiPos.y), ui.anchor.left, ui.anchor.center, {
				{ text=string.format(format, value), color=ui.theme.colors.reticuleCircle,     font=formatFont, tooltip=tooltip },
				{ text=unit,                         color=ui.theme.colors.reticuleCircleDark, font=formatFont, tooltip=tooltip }},
				ui.theme.colors.gaugeBackground)
		end
	end)
end
