-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- Convenience wrappers for the C++ UI functions and general functions
local Engine = require 'Engine'
local Game = require 'Game'
local utils = require 'utils'
local pigui = Engine.pigui

---@class ui
local ui = require 'pigui.libs.forwarded'

--
-- Function: ui.pcall
--
-- Run a function in *protected mode* with the given arguments. Any error
-- inside the function will be caught and the ImGui stack cleaned up to a safe
-- state to continue calling UI functions.
--
-- A detailed stack dump of the error will be written to the game's output log
-- and a traceback returned with the error message.
--
-- Example:
--
-- > local ok, err = ui.pcall(fun, ...)
--
-- Parameters:
--
--   fun - function to call in protected mode
--   ... - any arguments to be passed to the function
--
-- Returns:
--
--   nil
--
function ui.pcall(fun, ...)
	local stack = pigui.GetImguiStack()
	return xpcall(fun, function(msg)
		pigui.CleanupImguiStack(stack)
		logWarning("Caught error in Lua UI code:\n\t" .. tostring(msg) .. '\n')
		logVerbose(debug.dumpstack(2))
		return debug.traceback(msg, 2) .. "\n"
	end, ...)
end

local _nextWindowPadding = nil

--
-- Function: ui.setNextWindowPadding()
--
-- Overrides the window padding for the next ui.window() or ui.child() call
-- without propagating the padding to further subwindows.
--
-- Example:
--
-- > ui.setNextWindowPadding( Vector2(0, 0) )
-- > ui.window("NoPadding", function()
-- >     ui.child("ID", { "AlwaysUseWindowPadding" }, function() ... end)
-- > end)
--
-- Parameters:
--
--  padding - Vector2, the window padding to override with.
--
function ui.setNextWindowPadding(padding)
	_nextWindowPadding = padding
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
	local ok

	if _nextWindowPadding then
		pigui.PushStyleVar("WindowPadding", _nextWindowPadding)
		ok = pigui.Begin(name, params)
		pigui.PopStyleVar(1)
	else
		ok = pigui.Begin(name, params)
	end
	_nextWindowPadding = nil

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

	if _nextWindowPadding then
		pigui.PushStyleVar("WindowPadding", _nextWindowPadding)
		pigui.BeginChild(id, size, flags)
		pigui.PopStyleVar()
		_nextWindowPadding = nil
	else
		pigui.BeginChild(id, size, flags)
	end

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
	return ui.isWindowHovered({"AnyWindow", "AllowWhenBlockedByPopup", "AllowWhenBlockedByActiveItem"})
end

--
-- Function: ui.canClickOnScreenObjectHere
--
-- A set of checks sufficient to safely process a click at the current mouse
-- coordinates.
--
-- Returns:
--
--   boolean
--
function ui.canClickOnScreenObjectHere()
	return not ui.isAnyWindowHovered() and not ui.isAnyPopupOpen()
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
-- Function: ui.escapeKeyReleased
--
-- Performs some sanity checks and returns true if the user has pressed escape
-- and the escape key is not currently being consumed.
--
--
-- Parameters:
--
--   ignorePopup - if true, skip checking for open popups.
--
-- Returns:
--
--   boolean - true if the escape key is pressed and not being consumed
--
function ui.escapeKeyReleased(ignorePopup)
	return (ignorePopup or not ui.isAnyPopupOpen()) and ui.noModifierHeld() and ui.isKeyReleased(ui.keys.escape)
end

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
-- Function: ui.tabBarFont
--
-- ui.tabBarFont(id, tabs, font, [args...])
--
--
-- Example:
--
-- >
--
-- Parameters:
--   id    - String, unique id to identify the group of tabs by
--   items - Table, a list of contents. Each item should contain a 'name'
--           field and a 'draw' field with a function that displays that
--           tab's contents.
--   font  - Font Table, the header font for the tab
--   args  - [optional] varargs to pass to the draw function of each tab
--
-- Returns:
--
--   index - index of the open tab if the tab bar is open, 0 otherwise
--
function ui.tabBarFont(id, items, font, ...)
	local active_index = 0

	pigui.PushStyleVar("FramePadding", ui.theme.styles.TabPadding)
	local _fnt = pigui:PushFont(font.name, font.size)
	local open = pigui.BeginTabBar(id)
	if _fnt then pigui.PopFont() end
	pigui.PopStyleVar(1)

	if not open then return active_index end

	for i, item in ipairs(items) do
		pigui.PushStyleVar("FramePadding", ui.theme.styles.TabPadding)
		local _fnt = pigui:PushFont(font.name, font.size)
		local tab_open = pigui.BeginTabItem(item.name or item[1])
		if _fnt then pigui.PopFont() end
		pigui.PopStyleVar(1)

		if tab_open then
			ui.spacing()

			active_index = (item.draw or item[2])(...)

			pigui.EndTabItem()
		end
	end

	pigui.EndTabBar()

	return active_index
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
	local res = table.pack(fun())
	pigui.PopStyleColor(utils.count(styles))
	return table.unpack(res, 1, res.n)
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
-- Function: ui.withClipRect
--
-- ui.withClipRect(min, max, fun)
--
-- Wrap the passed UI code inside a user-defined clipping rectangle
--
-- Example:
--
-- >
--
-- Parameters:
--   min        - Vector2, minimum screen position of the new clip rect
--   max        - Vector2, maximum screen position of the new clip rect
--   fun        - function, a function to call that shows the contents
--
-- Returns:
--
--   any - the value returned from fun
--
function ui.withClipRect(min, max, fun)
	pigui.PushClipRect(min, max, true)
	local res = fun()
	pigui.PopClipRect()
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

--
-- Function: ui.incrementDrag
--
-- ui.incrementDrag(label, value, v_speed, v_min, v_max, format, draw_progress_bar)
--
-- Create a "drag with arrows and progress bar" widget, uses type double as value.
--
-- Example:
--
-- > value, changed = ui.incrementDrag("##mydrag", value, 1, 0, 20, "%.0f", false)
--
-- Parameters:
--
--   label - string, text, also used as ID
--   value - int, set drag to this value
--   v_speed - minimum change step
--   v_min - int, lower bound
--   v_max - int, upper bound
--   format - string, format according to snprintf
--   draw_progress_bar - optional boolean, whether to draw a progress bar as
--                       the value changes from minimum to maximum
--
-- Returns:
--
--   value - the value that the drag was set to
--   changed - nil, if the value has not changed
--             1, if value is changed by mouse
--             2, if the value is changed by keyboard input
--
function ui.incrementDrag(...)
	local args = table.pack(...)
	return ui.withButtonColors(ui.theme.buttonColors.transparent, function()
		return pigui.IncrementDrag(table.unpack(args))
	end)
end
