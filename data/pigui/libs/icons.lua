-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt
local Engine = require 'Engine'
local pigui = Engine.pigui

---@class ui
local ui = require 'pigui.baseui'

---@alias ui.Icon integer

local iconsX = 16
local iconsY = 19

local icons_texture_small = pigui:LoadTextureFromSVG(pigui.DataDirPath({"icons", "icons.svg"}), iconsX * 24, iconsY * 24)
local icons_texture_med = pigui:LoadTextureFromSVG(pigui.DataDirPath({"icons", "icons.svg"}), iconsX * 32, iconsY * 32)
local icons_texture_large = pigui:LoadTextureFromSVG(pigui.DataDirPath({"icons", "icons.svg"}), iconsX * 64, iconsY * 64)

local function get_wide_icon_tex_coords(icon)
	assert(icon, "no icon given")
	local rem = math.floor(icon % iconsX)
	local quot = math.floor(icon / iconsX)
	return Vector2(rem / iconsX, quot/iconsY), Vector2((rem+2) / iconsX, (quot+1)/iconsY)
end

function ui.get_icon_tex_coords(icon)
	assert(icon, "no icon given")
	local rem = math.floor(icon % iconsX)
	local quot = math.floor(icon / iconsX)
	return Vector2(rem / iconsX, quot/iconsY), Vector2((rem+1) / iconsX, (quot+1)/iconsY)
end

function ui.get_icons_texture(size)
	if size.x > 32.0 or size.y > 32.0 then
		return icons_texture_large
	elseif size.x <= 24.0 or size.y <= 24.0 then
		return icons_texture_small
	else
		return icons_texture_med
	end
end

--
-- Function: ui.addIcon
--
-- ui.addIcon(position, icon, color, size, anchor_horizontal,
--            anchor_vertical, tooltip, angle_rad)
--
-- Display an icon at the given screen position
--
--
-- Example:
--
-- >
--
-- Parameters:
--
--   position - Vector2, screen position
--   icon              - number, icon id ie ui.theme.icons.prograde
--   color             - Color, icon color
--   size              - Vector2, size to display the icon
--   anchor_horizontal - number, one of the following values:
--                         - ui.anchor.left   (1)
--                         - ui.anchor.right  (2)
--                         - ui.anchor.center (3)
--   anchor_vertical   - number, one of the following values:
--                         - ui.anchor.top    (4)
--                         - ui.anchor.center (3)
--                         - ui.anchor.bottom (5)
--   tooltip           - string, tooltip to be displayed on mouseover
--   angle_rad         - number, radians to rotate the icon
--
-- Returns:
--
--   number - the size of the icon displayed
--
function ui.addIcon(position, icon, color, size, anchor_horizontal, anchor_vertical, tooltip, angle_rad)
	local pos = ui.calcTextAlignment(position, size, anchor_horizontal, anchor_vertical)
	local uv0, uv1 = ui.get_icon_tex_coords(icon)
	if angle_rad then
		local center = Vector2(pos.x + pos.x + size.x, pos.y + pos.y + size.y) / 2
		local up_left = Vector2(-size.x/2, size.y/2):rotate(angle_rad)
		local up_right = up_left:right()
		local down_left = up_left:left()
		local down_right = -up_left
		pigui.AddImageQuad(ui.get_icons_texture(size), center + up_left, center + up_right, center + down_right, center + down_left, uv0, Vector2(uv1.x, uv0.y), uv1, Vector2(uv0.x, uv1.y), color)
	else
		pigui.AddImage(ui.get_icons_texture(size), pos, pos + size, uv0, uv1, color)
	end
	if tooltip and (ui.isMouseHoveringWindow() or not ui.isAnyWindowHovered()) and tooltip ~= "" then
		if pigui.IsMouseHoveringRect(pos, pos + size, true) then
			ui.maybeSetTooltip(tooltip)
		end
	end
	return size
end

--
-- Function: ui.addWideIcon
--
-- ui.addWideIcon(position, icon, color, size, anchor_horizontal,
--                anchor_vertical, tooltip, angle_rad)
--
-- Display a wide, double width, icon at the given screen position
--
--
-- Example:
--
-- >
--
-- Parameters:
--
--   position - Vector2, screen position
--   icon              - number, icon id ie ui.theme.icons.prograde
--   color             - Color, icon color
--   size              - Vector2, size to display the icon
--   anchor_horizontal - number, one of the following values:
--                         - ui.anchor.left   (1)
--                         - ui.anchor.right  (2)
--                         - ui.anchor.center (3)
--   anchor_vertical   - number, one of the following values:
--                         - ui.anchor.top    (4)
--                         - ui.anchor.center (3)
--                         - ui.anchor.bottom (5)
--   tooltip           - string, tooltip to be displayed on mouseover
--   angle_rad         - number, radians to rotate the icon
--
-- Returns:
--
--   number - the size of the icon displayed
--
function ui.addWideIcon(position, icon, color, size, anchor_horizontal, anchor_vertical, tooltip, angle_rad)
	local pos = ui.calcTextAlignment(position, size, anchor_horizontal, anchor_vertical)
	local uv0, uv1 = get_wide_icon_tex_coords(icon)
	if angle_rad then
		local center = (pos + pos + size) / 2
		local up_left = Vector2(-size.x/2, size.y/2):rotate2d(angle_rad)
		local up_right = up_left:right()
		local down_left = up_left:left()
		local down_right = -up_left
		pigui.AddImageQuad(ui.get_icons_texture(size), center + up_left, center + up_right, center + down_right, center + down_left, uv0, Vector2(uv1.x, uv0.y), uv1, Vector2(uv0.x, uv1.y), color)
	else
		pigui.AddImage(ui.get_icons_texture(size), pos, pos + size, uv0, uv1, color)
	end
	if tooltip and (ui.isMouseHoveringWindow() or not ui.isAnyWindowHovered()) and tooltip ~= "" then
		if pigui.IsMouseHoveringRect(pos, pos + size, true) then
			ui.maybeSetTooltip(tooltip)
		end
	end

	return size
end

--
-- Function: ui.addIconSimple
--
-- Display an icon at the given screen position
--
-- Example:
--
-- > ui.addIconSimple(screenPos, ui.theme.icons.prograde, size, color)
--
-- Parameters:
--
--   pos     - Vector2, screen position
--   icon    - number, icon id into the icons texture
--   size    - Vector2, size to display the icon
--   color   - Color, icon color
--   tooltip - string, tooltip to display when hovering icon
--
function ui.addIconSimple(pos, icon, size, color, tooltip)
	local uv0, uv1 = ui.get_icon_tex_coords(icon)
	pigui.AddImage(ui.get_icons_texture(size), pos, pos + size, uv0, uv1, color)

	if tooltip and (ui.isMouseHoveringWindow() or not ui.isAnyWindowHovered()) then
		if pigui.IsMouseHoveringRect(pos, pos + size, true) then
			ui.maybeSetTooltip(tooltip)
		end
	end
end

--
-- Function: ui.icon
--
-- ui.icon(icon, size, color, tooltip)
--
-- Display an icon
--
--
-- Example:
--
-- >
--
-- Parameters:
--
--   icon              - number, icon id ie ui.theme.icons.prograde
--   size              - Vector2, size to display the icon
--   color             - Color, icon color
--   tooltip           - string, tooltip to be displayed on mouseover
--
-- Returns:
--
--   nil
--
function ui.icon(icon, size, color, tooltip)
	local uv0, uv1 = ui.get_icon_tex_coords(icon)
	pigui.Image(ui.get_icons_texture(size), size, uv0, uv1, color)
	if tooltip and ui.isItemHovered() then
		ui.setTooltip(tooltip)
	end
end
