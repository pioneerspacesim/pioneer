-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- Stuff from the C++ side that we want available directly in Lua
-- without any wrappers
local Engine = require 'Engine'
local pigui = Engine.pigui

local ui = {} 

ui.calcTextAlignment = pigui.CalcTextAlignment
ui.lineOnClock = pigui.lineOnClock
ui.pointOnClock = pigui.pointOnClock
ui.screenWidth = pigui.screen_width
ui.screenHeight = pigui.screen_height
ui.setNextWindowPos = pigui.SetNextWindowPos
ui.setNextWindowSize = pigui.SetNextWindowSize
ui.setNextWindowSizeConstraints = pigui.SetNextWindowSizeConstraints
ui.dummy = pigui.Dummy
ui.newLine = pigui.NewLine
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
ui.setCursorPos = pigui.SetCursorPos
ui.getCursorPos = pigui.GetCursorPos
ui.setCursorScreenPos = pigui.SetCursorScreenPos
ui.getCursorScreenPos = pigui.GetCursorScreenPos
ui.getTextLineHeight = pigui.GetTextLineHeight
ui.getTextLineHeightWithSpacing = pigui.GetTextLineHeightWithSpacing
ui.lowThrustButton = pigui.LowThrustButton
ui.thrustIndicator = pigui.ThrustIndicator
ui.isMouseClicked = pigui.IsMouseClicked
ui.isMouseDown = pigui.IsMouseDown
ui.getMousePos = pigui.GetMousePos
ui.getMouseWheel = pigui.GetMouseWheel
--ui.setTooltip = maybeSetTooltip
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
ui.isWindowHovered = pigui.IsWindowHovered
ui.vSliderInt = pigui.VSliderInt
ui.sliderInt = pigui.SliderInt
ui.nextItemWidth = pigui.NextItemWidth
ui.pushItemWidth = pigui.PushItemWidth
ui.popItemWidth = pigui.PopItemWidth
ui.sliderFloat = pigui.SliderFloat
ui.beginTabBar = pigui.BeginTabBar
ui.beginTabItem = pigui.BeginTabItem
ui.endTabItem = pigui.EndTabItem
ui.endTabBar = pigui.EndTabBar

-- Flag validation functions. Call with a table of string flags as the only argument.
ui.SelectableFlags = pigui.SelectableFlags
ui.TreeNodeFlags = pigui.TreeNodeFlags
ui.InputTextFlags = pigui.InputTextFlags
ui.WindowFlags = pigui.WindowFlags
ui.HoveredFlags = pigui.HoveredFlags

ui.button = pigui.Button

ui.dataDirPath = pigui.DataDirPath
ui.addImage = pigui.AddImage
ui.image = pigui.Image

return ui
