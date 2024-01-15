-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt
local Engine = require 'Engine'
local pigui = Engine.pigui
local Vector2 = _G.Vector2

---@class ui
local ui = require 'pigui.baseui'

--
-- Function: ui.withButtonColors
--
-- > ui.withButtonColors(variant, fnc)
--
-- Example:
--
-- > ui.withButtonColors(ui.theme.buttonColors.transparent, function() ui.thrustIndicator( .... ) end)
--
-- Parameters:
--
--   variant - Table, color variants used for this button;
--             contains Color fields 'normal', 'hovered', and 'active'
--             if nil, nothing is pushed
--   fnc     - Function, procedure to perform with set colors
--
-- Returns:
--
--   what the function fnc returns
--
function ui.withButtonColors(variant, fnc)
	if variant then
		pigui.PushStyleColor("Button", variant.normal)
		pigui.PushStyleColor("ButtonHovered", variant.hovered)
		pigui.PushStyleColor("ButtonActive", variant.active)
		local ret = table.pack(fnc())
		pigui.PopStyleColor(3)
		return table.unpack(ret, 1, ret.n)
	else
		return fnc()
	end
end

--
-- Function: ui.button
--
-- > clicked = ui.button(label, button_size, variant, tooltip)
--
-- Example:
--
-- > clicked = ui.button("Click me", Vector2(100,0), ui.theme.buttonColors.default, "Does the thing")
--
-- Parameters:
--
--   label       - string, text rendered on the button, must be unique.
--                 Append any text following "##" to make a unique ID
--   button_size - [optional] Vector2, giving size of button
--   variant     - [optional] Table, color variants used for this button;
--                 contains Color fields 'normal', 'hovered', and 'active'
--   tooltip     - [optional] string, mouseover text
--
-- Returns:
--
--   clicked - true if button was clicked
--
function ui.button(label, button_size, variant, tooltip)
	if variant then
		pigui.PushStyleColor("Button", variant.normal)
		pigui.PushStyleColor("ButtonHovered", variant.hovered)
		pigui.PushStyleColor("ButtonActive", variant.active)
	end

	pigui.PushStyleVar("FramePadding", ui.theme.styles.ButtonPadding)
	local res = pigui.Button(label, button_size or Vector2(0, 0))
	pigui.PopStyleVar(1)

	if variant then
		pigui.PopStyleColor(3)
	end

	if pigui.IsItemHovered() and tooltip then pigui.SetTooltip(tooltip) end
	return res
end

function ui.alignTextToButtonPadding()
	pigui.PushStyleVar("FramePadding", ui.theme.styles.ButtonPadding)
	ui.alignTextToFramePadding()
	pigui.PopStyleVar(1)
end

--
-- Function: ui.calcButtonSize
--
-- Calculate the size of a button drawn with the given label, optionally under
-- the given font instead of the current font.
--
-- Parameters:
--   label - string, label of the button to calculate size for
--   font  - Font|string, optional font to use when calculating button size
--   size  - number|nil, optional font size to use
--
function ui.calcButtonSize(label, font, size)
	return ui.calcTextSize(label, font, size) + ui.theme.styles.ButtonPadding * 2
end

function ui.getButtonHeight(font)
	return (font and font.size or ui.getTextLineHeight()) + ui.theme.styles.ButtonPadding.y * 2
end

function ui.getButtonHeightWithSpacing()
	return ui.getTextLineHeightWithSpacing() + ui.theme.styles.ButtonPadding.y * 2.0
end

--
-- Function: ui.iconButton
--
-- > ui.iconButton(icon, size, tooltip, variant, fg_color, frame_padding, icon_size)
--
-- Example:
--
-- > if ui.iconButton(icons.random, buttonSpaceSize, l.RANDOM_FACE, nil, nil, 0, iconSize) then
-- >   doSmth()
-- > end
--
-- Parameters:
--
--   icon          - image to place on button, e.g. from ui.theme.icons
--   size          - size of button, Vector2
--   tooltip       - string, mouseover text, will be used as ID, must be unique, append "##uniqueID" if needed
--   variant       - [optional] Table, color variants used for this button;
--                   contains Color fields 'normal', 'hovered', and 'active'
--                   can be true for selected and false for default color variant
--   fg_color      - [optional] Color, for foreground, default - ui.theme.colors.white
--   frame_padding - [optional] number - the width of the frame around the icon in the button, default = 0
--   icon_size     - [optional] size of icon on the button, Vector2
--
-- Returns:
--
--   boolean - true if button was clicked
--
function ui.iconButton(icon, size, tooltip, variant, fg_color, frame_padding, icon_size)
	local uv0, uv1 = ui.get_icon_tex_coords(icon)
	local res = nil
	if not frame_padding then
		frame_padding = 0
	end
	if not fg_color then
		fg_color = ui.theme.colors.buttonInk
	end
	if variant == true then
		variant = ui.theme.buttonColors.selected
	end

	ui.withID(tooltip, function()
		ui.withButtonColors(variant, function()
			if icon_size then
				res = pigui.ButtonImageSized(ui.get_icons_texture(size), size, icon_size, uv0, uv1, frame_padding, ui.theme.colors.transparent, fg_color)
			else
				res = pigui.ImageButton(ui.get_icons_texture(size), size, uv0, uv1, frame_padding, ui.theme.colors.transparent, fg_color)
			end
		end)
	end)

	if pigui.IsItemHovered() then
		local pos = tooltip:find("##") -- get position for id tag start
		if not pos then
			ui.setTooltip(tooltip)
		elseif pos > 1 then
			ui.setTooltip(string.sub(tooltip, 1, pos - 1))
		end
	end

	return res
end

--
-- Function: ui.mainMenuButton
--
-- > clicked = ui.mainMenuButton(icon, tooltip, variant, size, fg_color)
--
-- Example:
--
-- > clicked = ui.mainMenuButton(ui.theme.icons.ship, "Tooltip", ui.theme.buttonColors.transparent, ui.theme.colors.unknown)
--
-- Parameters:
--
--   icon          - icon index from ui.theme.icons (integer number)
--   tooltip       - string, mouseover text, will be used as ID, must be
--                   unique, append "##uniqueID" if needed
--   variant       - [optional] Table, color variants used for this button;
--                   contains Color fields 'normal', 'hovered', and 'active'.
--                   can be true for selected and false (nil) for default color variant
--   size          - [optional] size of button, Vector2, default - ui.theme.styles.MainButtonSize
--   fg_color      - [optional] Color, for foreground, default - ui.theme.colors.white
--
-- Returns:
--
--   clicked - true if button was clicked
--
function ui.mainMenuButton(icon, tooltip, variant, size, fg_color)
	if size == nil then
		size = ui.theme.styles.MainButtonSize
	end
	if fg_color == nil then
		fg_color = ui.theme.colors.buttonInk
	end
	if variant == true then
		variant = ui.theme.buttonColors.selected
	end
	return ui.iconButton(icon, size, tooltip, variant, fg_color, ui.theme.styles.MainButtonPadding)
end

--
-- Function: ui.inlineIconButton
--
-- Draw an icon button sized to match the current font size. Calling
-- alignTextToButtonPadding() is recommended if submitting text before the
-- icon button.
--
-- Example:
--
-- > clicked = ui.inlineButton(ui.theme.icons.ship, "Tooltip", ui.theme.buttonColors.transparent, ui.theme.colors.unknown)
--
-- Parameters:
--
--   icon          - icon index from ui.theme.icons (integer number)
--   tooltip       - string, mouseover text, will be used as ID, must be
--                   unique, append "##uniqueID" if needed
--   variant       - [optional] Table, color variants used for this button;
--                   contains Color fields 'normal', 'hovered', and 'active'.
--                   can be true for selected and false (nil) for default color variant
--   fg_color      - [optional] Color, for foreground, default - ui.theme.colors.white
--
-- Returns:
--
--   clicked - true if button was clicked
--
function ui.inlineIconButton(icon, tooltip, variant, fg_color)
	local height = ui.getTextLineHeight() + (ui.theme.styles.ButtonPadding.y - ui.theme.styles.MainButtonPadding) * 2
	return ui.iconButton(icon, Vector2(height), tooltip, variant, fg_color, ui.theme.styles.MainButtonPadding)
end
