-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt
local Engine = require 'Engine'
local ui = require 'pigui.baseui'
local pigui = Engine.pigui

--
-- Function: ui.imageButton
--
-- ui.imageButton(icon, size, frame_padding, bg_color, fg_color, tint_color, tooltip)
--
-- Example:
--
-- > 
--
-- Parameters:
--
--   icon          - image to place on button, e.g. from ui.theme.icons
--   size          - size of button, Vector2
--   frame_padding - number
--   bg_color      - Color, for background
--   fg_color      - Color, for forground
--   tint_color    - Color, 
--   tooltip       - string, mouseover text, will be used as ID, must be unique, append "##uniqueID" if needed
--
-- Returns:
--
--   boolean - true if button was clicked
--
function ui.imageButton(icon, size, frame_padding, bg_color, tint_color, tooltip)
	local uv0, uv1 = get_icon_tex_coords(icon)
	ui.withID(tooltip, function()
		local res = pigui.ImageButton(ui.icons_texture, size, uv0, uv1, frame_padding, bg_color, tint_color)
	end)
	return res
end


--
-- Function: ui.coloredSelectedButton
--
-- ui.coloredSelectedButton(label, thesize, is_selected, bg_color, tooltip, enabled)
--
-- Example:
--
-- > 
--
-- Parameters:
--
--   label       - string,
--   thesize     - Vector2,
--   is_selected - boolean,
--   bg_color    - Color, for background
--   fg_color    - Color, for forground
--   tint_color  - Color, 
--   tooltip     - string, mouseover text, will be used as ID, must be unique, append "##uniqueID" if needed
--
-- Returns:
--
--   boolean - true if button was clicked
--
function ui.coloredSelectedButton(label, thesize, is_selected, bg_color, tooltip, enabled)
	if is_selected then
		pigui.PushStyleColor("Button", bg_color)
		if enabled then
			pigui.PushStyleColor("ButtonHovered", bg_color:tint(0.1))
			pigui.PushStyleColor("ButtonActive", bg_color:tint(0.2))
		else
			pigui.PushStyleColor("ButtonHovered", bg_color)
			pigui.PushStyleColor("ButtonActive", bg_color)
		end
	else
		pigui.PushStyleColor("Button", bg_color:shade(0.6))
		if enabled then
			pigui.PushStyleColor("ButtonHovered", bg_color:shade(0.4))
			pigui.PushStyleColor("ButtonActive", bg_color:shade(0.2))
		else
			pigui.PushStyleColor("ButtonHovered", bg_color)
			pigui.PushStyleColor("ButtonActive", bg_color)
		end
	end
	--pigui.PushID(label)
	local res = pigui.Button(label,thesize)
	--pigui.PopID()
	pigui.PopStyleColor(3)
	if pigui.IsItemHovered() and enabled and tooltip then
		pigui.SetTooltip(tooltip)
	end
	return res
end

--
-- Function: ui.coloredSelectedIconButton
--
-- > clicked = ui.coloredSelectedIconButton(icon, button_size, is_selected,
-- >               frame_padding, bg_color, fg_color, tooltip, img_size)
--
--
-- Example:
--
-- > clicked = ui.coloredSelectedIconButton(ui.theme.icons.bullseye, Vector2(10,10), false,
-- >               0, ui.theme.colors.buttonBlue, Color(255,0,0), "Click for action##42", Vector2(8,8))
--
-- Parameters:
--
--   icon - image to place on button, e.g. from ui.theme.icons
--   button_size - size of button, Vector2
--   is_selected - bool
--   frame_padding - number
--   bg_color - Color(R,G,B), for background
--   fg_color - Color(R,G,B), for forground
--   tooltip - string, mouseover text, will be used as ID, must be unique, append "##uniqueID" if needed
--   img_size - size of icon on the button, Vector2
--
-- Returns:
--
--   clicked - true if button was clicked
--
ui.coloredSelectedIconButton = function(icon, thesize, is_selected, frame_padding, bg_color, fg_color, tooltipID, img_size)
	if is_selected then
		pigui.PushStyleColor("Button", bg_color)
		pigui.PushStyleColor("ButtonHovered", bg_color:tint(0.1))
		pigui.PushStyleColor("ButtonActive", bg_color:tint(0.2))
	else
		pigui.PushStyleColor("Button", bg_color:shade(0.6))
		pigui.PushStyleColor("ButtonHovered", bg_color:shade(0.4))
		pigui.PushStyleColor("ButtonActive", bg_color:shade(0.2))
	end
	local uv0,uv1 = ui.get_icon_tex_coords(icon)
	pigui.PushID(tooltipID)
	local res = pigui.ButtonImageSized(ui.icons_texture, thesize, img_size or Vector2(0,0), uv0, uv1, frame_padding, ui.theme.colors.lightBlueBackground, fg_color)
	pigui.PopID()
	pigui.PopStyleColor(3)
	local pos = tooltipID:find("##") -- get position for id tag start
	local pos = pos and pos - 1      -- if found, move back beyond first "#"
	local tooltip = pos and string.sub(tooltipID, 1, pos) or tooltipID
	if pigui.IsItemHovered() then
		pigui.SetTooltip(tooltip)
	end
	return res
end
