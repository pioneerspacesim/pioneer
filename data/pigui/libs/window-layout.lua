-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local utils = require "libs.utils"
local ui = require 'pigui'

-- Layout is a very simple class intended to handle layout and drawing of
-- multiple auto-sizing windows. Create a layout, create window objects,
-- and call display() each frame.

local layout = utils.inherits(nil, 'ui.Layout')

local __newLayout = layout.New
function layout.New(windows)
	local self = __newLayout()
	self.__dummyFrames = 3

	self.enabled = true
	self.mainFont = ui.fonts.pionillium.medium
	self.windows = windows or {}

	return self
end

local defaultWindowFlags = ui.WindowFlags {"NoTitleBar", "AlwaysAutoResize", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus", "NoSavedSettings"}
function layout.NewWindow(name, bgColor, flags)
	local new_window = {
		size = Vector2(0.0, 0.0),
		pos = Vector2(0.0, 0.0),
		visible = true,
		name = name,
		style_colors = {["WindowBg"] = bgColor or ui.theme.colors.lightBlackBackground},
		params = flags or defaultWindowFlags,
	}
	--- func desc
	---@param collapse boolean|nil If not present or true, collapse this window, else expand it
	function new_window:Collapse( collapse )
		if collapse == nil then collapse = true end 
		self.to_collapse = collapse
	end
	return new_window
end

function layout:onUpdateWindowConstraints(windows)
	-- override me
end

function layout:onUpdateWindowPivots(windows)
	-- override me
end

local function calcWindowPivot(anchorH, anchorV)
	local pv = Vector2(0, 0)
	if anchorH == ui.anchor.right then pv.x = 1.0 end
	if anchorH == ui.anchor.center then pv.x = 0.5 end

	if anchorV == ui.anchor.bottom then pv.y = 1.0 end
	if anchorV == ui.anchor.center then pv.y = 0.5 end

	return pv
end

local function showWindow(w)
	if not w.visible or (w.ShouldShow and not w:ShouldShow()) then
		return end

	ui.setNextWindowSize(w.size, "Always")
	ui.setNextWindowPos(w.pos, "Always", w.pivot)
	if w.to_collapse ~= nil then
		ui.setNextWindowCollapsed( w.to_collapse )
		w.to_collapse = nil
	end
	ui.withStyleColors(w.style_colors, function() ui.window(w.name, w.params, function() w:Show() end) end)
end

function layout:display()
	if self.__dummyFrames > 0 then -- do it a few frames, because imgui need a few frames to make the correct window size

		-- measuring windows (or dummies)
		ui.withFont(self.mainFont, function()
			for _, w in pairs(self.windows) do
				-- draw window outside the visible viewport
				ui.setNextWindowPos(Vector2(ui.screenWidth, 0.0), "Always")
				ui.window(w.name, w.params, function()
					if w.Dummy then w:Dummy() else w:Show() end
					w.size = ui.getWindowSize()
				end)
			end
		end)

		-- make final calculations on the last non-working frame
		if self.__dummyFrames == 1 then
			self:onUpdateWindowPivots(self.windows)

			for _, w in pairs(self.windows) do
				local anchorH = w.anchors and w.anchors[1] or ui.anchor.left
				local anchorV = w.anchors and w.anchors[2] or ui.anchor.top
				w.pivot = calcWindowPivot(anchorH, anchorV)
				w.pos = w.pivot * Vector2(ui.screenWidth, ui.screenHeight)
			end

			-- callback allows client code to override automatic window calculations
			self:onUpdateWindowConstraints(self.windows)
		end

		self.__dummyFrames = self.__dummyFrames - 1
	else
		if self.enabled then
			-- display all windows
			ui.withFont(self.mainFont, function()
				for _,w in pairs(self.windows) do showWindow(w) end
			end)
		end
	end
end

return layout
