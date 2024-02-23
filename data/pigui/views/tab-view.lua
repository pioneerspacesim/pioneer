-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local ui = require 'pigui'
local lui = require 'Lang'.GetResource 'ui-core'

local orbiteer = ui.fonts.orbiteer
local Vector2 = _G.Vector2
local logWarning = _G.logWarning

local colors = ui.theme.colors

local bigButtonSize = ui.rescaleUI(Vector2(58, 58))
local bigButtonFramePadding = ui.rescaleUI(6)
local bigButtonWindowPadding = ui.rescaleUI(Vector2(6, 6))
local bigButtonItemSpacing = ui.rescaleUI(Vector2(6, 6))
local bottomUiMargin = require 'pigui.modules.time-window'.window_height
local buttonWindowPos = Vector2(0, 0)
local viewWindowPadding = ui.rescaleUI(Vector2(4, 8))

local PiGuiTabView = {}

local function infoButton(icon, selected, tooltip)
	local variant = selected and ui.theme.buttonColors.selected or nil
	return ui.iconButton(icon, bigButtonSize, tooltip, variant, nil, bigButtonFramePadding)
end

local function drawTabWindow(_, fn)
	return fn()
end

function PiGuiTabView.New(viewName)
	local self = {
		name = viewName,
		currentTab = 1,
		viewCount = 0,
		isActive = false,
		tabs = {},
		windowPadding = Vector2(0),
		renderTab = drawTabWindow
	}

	setmetatable(self, {
		__index = PiGuiTabView,
		class = "UI.PiGuiTabView",
	})

	return self
end

function PiGuiTabView.resize(self)
	self.buttonWindowSize = Vector2(
		(bigButtonSize.x + bigButtonFramePadding * 2) * self.viewCount + bigButtonItemSpacing.x * (self.viewCount-1) + bigButtonWindowPadding.x,
		(bigButtonSize.y + bigButtonFramePadding * 2) + bigButtonWindowPadding.y)

	self.viewWindowSize = Vector2(
		ui.screenWidth - viewWindowPadding.x * 2,
		ui.screenHeight - self.buttonWindowSize.y - bottomUiMargin - viewWindowPadding.y * 2)

	self.viewWindowPos = Vector2(
		viewWindowPadding.x,
		self.buttonWindowSize.y + viewWindowPadding.y)
end

function PiGuiTabView.registerView(self, view)
	local idx = self.tabs[view.id]
	if idx then self.tabs[idx] = view; return end

	table.insert(self.tabs, view)
	self.viewCount = self.viewCount + 1
	self.tabs[view.id] = self.viewCount
	self:resize()
end

function PiGuiTabView:refreshTab(i)
	local tab = self.tabs[i]
	local ok, err = ui.pcall(tab.refresh, tab)

	if not ok then
		tab.showView = false
		tab.err = err
	end
end

function PiGuiTabView:SwitchTo(id)
	for i, v in ipairs(self.tabs) do
		if v.id == id then
			if self.currentTab ~= i then
				self.currentTab = i
				self:refreshTab(i)
			end
			return
		end
	end
	print("View not found:", id)
end

local staticButtonFlags = ui.WindowFlags {"NoResize", "NoTitleBar", "NoMove", "NoFocusOnAppearing", "NoScrollbar", "NoScrollWithMouse"}
local vCenter = Vector2(0.5, 0.5)
local mainWindowFlags = ui.WindowFlags {"NoResize", "NoTitleBar"}

function PiGuiTabView.renderTabView(self)
	local wasActive = self.isActive
	self.isActive = Game.CurrentView() == self.name
	if not self.isActive then return end

	if not self.tabs[self.currentTab] then
		self.currentTab = 1
	end

	local tab = self.tabs[self.currentTab]
	if not tab then return end

	-- refresh the tab since we're swapping back to the view
	if self.isActive and not wasActive then
		self:refreshTab(self.currentTab)
	end

	local styleColors = {
		["WindowBg"] = colors.blueBackground,
		["Border"] = colors.blueFrame,
	}
	local styleVars = {
		WindowRounding = 0,
		WindowBorderSize = 1.0,
		WindowPadding = self.windowPadding,
	}

	if(tab.windows) then
		tab.windows:display()
	end	

	if (tab.showView) then
		ui.withStyleColorsAndVars(styleColors, styleVars, function()
			ui.setNextWindowPos(self.viewWindowPos, "Always")
			ui.setNextWindowSize(self.viewWindowSize, "Always")
			ui.window("StationView", mainWindowFlags, function()
				self:renderTab(function()
					tab.showView, tab.err = ui.pcall(tab.draw, tab)
				end)
			end)
		end)
	end

	if (tab.err) then
		ui.setNextWindowPos(self.viewWindowPos + self.viewWindowSize * 0.5, "Always", vCenter)
		ui.setNextWindowSize(self.viewWindowSize * 0.6, "Always")
		ui.withStyleColorsAndVars(styleColors, styleVars, function()
			ui.window("StationViewTabError", {"NoResize", "NoTitleBar"}, function()
				ui.withFont(ui.fonts.orbiteer.medlarge, function() ui.text(lui.AN_ERROR_HAS_OCCURRED) end)
				ui.withFont(ui.fonts.pionillium.medium, function()
					ui.spacing()
					ui.textWrapped(lui.PLEASE_REPORT_THIS_ERROR:interp {path = ui.userDirPath()})
					ui.spacing()
					ui.child("#TabErrorMsg", function()
						local err = tab.err
						if type(err) == "string" then err = err:gsub('\t', '        ') end
						ui.textWrapped(err)
					end)
				end)
			end)
		end)
	end

	ui.withFont(orbiteer.large.name, orbiteer.large.size * 1.5, function()
		local text_window_padding = 12
		local text_window_size = Vector2(
			ui.calcTextSize(tab.name).x + text_window_padding * 2,
			self.buttonWindowSize.y + 6)
		local text_window_pos = Vector2(ui.screenWidth - text_window_size.x, 3)

		ui.setNextWindowPos(text_window_pos, "Always")
		ui.setNextWindowSize(text_window_size, "Always")
		ui.window("StationViewName", staticButtonFlags, function()
			ui.text(tab.name)
		end)
	end)

	ui.setNextWindowSize(self.buttonWindowSize, "Always")
	ui.setNextWindowPos(buttonWindowPos, "Always")
	ui.withStyleVars({WindowPadding = bigButtonWindowPadding, ItemSpacing = bigButtonItemSpacing}, function()
		ui.window("StationViewButtons", staticButtonFlags, function()
			for i, v in ipairs(self.tabs) do
				if infoButton(v.icon, i == self.currentTab, v.name) then
					self:SwitchTo(v.id)
				end
				ui.sameLine()
			end
		end)
	end)

	if ui.ctrlHeld() and ui.isKeyReleased(ui.keys.delete) then
		if tab.debugReload then tab:debugReload() end
		-- refresh the (possibly) new tab
		self:refreshTab(self.currentTab)
	end
end

return PiGuiTabView
