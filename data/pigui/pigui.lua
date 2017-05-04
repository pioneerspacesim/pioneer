-- Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Format = import('Format')
local Game = import('Game')
local Space = import('Space')
local Engine = import('Engine')
local Event = import("Event")
local ShipDef = import("ShipDef")
local Vector = import("Vector")
local Color = import("Color")
local Lang = import("Lang")

local lui = Lang.GetResource("ui-core");
local lc = Lang.GetResource("core");
local lec = Lang.GetResource("equipment-core");

local utils = import("utils")
local pigui = Engine.pigui

local pi = 3.14159264
local pi_2 = pi / 2
local pi_4 = pi / 4
local two_pi = pi * 2
local standard_gravity = 9.80665
local one_over_sqrt_two = 1 / math.sqrt(2)

local ui = { }

local function maybeSetTooltip(tooltip)
	if not Game.player:IsMouseActive() then
		pigui.SetTooltip(tooltip)
	end
end

local textBackgroundMarginPixels = 2

ui.icons_texture = pigui:LoadTextureFromSVG(pigui.DataDirPath({"icons", "icons.svg"}), 16 * 64, 16 * 64)

function ui.window(name, params, fun)
	pigui.Begin(name, params)
	fun()
	pigui.End()
end

function ui.group(fun)
	pigui.BeginGroup()
	fun()
	pigui.EndGroup()
end

function ui.popup(name, fun)
	if pigui.BeginPopup(name) then
		fun()
		pigui.EndPopup()
	end
end

function ui.withFont(name, size, fun)
	local font = pigui:PushFont(name, size)
	fun()
	if font then
		pigui.PopFont()
	end
end

function ui.withStyleColors(styles, fun)
	for k,v in pairs(styles) do
		pigui.PushStyleColor(k, v)
	end
	fun()
	pigui.PopStyleColor(utils.count(styles))
end

pigui.handlers.INIT = function(progress)
	if pigui.handlers and pigui.handlers.init then
		pigui.handlers.init(progress)
	end
end

pigui.handlers.GAME = function(deltat)
	if pigui.handlers and pigui.handlers.game then
		pigui.handlers.game(deltat)
	end
end

pigui.handlers.MAINMENU = function(deltat)
	if pigui.handlers and pigui.handlers.mainMenu then
		pigui.handlers.mainMenu(deltat)
	end
end

local function get_icon_tex_coords(icon)
	assert(icon, "no icon given")
	local count = 16.0 -- icons per row/column
	local rem = math.floor(icon % count)
	local quot = math.floor(icon / count)
	return Vector(rem / count, quot/count), Vector((rem+1) / count, (quot+1)/count)
end

ui.registerHandler = function(name, fun)
	pigui.handlers[name] = fun
end

ui.circleSegments = function(radius)
	if radius < 5 then
		return 8
	elseif radius < 20 then
		return 16
	elseif radius < 50 then
		return 32
	elseif radius < 100 then
		return 64
	else
		return 128
	end
end

ui.Format = {
	Duration = function(duration, elements)
		-- shown elements items (2 -> wd or dh, 3 -> dhm or hms)
		local negative = false
		if duration < 0 then
			duration = math.abs(duration)
			negative = true
		end
		local seconds = math.floor(duration % 60)
		local minutes = math.floor(duration / 60 % 60)
		local hours = math.floor(duration / 60 / 60 % 24)
		local days = math.floor(duration / 60 / 60 / 24 % 7)
		local weeks = math.floor(duration / 60 / 60 / 24 / 7)
		local i = elements or 5
		local count = false
		local result = ""
		if i > 0 then
			if weeks ~= 0 then
				result = result .. weeks .. "w"
				count = true
			end
			if count then
				i = i - 1
			end
		end
		if i > 0 then
			if days ~= 0 then
				result = result .. days .. "d"
				count = true
			end
			if count then
				i = i - 1
			end
		end
		if i > 0 then
			if hours ~= 0 then
				result = result .. hours .. "h"
				count = true
			end
			if count then
				i = i - 1
			end
		end
		if i > 0 then
			if minutes ~= 0 then
				result = result .. minutes .. "m"
				count = true
			end
			if count then
				i = i - 1
			end
		end
		if i > 0 then
			if seconds ~= 0 then
				result = result .. seconds .. "s"
				count = true
			end
			if result == "" then
				result = "0s"
			end
			if count then
				i = i - 1
			end
		end
		if negative then
			result = "-" .. result
		end
		return result
	end,
	Distance = function(distance)
		local d = math.abs(distance)
		if d < 1000 then
			return math.floor(distance), lc.UNIT_METERS
		end
		if d < 1000*1000 then
			return string.format("%0.2f", distance / 1000), lc.UNIT_KILOMETERS
		end
		if d < 1000*1000*1000 then
			return string.format("%0.2f", distance / 1000 / 1000), lc.UNIT_MILLION_METERS
		end
		return string.format("%0.2f", distance / 1.4960e11), lc.UNIT_AU
	end,
	Speed = function(distance)
		local d = math.abs(distance)
		if d < 1000 then
			return math.floor(distance), lc.UNIT_METERS_PER_SECOND
		end
		if d < 1000*1000 then
			return string.format("%0.2f", distance / 1000), lc.UNIT_KILOMETERS_PER_SECOND
		end
		return string.format("%0.2f", distance / 1000 / 1000), lc.UNIT_MILLION_METERS_PER_SECOND
		-- no need for au/s
	end,
}

ui.pointOnClock = function(center, radius, hours)
	-- 0 hours is top, going rightwards, negative goes leftwards
	local a = math.fmod(hours / 12 * two_pi, two_pi)
	local p = Vector(0, -radius)
	return Vector(center.x, center.y) + Vector(p.x * math.cos(a) - p.y * math.sin(a), p.y * math.cos(a) + p.x * math.sin(a))
end

-- font sizes are correct for 1920x1200
local font_factor = pigui.screen_height / 1200.0
ui.fonts = {
	-- dummy font, actually renders icons
	pionicons = {
		small = { name = "icons", size = 16 * font_factor, offset = 14 * font_factor},
		large = { name = "icons", size = 22 * font_factor, offset = 28 * font_factor}
	},
	pionillium = {
		large = { name = "pionillium", size = 30 * font_factor, offset = 24 * font_factor},
		medium = { name = "pionillium", size = 18 * font_factor, offset = 14 * font_factor},
		-- 		medsmall = { name = "pionillium", size = 15, offset = 12 },
		small = { name = "pionillium", size = 12 * font_factor, offset = 10 * font_factor},
		tiny = { name = "pionillium", size = 8 * font_factor, offset = 7 * font_factor},
	}
}

ui.anchor = { left = 1, right = 2, center = 3, top = 4, bottom = 5, baseline = 6 }

ui.calcTextAlignment = function(pos, size, anchor_horizontal, anchor_vertical)
	local position = Vector(pos.x, pos.y)
	if anchor_horizontal == ui.anchor.left or anchor_horizontal == nil then
	  position.x = position.x -- do nothing
	elseif anchor_horizontal == ui.anchor.right then
	  position.x = position.x - size.x
	elseif anchor_horizontal == ui.anchor.center then
	  position.x = position.x - size.x/2
	else
	  error("show_text: incorrect horizontal anchor " .. anchor_horizontal)
	end
	if anchor_vertical == ui.anchor.top or anchor_vertical == nil then
	  position.y = position.y -- do nothing
	elseif anchor_vertical == ui.anchor.center then
	  position.y = position.y - size.y/2
	elseif anchor_vertical == ui.anchor.bottom then
	  position.y = position.y - size.y
	else
	  error("show_text: incorrect vertical anchor " .. anchor_vertical)
	end
	return position
end

ui.addIcon = function(position, icon, color, size, anchor_horizontal, anchor_vertical, tooltip, angle_rad)
	local pos = ui.calcTextAlignment(position, Vector(size, size), anchor_horizontal, anchor_vertical)
	local uv0, uv1 = get_icon_tex_coords(icon)
	if angle_rad then
	  local center = (pos + pos + Vector(size,size)) / 2
	  local up_left = Vector(-size/2, size/2):rotate2d(angle_rad)
	  local up_right = up_left:right()
	  local down_left = up_left:left()
	  local down_right = -up_left
	  pigui.AddImageQuad(ui.icons_texture, center + up_left, center + up_right, center + down_right, center + down_left, uv0, Vector(uv1.x, uv0.y), uv1, Vector(uv0.x, uv1.y), color)
	else
	  pigui.AddImage(ui.icons_texture, pos, pos + Vector(size, size), uv0, uv1, color)
	end
	if tooltip and not pigui.IsMouseHoveringAnyWindow() and tooltip ~= "" then
	  if pigui.IsMouseHoveringRect(pos, pos + size, true) then
			maybeSetTooltip(tooltip)
	  end
	end

	return Vector(size, size)
end

ui.addFancyText = function(position, anchor_horizontal, anchor_vertical, data, bg_color)
	-- always align texts at baseline
	local spacing = 2
	local size = Vector(0, 0)
	local max_offset = 0
	for i=1,#data do
		local item = data[i]
		assert(item.text, "missing text for item in addFancyText")
		assert(item.font, "missing font for item in addFancyText")
		assert(item.color, "missing font for item in addFancyText")

	  local is_icon = item.font.name ~= "icons"
	  local s
		local popfont
	  if is_icon then
			popfont = pigui:PushFont(item.font.name, item.font.size)
			s = pigui.CalcTextSize(item.text)
	  else
			s = Vector(item.font.size, item.font.size)
	  end
	  size.x = size.x + s.x
	  size.x = size.x + spacing -- spacing
	  size.y = math.max(size.y, s.y)
	  max_offset = math.max(max_offset, item.font.offset)
	  if is_icon then
			if popfont then
				pigui.PopFont()
			end
	  end
	end
	size.x = size.x - spacing -- remove last spacing
	position = ui.calcTextAlignment(position, size, anchor_horizontal, nil)
	if anchor_vertical == ui.anchor.top then
	  position.y = position.y + max_offset
	elseif anchor_vertical == ui.anchor.bottom then
	  position.y = position.y - (size.y - max_offset)
	end
	if bg_color then
		pigui.AddRectFilled(position - Vector(textBackgroundMarginPixels, size.y + textBackgroundMarginPixels),
												position + Vector(size.x + textBackgroundMarginPixels, textBackgroundMarginPixels),
												bg_color,
												0,
												0)
	end
	for i=1,#data do
		local item = data[i]
	  local is_icon = item.font.name ~= "icons"
	  if is_icon then
			ui.withFont(item.font.name, item.font.size, function()
										local s = ui.addStyledText(position, ui.anchor.left, ui.anchor.baseline, item.text, item.color, item.font, item.tooltip)
										position.x = position.x + s.x + spacing
			end)
	  else
			local s = ui.addIcon(position, item.text, item.color, item.font.size, ui.anchor.left, ui.anchor.bottom, item.tooltip)
			position.x = position.x + s.x + spacing
	  end
	end
	return size
end

ui.addStyledText = function(position, anchor_horizontal, anchor_vertical, text, color, font, tooltip, bg_color)
	-- addStyledText aligns to upper left
	local size
	ui.withFont(font.name, font.size, function()
								size = pigui.CalcTextSize(text)
								local vert
								if anchor_vertical == ui.anchor.baseline then
									vert = nil
								else
									vert = anchor_vertical
								end
								position = ui.calcTextAlignment(position, size, anchor_horizontal, vert) -- ignore vertical if baseline
								if anchor_vertical == ui.anchor.baseline then
									position.y = position.y - font.offset
								end
								if bg_color then
									pigui.AddRectFilled(position - textBackgroundMarginPixels, position + size + textBackgroundMarginPixels, bg_color, 0, 0)
								end
								pigui.AddText(position, color, text)
								-- pigui.AddQuad(position, position + Vector(size.x, 0), position + Vector(size.x, size.y), position + Vector(0, size.y), colors.red, 1.0)
	end)
	if tooltip and not pigui.IsMouseHoveringAnyWindow() and tooltip ~= "" then
	  if pigui.IsMouseHoveringRect(position, position + size, true) then
			maybeSetTooltip(tooltip)
	  end
	end
	return Vector(size.x, size.y)
end

ui.icon = function(icon, size, color)
	local uv0, uv1 = get_icon_tex_coords(icon)
	pigui.Image(ui.icons_texture, size, uv0, uv1, color)
end

-- Forward selected functions
ui.screenWidth = pigui.screen_width
ui.screenHeight = pigui.screen_height
ui.setNextWindowPos = pigui.SetNextWindowPos
ui.setNextWindowSize = pigui.SetNextWindowSize
ui.dummy = pigui.Dummy
ui.sameLine = pigui.SameLine
ui.text = pigui.Text
ui.selectable = pigui.Selectable
ui.progressBar = pigui.ProgressBar
ui.calcTextSize = pigui.CalcTextSize
ui.addCircle = pigui.AddCircle
ui.addCircleFilled = pigui.AddCircleFilled
ui.addRect = pigui.AddRect
ui.addRectFilled = pigui.AddRectFilled
ui.addLine = pigui.AddLine
ui.pathArcTo = pigui.PathArcTo
ui.pathStroke = pigui.PathStroke
ui.twoPi = two_pi
ui.pi_2 = pi_2
ui.pi_4 = pi_4
ui.pi = pi
ui.imageButton = function(icon, size, frame_padding, bg_color, tint_color, tooltip)
	local uv0, uv1 = get_icon_tex_coords(icon)
	pigui.PushID(tooltip)
	local res = pigui.ImageButton(ui.icons_texture, size, uv0, uv1, frame_padding, bg_color, tint_color)
	pigui.PopID()
	return res
end
ui.oneOverSqrtTwo = one_over_sqrt_two
ui.isMouseClicked = pigui.IsMouseClicked
ui.getMousePos = pigui.GetMousePos
ui.getMouseWheel = pigui.GetMouseWheel
ui.setTooltip = maybeSetTooltip
ui.shouldDrawUI = pigui.ShouldDrawUI
ui.getWindowPos = pigui.GetWindowPos
ui.getTargetsNearby = pigui.GetTargetsNearby
ui.getProjectedBodies = pigui.GetProjectedBodies
ui.isMouseReleased = pigui.IsMouseReleased
ui.isMouseHoveringRect = pigui.IsMouseHoveringRect
ui.openPopup = pigui.OpenPopup
ui.shouldShowLabels = pigui.ShouldShowLabels
ui.keys = pigui.keys
ui.systemInfoViewNextPage = pigui.SystemInfoViewNextPage -- deprecated
ui.isKeyReleased = pigui.IsKeyReleased
ui.playSfx = pigui.PlaySfx
ui.ctrlHeld = function() return pigui.key_ctrl end
ui.coloredSelectedIconButton = function(icon, size, is_selected, frame_padding, bg_color, fg_color, tooltip)
	if is_selected then
		pigui.PushStyleColor("Button", bg_color)
		pigui.PushStyleColor("ButtonHovered", bg_color:tint(0.1))
		pigui.PushStyleColor("ButtonActive", bg_color:tint(0.2))
	else
		pigui.PushStyleColor("Button", bg_color:shade(0.6))
		pigui.PushStyleColor("ButtonHovered", bg_color:shade(0.4))
		pigui.PushStyleColor("ButtonActive", bg_color:shade(0.2))
	end
	local uv0,uv1 = get_icon_tex_coords(icon)
	pigui.PushID(tooltip)
	local res = pigui.ImageButton(ui.icons_texture, size, uv0, uv1, frame_padding, ui.theme.colors.lightBlueBackground, fg_color)
	pigui.PopID()
	pigui.PopStyleColor(3)
	if pigui.IsItemHovered() then
		pigui.SetTooltip(tooltip)
	end
	return res
end
ui.loadTextureFromSVG = function(a, b, c)
	return pigui:LoadTextureFromSVG(a, b, c)
end
ui.dataDirPath = pigui.DataDirPath
ui.addImage = pigui.AddImage
local defaultTheme = import("themes/default")
ui.theme = defaultTheme

local modules = {}
ui.registerModule = function(mode, fun)
	if not modules[mode] then
		modules[mode] = {}
	end
	table.insert(modules[mode], fun)
end

ui.getModules = function(mode)
	return modules[mode] or {}
end

return ui
