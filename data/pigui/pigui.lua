-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- TODO: don't move pointer in radial menu

local Format = require 'Format'
local Game = require 'Game'
local Player = require 'Player'
local Space = require 'Space'
local Engine = require 'Engine'
local Event = require 'Event'
local ShipDef = require 'ShipDef'
local Lang = require 'Lang'
local Vector2 = _G.Vector2

local lui = Lang.GetResource("ui-core");
local lc = Lang.GetResource("core");
local lec = Lang.GetResource("equipment-core");

local utils = require 'utils'
local pigui = Engine.pigui

local pi = 3.14159264
local pi_2 = pi / 2
local pi_4 = pi / 4
local two_pi = pi * 2
local standard_gravity = 9.80665
local one_over_sqrt_two = 1 / math.sqrt(2)

local ui = { }

local defaultTheme = require '.themes.default'
ui.theme = defaultTheme

ui.rescaleUI = function(val, baseResolution, rescaleToScreenAspect, targetResolution)
	if not targetResolution then
		targetResolution = Vector2(pigui.screen_width, pigui.screen_height)
	end

	local rescaleVector = Vector2(targetResolution.x / baseResolution.x, targetResolution.y / baseResolution.y)
	local rescaleFactor = math.min(rescaleVector.x, rescaleVector.y)
	local type = type(val)

	if type == 'table' then
		local result = {}
		for k, v in pairs(val) do
			result[k] = ui.rescaleUI(v, baseResolution, rescaleToScreenAspect, targetResolution)
		end

		return result
	elseif type == 'userdata' and val.x and val.y then
		return Vector2(val.x * ((rescaleToScreenAspect and rescaleVector.x) or rescaleFactor), val.y * ((rescaleToScreenAspect and rescaleVector.y) or rescaleFactor))
	elseif type == 'number' then
		return val * rescaleFactor
	end
end

-- font sizes are correct for 1920x1200
local font_factor = ui.rescaleUI(1, Vector2(1920, 1200))

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

ui.anchor = { left = 1, right = 2, center = 3, top = 4, bottom = 5, baseline = 6 }

local function maybeSetTooltip(tooltip)
	if not Game.player:IsMouseActive() then
		pigui.SetTooltip(tooltip)
	end
end

local textBackgroundMarginPixels = 2

ui.icons_texture = pigui:LoadTextureFromSVG(pigui.DataDirPath({"icons", "icons.svg"}), 16 * 64, 16 * 64)

-- Clean up the ImGui stack in case of an error
function ui.pcall(fun, ...)
	local stack = pigui.GetImguiStack()
	return xpcall(fun, function(msg)
		return msg .. pigui.CleanupImguiStack(stack)
	end, ...)
end

function ui.window(name, params, fun)
	local ok = pigui.Begin(name, params)
	if ok then fun() end
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

function ui.child(id, size, flags, fun)
	if flags == nil and fun == nil then -- size is optional
		fun = size
		size = Vector2(-1,-1)
		flags = {}
	elseif fun == nil then
		fun = flags
		flags = {}
	end

	pigui.BeginChild(id, size, flags)
	fun()
	pigui.EndChild()
end
function ui.withFont(name, size, fun)
	-- allow `withFont(fontObj, fun)`
	if type(name) == "table" and type(size) == "function" then
		name, size, fun = table.unpack{name.name, name.size, size}
	end

	local font = pigui:PushFont(name, size)
	local res = fun()
	if font then
		pigui.PopFont()
	end
	return res
end

function ui.withStyleColors(styles, fun)
	for k,v in pairs(styles) do
		pigui.PushStyleColor(k, v)
	end
	local res = fun()
	pigui.PopStyleColor(utils.count(styles))
	return res
end

function ui.withStyleVars(vars, fun)
	for k,v in pairs(vars) do
		pigui.PushStyleVar(k, v)
	end
	local res = fun()
	pigui.PopStyleVar(utils.count(vars))
	return res
end

function ui.withStyleColorsAndVars(styles, vars, fun)
	for k,v in pairs(styles) do
		pigui.PushStyleColor(k, v)
	end
	for k,v in pairs(vars) do
		pigui.PushStyleVar(k, v)
	end
	local res = fun()
	pigui.PopStyleVar(utils.count(vars))
	pigui.PopStyleColor(utils.count(styles))
	return res
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
	return Vector2(rem / count, quot/count), Vector2((rem+1) / count, (quot+1)/count)
end

local function get_wide_icon_tex_coords(icon)
	assert(icon, "no icon given")
	local count = 16.0 -- icons per row/column
	local rem = math.floor(icon % count)
	local quot = math.floor(icon / count)
	return Vector2(rem / count, quot/count), Vector2((rem+2) / count, (quot+1)/count)
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

ui.addIcon = function(position, icon, color, size, anchor_horizontal, anchor_vertical, tooltip, angle_rad)
	local pos = ui.calcTextAlignment(position, size, anchor_horizontal, anchor_vertical)
	local uv0, uv1 = get_icon_tex_coords(icon)
	if angle_rad then
	  local center = Vector2(pos.x + pos.x + size.x, pos.y + pos.y + size.y) / 2
	  local up_left = Vector2(-size.x/2, size.y/2):rotate(angle_rad)
	  local up_right = up_left:right()
	  local down_left = up_left:left()
	  local down_right = -up_left
	  pigui.AddImageQuad(ui.icons_texture, center + up_left, center + up_right, center + down_right, center + down_left, uv0, Vector2(uv1.x, uv0.y), uv1, Vector2(uv0.x, uv1.y), color)
	else
	  pigui.AddImage(ui.icons_texture, pos, pos + size, uv0, uv1, color)
	end
	if tooltip and (ui.isMouseHoveringWindow() or not ui.isAnyWindowHovered()) and tooltip ~= "" then
	  if pigui.IsMouseHoveringRect(pos, pos + size, true) then
			maybeSetTooltip(tooltip)
	  end
	end

	return size
end

ui.addWideIcon = function(position, icon, color, size, anchor_horizontal, anchor_vertical, tooltip, angle_rad)
	local pos = ui.calcTextAlignment(position, size, anchor_horizontal, anchor_vertical)
	local uv0, uv1 = get_wide_icon_tex_coords(icon)
	if angle_rad then
	  local center = (pos + pos + size) / 2
	  local up_left = Vector2(-size.x/2, size.y/2):rotate2d(angle_rad)
	  local up_right = up_left:right()
	  local down_left = up_left:left()
	  local down_right = -up_left
	  pigui.AddImageQuad(ui.icons_texture, center + up_left, center + up_right, center + down_right, center + down_left, uv0, Vector2(uv1.x, uv0.y), uv1, Vector2(uv0.x, uv1.y), color)
	else
	  pigui.AddImage(ui.icons_texture, pos, pos + size, uv0, uv1, color)
	end
	if tooltip and (ui.isMouseHoveringWindow() or not is.isAnyWindowHovered()) and tooltip ~= "" then
	  if pigui.IsMouseHoveringRect(pos, pos + size, true) then
			maybeSetTooltip(tooltip)
	  end
	end

	return size
end

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
			maybeSetTooltip(tooltip)
	  end
	end
	return size
end

ui.icon = function(icon, size, color, tooltip)
	local uv0, uv1 = get_icon_tex_coords(icon)
	pigui.Image(ui.icons_texture, size, uv0, uv1, color)
	if tooltip and ui.isItemHovered() then
		ui.setTooltip(tooltip)
	end
end

-- Forward selected functions
ui.calcTextAlignment = pigui.CalcTextAlignment
ui.lineOnClock = pigui.lineOnClock
ui.pointOnClock = pigui.pointOnClock
ui.screenWidth = pigui.screen_width
ui.screenHeight = pigui.screen_height
ui.screenSize = function()
	return Vector2(ui.screenWidth, ui.screenHeight)
end
ui.setNextWindowPos = pigui.SetNextWindowPos
ui.setNextWindowPosCenter = function(cond)
	ui.setNextWindowPos(ui.screenSize() / 2, cond, Vector2(0.5, 0.5))
end
ui.setNextWindowSize = pigui.SetNextWindowSize
ui.setNextWindowSizeConstraints = pigui.SetNextWindowSizeConstraints
ui.dummy = pigui.Dummy
ui.sameLine = function(pos_x, spacing_w)
	local px = pos_x or 0.0
	local sw = spacing_w or -1.0
	pigui.SameLine(px, sw)
end
ui.spacing = pigui.Spacing
ui.text = pigui.Text
ui.combo = pigui.Combo
ui.listBox = pigui.ListBox
ui.textWrapped = pigui.TextWrapped
ui.textColored = pigui.TextColored
ui.inputText = pigui.InputText
ui.checkbox = pigui.Checkbox
ui.separator = pigui.Separator
ui.pushTextWrapPos = pigui.PushTextWrapPos
ui.popTextWrapPos = pigui.PopTextWrapPos
ui.setScrollHere = pigui.SetScrollHere
ui.selectable = pigui.Selectable
ui.progressBar = pigui.ProgressBar
ui.plotHistogram = pigui.PlotHistogram
ui.calcTextSize = pigui.CalcTextSize
ui.addCircle = pigui.AddCircle
ui.addCircleFilled = pigui.AddCircleFilled
ui.addRect = pigui.AddRect
ui.addRectFilled = pigui.AddRectFilled
ui.addLine = pigui.AddLine
ui.addText = pigui.AddText
ui.pathArcTo = pigui.PathArcTo
ui.pathStroke = pigui.PathStroke
ui.twoPi = two_pi
ui.pi_2 = pi_2
ui.pi_4 = pi_4
ui.pi = pi
ui.withID = function(id, fun)
		pigui.PushID(id)
		fun()
		pigui.PopID()
end
ui.imageButton = function(icon, size, frame_padding, bg_color, tint_color, tooltip)
	local uv0, uv1 = get_icon_tex_coords(icon)
	ui.withID(tooltip, function()
								 local res = pigui.ImageButton(ui.icons_texture, size, uv0, uv1, frame_padding, bg_color, tint_color)
	end)
	return res
end
ui.setCursorPos = pigui.SetCursorPos
ui.getCursorPos = pigui.GetCursorPos
ui.setCursorScreenPos = pigui.SetCursorScreenPos
ui.getCursorScreenPos = pigui.GetCursorScreenPos
ui.getTextLineHeight = pigui.GetTextLineHeight
ui.getTextLineHeightWithSpacing = pigui.GetTextLineHeightWithSpacing
ui.lowThrustButton = pigui.LowThrustButton
ui.thrustIndicator = pigui.ThrustIndicator
ui.oneOverSqrtTwo = one_over_sqrt_two
ui.isMouseClicked = pigui.IsMouseClicked
ui.isMouseDown = pigui.IsMouseDown
ui.getMousePos = pigui.GetMousePos
ui.getMouseWheel = pigui.GetMouseWheel
ui.setTooltip = maybeSetTooltip
ui.shouldDrawUI = pigui.ShouldDrawUI
ui.getWindowPos = pigui.GetWindowPos
ui.getWindowSize = pigui.GetWindowSize
-- available content region
ui.getContentRegion = pigui.GetContentRegion
ui.getTextLineHeight = pigui.GetTextLineHeight
ui.getTextLineHeightWithSpacing = pigui.GetTextLineHeightWithSpacing
ui.getFrameHeight = pigui.GetFrameHeight
ui.getFrameHeightWithSpacing = pigui.GetFrameHeightWithSpacing
ui.getTargetsNearby = pigui.GetTargetsNearby
ui.getProjectedBodies = pigui.GetProjectedBodies
ui.getProjectedBodiesGrouped = pigui.GetProjectedBodiesGrouped
ui.isMouseReleased = pigui.IsMouseReleased
ui.isMouseHoveringRect = pigui.IsMouseHoveringRect
ui.isMouseHoveringWindow = function()
	return ui.isWindowHovered({"AllowWhenBlockedByPopup", "AllowWhenBlockedByActiveItem"})
end
ui.isWindowHovered = pigui.IsWindowHovered
ui.isAnyWindowHovered = function()
	return ui.isWindowHovered({"AnyWindow"})
end
ui.collapsingHeader = pigui.CollapsingHeader
ui.beginPopupModal = pigui.BeginPopupModal
ui.endPopup = pigui.EndPopup
ui.openPopup = pigui.OpenPopup
ui.closeCurrentPopup = pigui.CloseCurrentPopup
ui.shouldShowLabels = pigui.ShouldShowLabels
ui.columns = pigui.Columns
ui.nextColumn = pigui.NextColumn
ui.setColumnOffset = pigui.SetColumnOffset
ui.getColumnWidth = pigui.GetColumnWidth
ui.setColumnWidth = pigui.SetColumnWidth
ui.getScrollY = pigui.GetScrollY
ui.keys = pigui.keys
ui.systemInfoViewNextPage = pigui.SystemInfoViewNextPage -- deprecated
ui.isKeyReleased = pigui.IsKeyReleased
ui.playSfx = pigui.PlaySfx
ui.isItemHovered = pigui.IsItemHovered
ui.isItemActive = pigui.IsItemActive
ui.isItemClicked = pigui.IsItemClicked
ui.ctrlHeld = function() return pigui.key_ctrl end
ui.altHeld = function() return pigui.key_alt end
ui.shiftHeld = function() return pigui.key_shift end
ui.noModifierHeld = function() return pigui.key_none end
ui.vSliderInt = pigui.VSliderInt
ui.sliderInt = pigui.SliderInt
ui.pushItemWidth = pigui.PushItemWidth
ui.popItemWidth = pigui.PopItemWidth
ui.sliderFloat = pigui.SliderFloat

-- Flag validation functions. Call with a table of string flags as the only argument.
ui.SelectableFlags = pigui.SelectableFlags
ui.TreeNodeFlags = pigui.TreeNodeFlags
ui.InputTextFlags = pigui.InputTextFlags
ui.WindowFlags = pigui.WindowFlags
ui.HoveredFlags = pigui.HoveredFlags

-- FINALLY OUT OF Pi.cpp! BEGONE!
ui.playBoinkNoise = function ()
	ui.playSfx("Click", 0.3, 0.3)
end

local shouldShowRadialMenu = false
local radialMenuPos = Vector2(0,0)
local radialMenuSize = 10
local radialMenuTarget = nil
local radialMenuMouseButton = 1
local radialMenuActions = {}
local radialMenuMousePos = nil
ui.openRadialMenu = function(target, mouse_button, size, actions)
	ui.openPopup("##radialmenupopup")
	shouldShowRadialMenu = true
	radialMenuTarget = target
	radialMenuPos = ui.getMousePos()
	radialMenuSize = size
	radialMenuMouseButton = mouse_button
	radialMenuActions = actions
	radialMenuMousePos = ui.getMousePos()
	-- move away from screen edge
	radialMenuPos.x = math.min(math.max(radialMenuPos.x, size*3), ui.screenWidth - size*3)
	radialMenuPos.y = math.min(math.max(radialMenuPos.y, size*3), ui.screenHeight - size*3)
end

-- TODO: add cloud Lang::SET_HYPERSPACE_TARGET_TO_FOLLOW_THIS_DEPARTURE
local radial_menu_actions_station = {
	{icon=ui.theme.icons.comms, tooltip=lc.REQUEST_DOCKING_CLEARANCE,
	 action=function(target)
			local msg = Game.player:RequestDockingClearance(target)
		 	Game.AddCommsLogLine(msg, target.label)
		 	Game.player:SetNavTarget(target)
	end},
	{icon=ui.theme.icons.autopilot_dock, tooltip=lc.AUTOPILOT_DOCK_WITH_STATION,
	 action=function(target)
	 		if next(Game.player:GetEquip('autopilot')) ~= nil then
		 		Game.player:SetFlightControlState("CONTROL_AUTOPILOT")
		 		Game.player:AIDockWith(target)
		 		Game.player:SetNavTarget(target)
			else
				Game.AddCommsLogLine(lc.NO_AUTOPILOT_INSTALLED)
			end
	end},
}

local radial_menu_actions_all_bodies = {
	{icon=ui.theme.icons.autopilot_fly_to, tooltip=lc.AUTOPILOT_FLY_TO_VICINITY_OF,
	 action=function(target)
		if next(Game.player:GetEquip('autopilot')) ~= nil then
		 	Game.player:SetFlightControlState("CONTROL_AUTOPILOT")
		 	Game.player:AIFlyTo(target)
		 	Game.player:SetNavTarget(target)
		else
			Game.AddCommsLogLine(lc.NO_AUTOPILOT_INSTALLED)
		end

	end},
}

local radial_menu_actions_systembody = {
	{icon=ui.theme.icons.autopilot_low_orbit, tooltip=lc.AUTOPILOT_ENTER_LOW_ORBIT_AROUND,
	 action=function(target)
	 		if next(Game.player:GetEquip('autopilot')) ~= nil then
		 		Game.player:SetFlightControlState("CONTROL_AUTOPILOT")
		 		Game.player:AIEnterLowOrbit(target)
		 		Game.player:SetNavTarget(target)
		 	else
				Game.AddCommsLogLine(lc.NO_AUTOPILOT_INSTALLED)
			end
	end},
	{icon=ui.theme.icons.autopilot_medium_orbit, tooltip=lc.AUTOPILOT_ENTER_MEDIUM_ORBIT_AROUND,
	 action=function(target)
	 		if next(Game.player:GetEquip('autopilot')) ~= nil then
		 		Game.player:SetFlightControlState("CONTROL_AUTOPILOT")
		 		Game.player:AIEnterMediumOrbit(target)
		 		Game.player:SetNavTarget(target)
		 	else
				Game.AddCommsLogLine(lc.NO_AUTOPILOT_INSTALLED)
			end
	end},
	{icon=ui.theme.icons.autopilot_high_orbit, tooltip=lc.AUTOPILOT_ENTER_HIGH_ORBIT_AROUND,
	 action=function(target)
	 		if next(Game.player:GetEquip('autopilot')) ~= nil then
		 		Game.player:SetFlightControlState("CONTROL_AUTOPILOT")
		 		Game.player:AIEnterHighOrbit(target)
		 		Game.player:SetNavTarget(target)
		 	else
				Game.AddCommsLogLine(lc.NO_AUTOPILOT_INSTALLED)
			end
	end},
}

ui.openDefaultRadialMenu = function(body)
	if body then
		local actions = {}
		for _,v in pairs(radial_menu_actions_all_bodies) do
			table.insert(actions, v)
		end
		if body:IsStation() then
			for _,v in pairs(radial_menu_actions_station) do
				table.insert(actions, v)
			end
		elseif body:GetSystemBody() then
			for _,v in pairs(radial_menu_actions_systembody) do
				table.insert(actions, v)
			end
		end
		ui.openRadialMenu(body, 1, 30, actions)
	end
end

local radialMenuWasOpen = {}
ui.radialMenu = function(id)
	if not radialMenuActions or #radialMenuActions == 0 then
		return
 	end
	local icons = {}
	local tooltips = {}
	for _,action in pairs(radialMenuActions) do
		local uv0, uv1 = get_icon_tex_coords(action.icon)
		table.insert(icons, { id = ui.icons_texture, uv0 = uv0, uv1 = uv1 })
		-- TODO: don't just assume that radialMenuTarget is a Body
		table.insert(tooltips, string.interp(action.tooltip, { target = radialMenuTarget and radialMenuTarget.label or "UNKNOWN" }))
	end
	local n = pigui.RadialMenu(radialMenuPos, "##radialmenupopup", radialMenuMouseButton, icons, radialMenuSize, tooltips)
	if n == -1 then
		pigui.DisableMouseFacing(true)
		radialMenuWasOpen[id] = true
	elseif n >= 0 or n == -2 then
		radialMenuWasOpen[id] = false
		shouldShowRadialMenu = false
		local target = radialMenuTarget
		radialMenuTarget = nil
		pigui.DisableMouseFacing(false)
		-- hack, imgui lets the press go through, but eats the release, so Pi still thinks rmb is held
		pigui.SetMouseButtonState(3, false);
		-- ui.setMousePos(radialMenuMousePos)
		-- do this last, so it can theoretically open a new radial menu
		-- though we can't as no button is pressed that could be released :-/
		if n >= 0 then
			radialMenuActions[n+1].action(target)
		end
	end
	if n == -3 and radialMenuWasOpen[id] then
		pigui.SetMouseButtonState(3, false)
		pigui.DisableMouseFacing(false)
		radialMenuWasOpen[id] = false
	end
	return n
end
ui.button = pigui.Button
ui.coloredSelectedButton = function(label, thesize, is_selected, bg_color, tooltip, enabled)
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
ui.coloredSelectedIconButton = function(icon, thesize, is_selected, frame_padding, bg_color, fg_color, tooltip, img_size, extraID)
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
	pigui.PushID(tooltip .. (extraID or ""))
	local res = pigui.ButtonImageSized(ui.icons_texture, thesize, img_size or Vector2(0,0), uv0, uv1, frame_padding, ui.theme.colors.lightBlueBackground, fg_color)
	pigui.PopID()
	pigui.PopStyleColor(3)
	if pigui.IsItemHovered() then
		pigui.SetTooltip(tooltip)
	end
	return res
end

local gauge_show_percent = true
ui.gauge_height = 25
ui.gauge_width = 275

ui.gauge = function(position, value, unit, format, minimum, maximum, icon, color, tooltip, width, height, formatFont, percentFont)
	local percent = math.clamp((value - minimum) / (maximum - minimum), 0, 1)
	local offset = 60
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
		if value and format then
			ui.addFancyText(Vector2(uiPos.x + gauge_height/2, uiPos.y + gauge_height/4), ui.anchor.left, ui.anchor.center, {
				{ text=string.format(format, value), color=ui.theme.colors.reticuleCircle,     font=(formatFont or ui.fonts.pionillium.small), tooltip=tooltip },
				{ text=unit,                         color=ui.theme.colors.reticuleCircleDark, font=(formatFont or ui.fonts.pionillium.small), tooltip=tooltip }},
					ui.theme.colors.gaugeBackground)
		end
	end)
end

ui.loadTextureFromSVG = function(a, b, c)
	return pigui:LoadTextureFromSVG(a, b, c)
end

ui.loadTexture = function(filename)
	return pigui:LoadTexture(filename)
end

ui.dataDirPath = pigui.DataDirPath
ui.addImage = pigui.AddImage
ui.image = pigui.Image

local modules = {}

ui.registerModule = function(mode, fun)
	if not modules[mode] then
		modules[mode] = {}
	end
	table.insert(modules[mode], { fun = fun, enabled = true })
end

ui.getModules = function(mode)
	return modules[mode] or {}
end

ui.withTooltip = function(tooltip, fun)
	local startPos = pigui.GetCursorPos()
	fun()
	if string.len(tooltip) > 0 then
		local endPos = pigui.GetCursorPos()
		local offset = pigui.GetWindowPos()
		offset.y = offset.y - pigui.GetScrollY()
		if pigui.IsMouseHoveringRect(offset + startPos, offset + endPos, false) then
			pigui.SetTooltip(tooltip)
		end
	end
end

return ui
