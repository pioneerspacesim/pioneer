-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Defs = require 'pigui.modules.new-game-window.defs'
local ui = require 'pigui'
local Lang = require 'Lang'
local lui = Lang.GetResource("ui-core")
local ModalWindow = require 'pigui.libs.modal-win'

local Widgets = {}

Widgets.ConfirmPopup = function()
	local ConfirmPopup
	ConfirmPopup = ModalWindow.New("Confirm", function()
		ConfirmPopup.drawQuestion()
		if ui.button(lui.YES) then
			ConfirmPopup.yesAction()
			ConfirmPopup:close()
		end
		ui.sameLine()
		if ui.button(lui.NO) then
			ConfirmPopup:close()
		end
	end)
	return ConfirmPopup
end

-- layout: { labelWidth (optional), width (optional) }
Widgets.alignLabel = function(label, layout, fnc)
	if not layout.labelWidth then layout.labelWidth = 0 end
	ui.alignTextToFramePadding()
	ui.text(label)
	local labelWidth = ui.calcTextSize(label).x + ui.getItemSpacing().x
	layout.labelWidth = math.max(layout.labelWidth, labelWidth)
	ui.sameLine(0, layout.labelWidth - labelWidth + ui.getItemSpacing().x)
	if layout.width then
		ui.nextItemWidth(layout.width - layout.labelWidth)
		fnc()
	else
		-- calculate item's width
		local before = ui.getCursorPos()
		fnc()
		ui.sameLine(0)
		local after = ui.getCursorPos()
		local width = after.x - before.x

		if not layout.itemWidth or width > layout.itemWidth then
			layout.itemWidth = width
		end
		ui.dummy(Vector2(layout.itemWidth - width, 1))
	end
end

-- layout: { width }
Widgets.oneLiner = function(label, layout, fnc)
	ui.alignTextToFramePadding()
	ui.text(label)
	local labelWidth = ui.calcTextSize(label).x + ui.getItemSpacing().x
	ui.sameLine()
	ui.nextItemWidth(layout.width - labelWidth)
	fnc()
end

-- layout: { v_indent (internal), height }
Widgets.verticalCenter = function(layout, fnc)
	if not layout.v_indent then layout.v_indent = 0 end
	ui.dummy(Vector2(0, layout.v_indent))
	local h1 = ui.getCursorPos().y
	fnc()
	local h2 = ui.getCursorPos().y
	layout.v_indent = (layout.height - h2 + h1) / 2
end

-- layout: { h_indent (internal), width }
Widgets.horizontalCenter = function(layout, fnc)
	if not layout.h_indent then layout.h_indent = 0 end
	ui.dummy(Vector2(layout.h_indent, 0))
	ui.sameLine(0, 0)
	local x1 = ui.getCursorPos().x
	fnc()
	ui.sameLine(0, 0)
	local x2 = ui.getCursorPos().x
	ui.dummy(Vector2(0, 0))
	layout.h_indent = math.ceil((layout.width - x2 + x1) * 0.5)
end

-- layout: { h_indent (internal), v_indent (internal), child_id, width, height }
Widgets.centeredIn = function(layout, fnc)
	ui.child(layout.child_id, Vector2(layout.width, layout.height), function()
		Widgets.verticalCenter(layout, function()
			Widgets.horizontalCenter(layout, fnc)
		end)
	end)
end

-- lockable widgets

Widgets.inputText = function(lock, valid, id, text, randomFnc)
	return ui.withStyleColors(valid and {} or { FrameBg = ui.theme.colors.alertRed }, function()
		if lock then
			ui.withStyleColors({FrameBg = Defs.frameShadeColor }, function()
				ui.inputText(id, text, { 'ReadOnly' })
			end)
		else
			if randomFnc then
				local size = ui.getFrameHeight()
				local wholeWidth = ui.calcItemWidth()
				ui.nextItemWidth(wholeWidth - size - Defs.gap.x)
				local txt, changed = ui.inputText(id, text)
				ui.sameLine()
				if ui.iconButton(ui.theme.icons.random, Vector2(size, size), tostring(id) .. "_random_button") then
					randomFnc()
				end
				return txt, changed
			else
				return ui.inputText(id, text)
			end
		end
	end)
end

Widgets.combo = function(lock, id, selected, list)
	if lock then
		ui.withStyleColors({FrameBg = Defs.frameShadeColor }, function()
			ui.inputText(id, list[selected + 1], { 'ReadOnly' })
		end)
	else
		return ui.combo(id, selected, list)
	end
end

Widgets.incrementDrag = function(lock, label, value, v_speed, v_min, v_max, format, draw_progress_bar)
	if lock then
		local txt = string.format(format, value)
		local itemWidth = ui.calcItemWidth()
		local itemHeight = ui.getFrameHeight()
		local txtWidth = ui.calcTextSize(txt).x
		local indent = (itemWidth - txtWidth) * 0.5
		local cur = ui.getCursorPos()
		if draw_progress_bar then
			ui.withStyleColors({ ["PlotHistogram"] = Defs.progressBarColor }, function()
				ui.progressBar((value - v_min) / (v_max - v_min), Vector2(0,0), "")
			end)
		else
			local spos = ui.getCursorScreenPos()
			ui.withStyleColors({FrameBg = Defs.frameShadeColor }, function()
				ui.addLine(Vector2(spos.x, spos.y + itemHeight / 2 - 0.5), Vector2(spos.x + itemWidth, spos.y + itemHeight / 2 - 0.5), ui.getStyleColor("FrameBg"), itemHeight);
			end)
		end
		ui.setCursorPos(cur)
		ui.alignTextToFramePadding()
		ui.dummy(Vector2(indent, itemHeight))
		ui.sameLine(0, 0)
		ui.text(txt)
		ui.sameLine(0, 0)
		ui.dummy(Vector2(indent, itemHeight))
		return nil, false
	else
		return ui.incrementDrag(label, value, v_speed, v_min, v_max, format, draw_progress_bar)
	end
end

return Widgets
