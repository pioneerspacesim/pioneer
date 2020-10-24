-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt
local Engine = require 'Engine'
local Lang = require 'Lang'
local Game = require 'Game'
local ui = require 'pigui.baseui'
local pigui = Engine.pigui

local lc = Lang.GetResource("core");

local font_factor = ui.rescaleUI(1, Vector2(1920, 1200))

local textBackgroundMarginPixels = 2



ui.fonts = {
	-- dummy font, actually renders icons
	pionicons = {
		small = { name = "icons", size = 16 * font_factor, offset = 14 * font_factor},
		medium = { name = "icons", size = 18 * font_factor, offset = 20 * font_factor},
		large = { name = "icons", size = 22 * font_factor, offset = 28 * font_factor}
	},
	pionillium = {
		xlarge = { name = "pionillium", size = 36 * font_factor, offset = 24 * font_factor },
		large = { name = "pionillium", size = 30 * font_factor, offset = 24 * font_factor},
		medlarge = { name = "pionillium", size = 24 * font_factor, offset = 18 * font_factor},
		medium = { name = "pionillium", size = 18 * font_factor, offset = 14 * font_factor},
		-- 		medsmall = { name = "pionillium", size = 15, offset = 12 },
		small = { name = "pionillium", size = 14 * font_factor, offset = 11 * font_factor},
		tiny = { name = "pionillium", size = 8 * font_factor, offset = 7 * font_factor},
	},
	orbiteer = {
		xlarge = { name = "orbiteer", size = 36 * font_factor, offset = 24 * font_factor },
		large = { name = "orbiteer", size = 30 * font_factor, offset = 24 * font_factor },
		medlarge = { name = "orbiteer", size = 24 * font_factor, offset = 20 * font_factor},
		medium = { name = "orbiteer", size = 20 * font_factor, offset = 16 * font_factor},
		small = { name = "orbiteer", size = 14 * font_factor, offset = 11 * font_factor},
		tiny = { name = "orbiteer", size = 8 * font_factor, offset = 7 * font_factor},
	},
}

ui.Format = {
	Latitude = function(decimal_degrees)
		local prefix = lc.LATITUDE_NORTH_ABBREV
		if decimal_degrees < 0 then
			prefix = lc.LATITUDE_SOUTH_ABBREV
			decimal_degrees = math.abs(decimal_degrees)
		end
		local deg = math.floor(decimal_degrees)
		local min = (decimal_degrees - deg) * 60
		local sec = (min - math.floor(min)) * 60
		return string.format('%s %03i°%02i\'%02i"', prefix, deg, min, sec)
	end,
	Longitude = function(decimal_degrees)
		local prefix = lc.LONGITUDE_EAST_ABBREV
		if decimal_degrees < 0 then
			prefix = lc.LONGITUDE_WEST_ABBREV
			decimal_degrees = math.abs(decimal_degrees)
		end
		local deg = math.floor(decimal_degrees)
		local min = (decimal_degrees - deg) * 60
		local sec = (min - math.floor(min)) * 60
		return string.format('%s %03i°%02i\'%02i"', prefix, deg, min, sec)
	end,
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
	Datetime = function(date)
		local second, minute, hour, day, month, year = Game.GetPartsFromDateTime(date)
		return string.format("%4i-%02i-%02i %02i:%02i:%02i", year, month, day, hour, minute, second)
	end
}

ui.addFancyText = function(position, anchor_horizontal, anchor_vertical, data, bg_color)
	-- always align texts at baseline
	local spacing = 2
	local size = Vector2(0, 0)
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
			s = Vector2(item.font.size, item.font.size)
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
		position.y = position.y + size.y -- was max_offset, seems wrong
	elseif anchor_vertical == ui.anchor.bottom then
		position.y = position.y - size.y + max_offset
	end
	if bg_color then
		pigui.AddRectFilled(position - Vector2(textBackgroundMarginPixels, size.y + textBackgroundMarginPixels),
			position + Vector2(size.x + textBackgroundMarginPixels, textBackgroundMarginPixels),
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
			local s = ui.addIcon(position, item.text, item.color, Vector2(item.font.size, item.font.size), ui.anchor.left, ui.anchor.bottom, item.tooltip)
			position.x = position.x + s.x + spacing
		end
	end
	return size
end

ui.addStyledText = function(position, anchor_horizontal, anchor_vertical, text, color, font, tooltip, bg_color)
	-- addStyledText aligns to upper left
	local size = Vector2(0, 0)
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
			pigui.AddRectFilled(Vector2(position.x - textBackgroundMarginPixels, position.y - textBackgroundMarginPixels),
				Vector2(position.x + size.x + textBackgroundMarginPixels, position.y + size.y + textBackgroundMarginPixels),
				bg_color,
				0,
				0)
		end
		pigui.AddText(position, color, text)
		-- pigui.AddQuad(position, position + Vector2(size.x, 0), position + Vector2(size.x, size.y), position + vector.new(0, size.y), colors.red, 1.0)
	end)
	if tooltip and (ui.isMouseHoveringWindow() or not ui.isAnyWindowHovered()) and tooltip ~= "" then
		if pigui.IsMouseHoveringRect(position, position + size, true) then
			ui.maybeSetTooltip(tooltip)
		end
	end
	return size
end


