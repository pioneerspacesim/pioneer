-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- Stuff from the C++ side that we want available directly in Lua
-- without any wrappers
local Engine = require 'Engine'
local pigui = Engine.pigui

---@class ui
local ui = {}

ui.calcTextAlignment = pigui.CalcTextAlignment
ui.alignTextToLineHeight = pigui.AlignTextToLineHeight
ui.alignTextToFramePadding = pigui.AlignTextToFramePadding
ui.lineOnClock = pigui.lineOnClock
ui.pointOnClock = pigui.pointOnClock
ui.screenWidth = pigui.screen_width
ui.screenHeight = pigui.screen_height

-- Return the size of the specified window's contents from last frame (without padding/decoration)
-- Returns {0,0} if the window hasn't been submitted during the lifetime of the program
ui.getWindowContentSize = pigui.GetWindowContentSize ---@type fun(name: string): Vector2
ui.setNextWindowPos = pigui.SetNextWindowPos ---@type fun(pos: Vector2, cond: string, pivot: Vector2?)
ui.setNextWindowSize = pigui.SetNextWindowSize ---@type fun(size: Vector2, cond: string)
ui.setNextWindowSizeConstraints = pigui.SetNextWindowSizeConstraints ---@type fun(min: Vector2, max: Vector2)
--- Collapse or expand the next window
ui.setNextWindowCollapsed = pigui.SetNextWindowCollapsed ---@type fun(collapse: boolean?)

-- Forwarded as-is for use in complicated layout primitives without introducing additional scopes
ui.beginGroup = pigui.BeginGroup
ui.endGroup = pigui.EndGroup
ui.getTime = pigui.GetTime

ui.dummy = pigui.Dummy
ui.newLine = pigui.NewLine
ui.spacing = pigui.Spacing
ui.text = pigui.Text
ui.combo = pigui.Combo ---@type fun(label: string, selected: integer, items: string[]): changed: boolean, selected: integer
ui.listBox = pigui.ListBox
ui.textWrapped = pigui.TextWrapped
ui.textColored = pigui.TextColored
ui.inputText = pigui.InputText
ui.checkbox = pigui.Checkbox ---@type fun(label: string, checked: boolean): changed:boolean, value:boolean
ui.separator = pigui.Separator
ui.pushTextWrapPos = pigui.PushTextWrapPos
ui.popTextWrapPos = pigui.PopTextWrapPos
ui.setScrollHereY = pigui.SetScrollHereY
ui.selectable = pigui.Selectable
ui.progressBar = pigui.ProgressBar
ui.plotHistogram = pigui.PlotHistogram
ui.setTooltip = pigui.SetTooltip
ui.setItemTooltip = pigui.SetItemTooltip
ui.addCircle = pigui.AddCircle
ui.addCircleFilled = pigui.AddCircleFilled
ui.addRect = pigui.AddRect ---@type fun(a: Vector2, b: Vector2, col: Color, rounding: number, edges: integer, thickness: number)
ui.addRectFilled = pigui.AddRectFilled ---@type fun(a: Vector2, b: Vector2, col: Color, rounding: number, edges: integer)
ui.addLine = pigui.AddLine ---@type fun(a: Vector2, b: Vector2, col: Color, thickness: number)
ui.addText = pigui.AddText ---@type fun(pos: Vector2, col: Color, text: string)
ui.pathArcTo = pigui.PathArcTo
ui.pathStroke = pigui.PathStroke
ui.setCursorPos = pigui.SetCursorPos ---@type fun(pos: Vector2)
ui.getCursorPos = pigui.GetCursorPos ---@type fun(): Vector2
ui.addCursorPos = pigui.AddCursorPos ---@type fun(add: Vector2)
ui.setCursorScreenPos = pigui.SetCursorScreenPos ---@type fun(pos: Vector2)
ui.getCursorScreenPos = pigui.GetCursorScreenPos ---@type fun(): Vector2
ui.addCursorScreenPos = pigui.AddCursorScreenPos ---@type fun(add: Vector2)
ui.lowThrustButton = pigui.LowThrustButton
ui.thrustIndicator = pigui.ThrustIndicator
ui.isMouseClicked = pigui.IsMouseClicked
ui.isMouseDoubleClicked = pigui.IsMouseDoubleClicked
ui.isMouseDown = pigui.IsMouseDown
ui.getMousePos = pigui.GetMousePos ---@type fun(): Vector2
ui.getMouseWheel = pigui.GetMouseWheel
ui.shouldDrawUI = pigui.ShouldDrawUI
ui.getWindowPos = pigui.GetWindowPos ---@type fun(): Vector2
ui.getWindowSize = pigui.GetWindowSize ---@type fun(): Vector2
-- available content region
ui.getContentRegion = pigui.GetContentRegion ---@type fun(): Vector2

-- Get the current height of a line of text (font.size)
ui.getTextLineHeight = pigui.GetTextLineHeight
-- Get the current height of a line of text plus line spacing (font.size + ItemSpacing.y)
ui.getTextLineHeightWithSpacing = pigui.GetTextLineHeightWithSpacing
-- Get current height of line (e.g. after ui.sameLine())
ui.getLineHeight = pigui.GetLineHeight ---@type fun(): number
-- Get the current frame height (font.size + FramePadding.y * 2)
ui.getFrameHeight = pigui.GetFrameHeight ---@type fun(): number
-- Get the current frame height including the next line spacing (getFrameHeight() + ItemSpacing.y)
ui.getFrameHeightWithSpacing = pigui.GetFrameHeightWithSpacing ---@type fun(): number
-- Get the current style item spacing value
ui.getItemSpacing = pigui.GetItemSpacing ---@type fun(): Vector2
-- Get the current style window padding value
ui.getWindowPadding = pigui.GetWindowPadding ---@type fun(): Vector2
-- Add extra window padding after beginning a window.
-- WARNING: this must only be called at "top-level" window scope (e.g. not in a Group or Columns etc.)
ui.addWindowPadding = pigui.AddWindowPadding ---@type fun(padding: Vector2)
ui.getItemRect = pigui.GetItemRect ---@type fun(): Vector2, Vector2 -- return min, max corners of last item bounding box

ui.getTargetsNearby = pigui.GetTargetsNearby
ui.getProjectedBodies = pigui.GetProjectedBodies
ui.getProjectedBodiesGrouped = pigui.GetProjectedBodiesGrouped
ui.isMouseReleased = pigui.IsMouseReleased
ui.isMouseDoubleClicked = pigui.IsMouseDoubleClicked
ui.isMouseHoveringRect = pigui.IsMouseHoveringRect
ui.collapsingHeader = pigui.CollapsingHeader
ui.treeNode = pigui.TreeNode
ui.treePop = pigui.TreePop
ui.beginPopupModal = pigui.BeginPopupModal
ui.endPopup = pigui.EndPopup
ui.openPopup = pigui.OpenPopup
ui.closeCurrentPopup = pigui.CloseCurrentPopup
ui.isAnyPopupOpen = pigui.IsAnyPopupOpen
ui.shouldShowLabels = pigui.ShouldShowLabels
ui.columns = pigui.Columns
ui.nextColumn = pigui.NextColumn
ui.setColumnOffset = pigui.SetColumnOffset
ui.getColumnWidth = pigui.GetColumnWidth
ui.setColumnWidth = pigui.SetColumnWidth
ui.getScrollY = pigui.GetScrollY
ui.keys = pigui.keys
ui.isKeyReleased = pigui.IsKeyReleased
ui.playSfx = pigui.PlaySfx
ui.isItemHovered = pigui.IsItemHovered
ui.isItemActive = pigui.IsItemActive
ui.isItemClicked = pigui.IsItemClicked
ui.isWindowHovered = pigui.IsWindowHovered
ui.vSliderInt = pigui.VSliderInt ---@type fun(l: string, v: integer, min: integer, max: integer, fmt: string?): value:integer, changed:boolean
ui.sliderInt = pigui.SliderInt ---@type fun(l: string, v: integer, min: integer, max: integer, fmt: string?): value:integer, changed:boolean
ui.colorEdit = pigui.ColorEdit
ui.getStyleColor = pigui.GetStyleColor
ui.nextItemWidth = pigui.NextItemWidth
ui.pushItemWidth = pigui.PushItemWidth
ui.popItemWidth = pigui.PopItemWidth
ui.calcItemWidth = pigui.CalcItemWidth
ui.sliderFloat = pigui.SliderFloat ---@type fun(l: string, v: number, min: number, max: number, fmt: string?): value:number, changed:boolean
ui.beginTabBar = pigui.BeginTabBar
ui.beginTabItem = pigui.BeginTabItem
ui.endTabItem = pigui.EndTabItem
ui.endTabBar = pigui.EndTabBar

ui.beginTable = pigui.BeginTable ---@type fun(id: string, columns: integer, flags: any)
ui.endTable = pigui.EndTable
ui.tableNextRow = pigui.TableNextRow
ui.tableNextColumn = pigui.TableNextColumn
ui.tableSetColumnIndex = pigui.TableSetColumnIndex
ui.tableSetupColumn = pigui.TableSetupColumn ---@type fun(id: string, flags: any)
ui.tableSetupScrollFreeze = pigui.TableSetupScrollFreeze
ui.tableHeadersRow = pigui.TableHeadersRow
ui.tableHeader = pigui.TableHeader

-- Flag validation functions. Call with a table of string flags as the only argument.
ui.SelectableFlags = pigui.SelectableFlags
ui.TreeNodeFlags = pigui.TreeNodeFlags
ui.InputTextFlags = pigui.InputTextFlags
ui.WindowFlags = pigui.WindowFlags
ui.HoveredFlags = pigui.HoveredFlags
ui.TableFlags = pigui.TableFlags
ui.TableColumnFlags = pigui.TableColumnFlags

-- Wrapped in buttons.lua
-- ui.button = pigui.Button

--
-- Function: ui.clearMouse
--
-- ui.clearMouse()
--
-- Resets the mouse click data in the Input class. The ActionBindings and
-- AxisBindings, as well as ImGui structs are not reset, but a subsequent call
-- to Input::MouseButtonState will return false for all mouse buttons.
--
-- Returns:
--
--   nothing
--
ui.clearMouse = pigui.ClearMouse

--
-- Function: ui.wantTextInput
--
-- ui.wantTextInput()
--
-- Returns true if text is being entered into a text field in the current frame
--
ui.wantTextInput = pigui.WantTextInput

ui.dataDirPath = pigui.DataDirPath
ui.userDirPath = pigui.UserDirPath
ui.addImage = pigui.AddImage
ui.image = pigui.Image

--
-- Function: ui.dragFloat
--
-- ui.dragFloat(label, value, v_speed, v_min, v_max, format)
--
-- Create a float drag
--
-- Example:
--
-- > value = ui.dragFloat("##mydrag", value, 0.5, 0, 20, "%dt")
--
-- Parameters:
--
--   label - string, text, also used as ID
--   value - float, set drag to this value
--   v_speed - minimum change step
--   v_min - float, lower bound
--   v_max - float, upper bound
--   format - format according to snprintf
--
-- Returns:
--
--   value - the value that the drag was set to
--   changed - boolean, whether the passed value has changed
--
ui.dragFloat = pigui.DragFloat
return ui
