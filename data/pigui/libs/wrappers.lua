-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- Convenience wrappers for the C++ UI functions and general functions
local Engine = require 'Engine'
local Game = require 'Game'
local utils = require 'utils'
local pigui = Engine.pigui
local ui = require 'pigui.libs.forwarded'

--
-- Function: ui.rescaleUI
--
-- ui.rescaleUI(val, baseResolution, rescaleToScreenAspect, targetResolution)
--
-- Scales a set of values (normally a size or a position) based on a base
-- resolution and the current or target resultion.
--
--
-- Example:
--
-- > size = ui.rescaleUI(Vector2(96, 96), Vector2(1600, 900))
--
-- Parameters:
--
--   val                   - number|Vector2|Table, the values to scale
--   baseResolution        - Vector2, the resolution at which val is valid
--   rescaleToScreenAspect - (Optional) number, when scaling a Vector2, scale x and y
--                           appropriately to match the given aspect ratio
--   targetResolution      - (Optional) Vector2, the target resolution to scale
--                           the value to. Default: current screen resolution.
--
-- Returns:
--
--   number|Vector2|Table - the scaled value
--
function ui.rescaleUI(val, baseResolution, rescaleToScreenAspect, targetResolution)
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

--
-- Function: ui.pcall
--
-- ui.pcall(fun, ...)
--
-- Clean up the ImGui stack in case of an error
--
--
-- Example:
--
-- >
--
-- Parameters:
--
--   fun -
--   ... -
--
-- Returns:
--
--   nil
--
function ui.pcall(fun, ...)
	local stack = pigui.GetImguiStack()
	return xpcall(fun, function(msg)
		return msg .. pigui.CleanupImguiStack(stack)
	end, ...)
end

--
-- Function: ui.window
--
-- ui.window(name, params, fun)
--
-- Display a window
--
--
-- Example:
--
-- >
--
-- Parameters:
--
--   name   - String, a unique name for the window,
--            used to group its children
--   params - Table, window options:
--              - NoTitleBar                : Disable title-bar
--              - NoResize                  : Disable user resizing with the lower-right grip
--              - NoMove                    : Disable user moving the window
--              - NoScrollbar               : Disable scrollbars (window can still scroll with
--                                            mouse or programmatically)
--              - NoScrollWithMouse         : Disable user vertically scrolling with mouse wheel.
--                                            On child window, mouse wheel will be forwarded to
--                                            the parent unless NoScrollbar is also set.
--              - NoCollapse                : Disable user collapsing window by double-clicking
--                                            on it
--              - AlwaysAutoResize          : Resize every window to its content every frame
--              - NoSavedSettings           : Never load/save settings in .ini file
--              - NoInputs                  :
--              - MenuBar                   : Has a menu-bar
--              - HorizontalScrollbar       : Allow horizontal scrollbar to appear (off by default).
--              - NoFocusOnAppearing        : Disable taking focus when transitioning from hidden to
--                                            visible state
--              - NoBringToFrontOnFocus     : Disable bringing window to front when taking focus
--                                            (e.g. clicking on it or programmatically giving it
--                                            focus)
--              - AlwaysVerticalScrollbar   : Always show vertical scrollbar
--              - AlwaysHorizontalScrollbar : Always show horizontal scrollbar
--              - AlwaysUseWindowPadding    : Ensure child windows without border uses
--                                            style.WindowPadding (ignored by default for
--                                            non-bordered child windows, because more convenient)
--   fun    - Function, a function that is called to define the window contents
--
-- Returns:
--
--   nil
--
function ui.window(name, params, fun)
	local ok = pigui.Begin(name, params)
	if ok then fun() end
	pigui.End()
end

--
-- Function: ui.group
--
-- ui.group(fun)
--
-- Display items in a group
--
--
-- Example:
--
-- >
--
-- Parameters:
--
--   fun - Function, a function that is called to define the group contents
--
-- Returns:
--
--   nil
--
function ui.group(fun)
	pigui.BeginGroup()
	fun()
	pigui.EndGroup()
end

--
-- Function: ui.popup
--
-- ui.popup(name, params, fun)
--
-- Display a popup window
--
--
-- Example:
--
-- >
--
-- Parameters:
--
--   name - String, a unique name for the window,
--          used to group its children
--   fun  - Function, a function that is called to define the popup contents
--
-- Returns:
--
--   nil
--
function ui.popup(name, fun)
	if pigui.BeginPopup(name) then
		fun()
		pigui.EndPopup()
	end
end

--
-- Function: ui.customTooltip
--
-- ui.customTooltip(fun)
--
-- Display a tooltip window
--
--
-- Example:
--
-- >
--
-- Parameters:
--
--   fun  - Function, a function that is called to define the tooltip contents
--
-- Returns:
--
--   nil
--
function ui.customTooltip(fun)
	pigui.BeginTooltip()
	fun()
	pigui.EndTooltip()
end

--
-- Function: ui.child
--
-- ui.child(id, size, flags, fun)
--
-- Define a child window
--
--
-- Example:
--
-- >
--
-- Parameters:
--
--   id    - String, a unique name for the window,
--           used to group its children
--   size  - (Optional)Vector2
--   flags - (Optional)Table, options:
--              - NoTitleBar                : Disable title-bar
--              - NoResize                  : Disable user resizing with the lower-right grip
--              - NoMove                    : Disable user moving the window
--              - NoScrollbar               : Disable scrollbars (window can still scroll with
--                                            mouse or programmatically)
--              - NoScrollWithMouse         : Disable user vertically scrolling with mouse wheel.
--                                            On child window, mouse wheel will be forwarded to
--                                            the parent unless NoScrollbar is also set.
--              - NoCollapse                : Disable user collapsing window by double-clicking
--                                            on it
--              - AlwaysAutoResize          : Resize every window to its content every frame
--              - NoSavedSettings           : Never load/save settings in .ini file
--              - NoInputs                  :
--              - MenuBar                   : Has a menu-bar
--              - HorizontalScrollbar       : Allow horizontal scrollbar to appear (off by default).
--              - NoFocusOnAppearing        : Disable taking focus when transitioning from hidden to
--                                            visible state
--              - NoBringToFrontOnFocus     : Disable bringing window to front when taking focus
--                                            (e.g. clicking on it or programmatically giving it
--                                            focus)
--              - AlwaysVerticalScrollbar   : Always show vertical scrollbar
--              - AlwaysHorizontalScrollbar : Always show horizontal scrollbar
--              - AlwaysUseWindowPadding    : Ensure child windows without border uses
--                                            style.WindowPadding (ignored by default for
--                                            non-bordered child windows, because more convenient)
--   fun   - Function, a function that is called to define the popup contents
--
-- Returns:
--
--   nil
--
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

--
-- Function: ui.withTooltip
--
-- ui.withTooltip(tooltip, fun)
--
-- Display something, but with a tooltip shown on mouseover
--
--
-- Example:
--
-- >
--
-- Parameters:
--
--   tooltip - String, the tooltip to display
--   fun     - Function, a function that is called to display the contents
--             that will have the tooltip
--
-- Returns:
--
--   nil
--
function ui.withTooltip(tooltip, fun)
	local startPos = pigui.GetCursorPos()
	pigui.BeginGroup()
	fun()
	pigui.EndGroup()
	if string.len(tooltip) > 0 and pigui.IsItemHovered() then
		pigui.SetTooltip(tooltip)
	end
end

--
-- Function: ui.playBoinkNoise
--
-- ui.playBoinkNoise()
--
-- Boink!
--
-- Example:
--
-- > ui.playBoinkNoise()
--
-- Parameters:
--
-- Returns:
--
--   nil
--
function ui.playBoinkNoise()
	ui.playSfx("Click", 0.3)
end

--
-- Function: ui.isMouseHoveringWindow
--
-- ui.isMouseHoveringWindow()
--
--
-- Example:
--
-- >
--
-- Parameters:
--
-- Returns:
--
--   boolean - true if the mouse is currently within the current
--             window, false otherwise
--
function ui.isMouseHoveringWindow()
	return ui.isWindowHovered({"AllowWhenBlockedByPopup", "AllowWhenBlockedByActiveItem"})
end

--
-- Function: ui.isAnyWindowHovered
--
-- ui.isAnyWindowHovered()
--
--
-- Example:
--
-- >
--
-- Parameters:
--
-- Returns:
--
--   boolean - true if the mouse is currently within any window,
--             false otherwise
--
function ui.isAnyWindowHovered()
	return ui.isWindowHovered({"AnyWindow"})
end

--
-- Function: ui.ctrlHeld
--
-- ui.ctrlHeld()
--
--
-- Example:
--
-- >
--
-- Parameters:
--
-- Returns:
--
--   boolean - true if a ctrl key is being held
--
function ui.ctrlHeld() return pigui.key_ctrl end

--
-- Function: ui.altHeld
--
-- ui.altHeld()
--
--
-- Example:
--
-- >
--
-- Parameters:
--
-- Returns:
--
--   boolean - true if an alt key is being held
--
function ui.altHeld() return pigui.key_alt end

--
-- Function: ui.shiftHeld
--
-- ui.shiftHeld()
--
--
-- Example:
--
-- >
--
-- Parameters:
--
-- Returns:
--
--   boolean - true if a shift key is being held
--
function ui.shiftHeld() return pigui.key_shift end

--
-- Function: ui.noModifierHeld
--
-- ui.noModifierHeld()
--
--
-- Example:
--
-- >
--
-- Parameters:
--
-- Returns:
--
--   boolean - true if no modifier (alt, shift, ctrl) keys are being held
--
function ui.noModifierHeld() return pigui.key_none end

--
-- Function: ui.tabBar
--
-- ui.tabBar(id, items)
--
--
-- Example:
--
-- >
--
-- Parameters:
--   id    - String, unique id to identify the group of tabs by
--   items - Table, a list of contents. Each item should be a
--           table containing a table of tab options and a function
--           to call that displays the tabs contents
--
-- Returns:
--
--   boolean - true if the tab bar is open, false otherwise
--
function ui.tabBar(id, items)
	local open = pigui.BeginTabBar(id)
	if not open then return false end

	for i, v in ipairs(items) do
		if type(v) == "table" and v[1] and type(v[2]) == "function" then
			if pigui.BeginTabItem(tostring(v[1]) .. "##" .. tostring(i)) then
				v[2](v)
			end
		end
	end

	pigui.EndTabBar()
	return true
end

--
-- Function: ui.withFont
--
-- ui.withFont(name, size, fun)
--
--
-- Example:
--
-- >
--
-- Parameters:
--   name    - Table|String, a table defining the font name and size or
--             a string containing the name of the font
--   size    - (Optional) number, font size. Optional if name is a table and defines size
--   fun     - function, a function to call that shows the contents with the defined font
--
-- Returns:
--
--   any - the value returned from fun
--
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

--
-- Function: ui.withStyleColors
--
-- ui.withStyleColors(styles, fun)
--
-- Display UI content with defined colors
--
-- Example:
--
-- >
--
-- Parameters:
--   styles - table, table of style elements with the desired colors:
--              Text, TextDisabled, WindowBg, ChildWindowBg, PopupBg, Border,
--              BorderShadow, FrameBg, FrameBgHovered, FrameBgActive,TitleBg,
--              TitleBgCollapsed, TitleBgActive, MenuBarBg, ScrollbarBg,
--              ScrollbarGrab, ScrollbarGrabHovered, ScrollbarGrabActive,
--              CheckMark, SliderGrab, SliderGrabActive, Button,
--              ButtonHovered, ButtonActive, Header, HeaderHovered,
--              HeaderActive, Separator, SeparatorHovered, SeparatorActive,
--              ResizeGrip, ResizeGripHovered, ResizeGripActive, PlotLines,
--              PlotLinesHovered, PlotHistogram, PlotHistogramHovered,
--              TextSelectedBg, ModalWindowDarkening
--   fun    - function, a function to call that shows the contents with the defined font
--
-- Returns:
--
--   any - the value returned from fun
--
function ui.withStyleColors(styles, fun)
	for k,v in pairs(styles) do
		pigui.PushStyleColor(k, v)
	end
	local res = fun()
	pigui.PopStyleColor(utils.count(styles))
	return res
end

--
-- Function: ui.withStyleVars
--
-- ui.withStyleVars(styles, fun)
--
-- Display UI content with defined styles
--
-- Example:
--
-- >
--
-- Parameters:
--   vars - table, table of style elements with the desired values:
--            Alpha, WindowPadding, WindowRounding, WindowBorderSize,
--            WindowMinSize, ChildRounding, ChildBorderSize, FramePadding,
--            FrameRounding, FrameBorderSize, ItemSpacing,
--            ItemInnerSpacing, IndentSpacing, GrabMinSize,
--            ButtonTextAlign
--   fun  - function, a function to call that shows the contents with the defined font
--
-- Returns:
--
--   any - the value returned from fun
--
function ui.withStyleVars(vars, fun)
	for k,v in pairs(vars) do
		pigui.PushStyleVar(k, v)
	end
	local res = fun()
	pigui.PopStyleVar(utils.count(vars))
	return res
end

--
-- Function: ui.withStyleColorsAndVars
--
-- ui.withStyleColorsAndVars(styles, vars, fun)
--
-- Display UI content with defined styles and colors
--
-- Example:
--
-- >
--
-- Parameters:
--   styles - table, table of style elements with the desired colors (see ui.withStyleColors)
--   vars   - table, table of style elements with the desired values (see ui.withStyleVars)
--   fun    - function, a function to call that shows the contents with the defined font
--
-- Returns:
--
--   any - the value returned from fun
--
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

--
-- Function: ui.screenSize
--
-- ui.screenSize()
--
-- Return the current screen resolution as a Vector2
--
-- Example:
--
-- >
--
-- Parameters:
--
-- Returns:
--
--   Vector2 - screen width and height
--
function ui.screenSize()
	return Vector2(ui.screenWidth, ui.screenHeight)
end

--
-- Function: ui.setNextWindowPosCenter
--
-- ui.setNextWindowPosCenter(cond)
--
-- Set the next window position to be centered on screen
--
-- Example:
--
-- >
--
-- Parameters:
--   cond - table, condition flags: Always, Once, FirstUseEver, Appearing
--
-- Returns:
--
--   nil
--
function ui.setNextWindowPosCenter(cond)
	ui.setNextWindowPos(ui.screenSize() / 2, cond, Vector2(0.5, 0.5))
end

--
-- Function: ui.sameLine
--
-- ui.sameLine(pos_x, spacing_w)
--
-- Draw the next command on the same line as the previous
--
-- Example:
--
-- >
--
-- Parameters:
--   pos_x     - (Optional) number, X position for next draw command, default 0
--   spacing_w - (Optional) number, draw with spacing relative to previous, default -1
--
-- Returns:
--
--   nil
--
function ui.sameLine(pos_x, spacing_w)
	local px = pos_x or 0.0
	local sw = spacing_w or -1.0
	pigui.SameLine(px, sw)
end

--
-- Function: ui.withID
--
-- ui.withID(id, fun)
--
-- Display content with a specified ID
--
-- Example:
--
-- >
--
-- Parameters:
--   id  - string, the desired ID
--   fun - function, function called to display content
--
-- Returns:
--
--   nil
--
function ui.withID(id, fun)
	pigui.PushID(id)
	fun()
	pigui.PopID()
end

--
-- Function: ui.loadTextureFromSVG
--
-- ui.loadTextureFromSVG(filename, width, height)
--
-- Create a texture from an SVG
--
-- Example:
--
-- >
--
-- Parameters:
--   filename - string, svg path
--   width    - number, width of texture to create
--   height   - number, height of texture to create
--
-- Returns:
--
--   userdata - the texture
--
function ui.loadTextureFromSVG(filename, width, height)
	return pigui:LoadTextureFromSVG(filename, width, height)
end

--
-- Function: ui.loadTexture
--
-- ui.loadTexture(filename)
--
-- Load a texture from file
--
-- Example:
--
-- >
--
-- Parameters:
--   filename - string, texture file path
--
-- Returns:
--
--   userdata - the texture
--
function ui.loadTexture(filename)
	return pigui:LoadTexture(filename)
end

function ui.maybeSetTooltip(tooltip)
	if not Game.player:IsMouseActive() then
		pigui.SetTooltip(tooltip)
	end
end

ui.setTooltip = ui.maybeSetTooltip
