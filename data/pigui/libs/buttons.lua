-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt
local Engine = require 'Engine'
local pigui = Engine.pigui
local Vector2 = _G.Vector2

---@class ui
local ui = require 'pigui.baseui'

--
-- Function: ui.getButtonColor
--
-- Return a color from the passed button style color object, based on the given
-- state flags for the button.
--
-- > color = ui.getButtonColor(variant, isHovered, isActive)
--
function ui.getButtonColor(variant, hovered, active)
	if active then
		return variant.active
	end

	return (active and variant.active)
		or hovered and variant.hovered or variant.normal
end

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
--   font  - Font, optional font to use when calculating button size
--
function ui.calcButtonSize(label, font)
	return ui.calcTextSize(label, font) + ui.theme.styles.ButtonPadding * 2
end

function ui.getButtonHeight(font)
	return (font and font.size or ui.getTextLineHeight()) + ui.theme.styles.ButtonPadding.y * 2
end

function ui.getButtonHeightWithSpacing(font)
	return ui.getButtonHeight(font) + ui.getItemSpacing().y
end

--
-- Function: ui.iconButton
--
-- Draws a square or rectangular button with the given icon glyph. The default
-- size of an icon button is the text height + current frame padding, for
-- consistency with other framed widgets.
--
-- > ui.iconButton(id, icon, tooltip, variant, size, padding, flags)
--
-- Example:
--
-- > if ui.iconButton("btn", icons.random, l.MY_TOOLTIP, isSelected) then
-- >   doSmth()
-- > end
--
-- Parameters:
--
--   id            - string, unique identifier for the icon button
--   icon          - image to place on button, e.g. from ui.theme.icons
--   tooltip       - string, mouseover text
--   variant       - [optional] Table, color variants used for this button;
--                   contains Color fields 'normal', 'hovered', and 'active'
--                   can be true for selected and false for default color variant
--   size          - size of button, Vector2
--   padding       - [optional] Vector2 - the width of the frame around the icon in the button
--   flags         - [optional] ImGuiButtonFlags enum controlling button behavior
--
-- Returns:
--
--   boolean - true if button was clicked
--
---@param id string
---@param icon any
---@param tooltip string?
---@param variant any
---@param size Vector2?
---@param padding Vector2?
---@param flags any
---@return boolean
function ui.iconButton(id, icon, tooltip, variant, size, padding, flags)
	padding = padding or ui.theme.styles.IconButtonPadding
	variant = variant == true and ui.theme.buttonColors.selected or variant

	local height = (size and size.y > 0 and size.y or ui.getFrameHeight()) - padding.y * 2
	local glyph = ui.get_icon_glyph(icon)
	local ret = false

	local pop = pigui:PushFont("icons", height)
	pigui.PushStyleVar("FramePadding", padding)

	ui.withButtonColors(variant, function()
		ret = pigui.GlyphButton(id, glyph, size, flags)
	end)

	pigui.PopStyleVar(1)
	if pop then
		pigui:PopFont()
	end

	if tooltip and pigui.IsItemHovered() then
		ui.setTooltip(tooltip)
	end

	return ret
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
--
-- Returns:
--
--   clicked - true if button was clicked
--
function ui.mainMenuButton(icon, tooltip, variant, size)
	if variant == true then
		variant = ui.theme.buttonColors.selected
	end

	local tt = string.match(tooltip, "^(.+)##.+$")
	return ui.iconButton(tooltip, icon, tt or tooltip, variant, size or ui.theme.styles.MainButtonSize)
end

--
-- Function: ui.inlineIconButton
--
-- Draw an icon button sized to match the current text line height.
-- Inline icon buttons do not add padding total extend vertically beyond the
-- height of the text; use iconButton if that is desired instead.
--
-- Example:
--
-- > clicked = ui.inlineButton("btn", ui.theme.icons.ship, "Tooltip", ui.theme.buttonColors.transparent)
--
-- Parameters:
--
--   id            - string, used as ID for the button; must be unique
--   icon          - icon index from ui.theme.icons (integer number)
--   tooltip       - string, mouseover text
--   variant       - [optional] Table, color variants used for this button;
--                   contains Color fields 'normal', 'hovered', and 'active'.
--                   can be true for selected and false (nil) for default color varian
--   flags         - [optional] ImGuiButtonFlags enum controlling button behavior
--
-- Returns:
--
--   clicked - true if button was clicked
--
function ui.inlineIconButton(id, icon, tooltip, variant, flags)
	return ui.iconButton(id, icon, tooltip, variant, Vector2(0, ui.getLineHeight()), ui.theme.styles.InlineIconPadding, flags)
end
