-- Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local ui = require 'pigui'

local orbiteer = ui.fonts.orbiteer

local colors = ui.theme.colors

local mainButtonSize = Vector2(math.ceil(58 * (ui.screenHeight / 1200)), math.ceil(58 * (ui.screenHeight / 1200)))
local mainButtonFramePadding = math.ceil(6 * (ui.screenHeight / 1200))
local mainButtonWindowPadding = Vector2(math.ceil(6 * (ui.screenHeight / 1200)), math.ceil(6 * (ui.screenHeight / 1200)))
local mainButtonItemSpacing = Vector2(math.ceil(6 * (ui.screenHeight / 1200)), math.ceil(6 * (ui.screenHeight / 1200)))
local bottomUiMargin = math.ceil(160 * (ui.screenHeight / 1200))
local buttonWindowPos = Vector2(0, 0)
local viewWindowPadding = Vector2(4, 8)

local PiGuiTabView = {}

local function infoButton(icon, selected, tooltip, color)
    if color == nil then
        color = colors.white
    end
    return ui.coloredSelectedIconButton(icon, mainButtonSize, selected, mainButtonFramePadding, colors.buttonBlue, color, tooltip)
end

function PiGuiTabView.New(viewName, legacyTabView)
    local self = {
        name = viewName,
        legacyTabView = legacyTabView,
        currentTab = 0,
		viewCount = 0,
		isActive = false,
		tabs = {},
		windowPadding = Vector2(0)
    }

    setmetatable(self, {
        __index = PiGuiTabView,
        class = "UI.PiGuiTabView",
    })

    return self
end

function PiGuiTabView.resize(self)
    self.buttonWindowSize = Vector2(
            (mainButtonSize.x + mainButtonFramePadding * 2) * self.viewCount + mainButtonItemSpacing.x * (self.viewCount-1) + mainButtonWindowPadding.x,
            (mainButtonSize.y + mainButtonFramePadding * 2) + mainButtonWindowPadding.y)

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

function PiGuiTabView:SwitchTo(id)
    for i, v in ipairs(self.tabs) do
        if v.id == id then
            if self.legacyTabView then
                self.legacyTabView:SwitchTo(v.id)
                self.legacyTabView.outerBody:Hide()
            end

            if(self.currentTab ~= i) then self.tabs[i].refresh() end
            self.currentTab = i
            return
        end
    end
    print("View not found:", id)
end

function PiGuiTabView.renderTabView(self)
	self.isActive = Game.CurrentView() == self.name
    if not self.isActive then
        self.currentTab = 1
        self.tabs[1].refresh()
        return
    end

    local tab = self.tabs[self.currentTab] or self.tabs[1]
    if not tab then return end

    if(tab.showView) then
        ui.withStyleColors({
            ["WindowBg"] = colors.blueBackground,
            ["Border"] = colors.blueFrame,
        }, function()
            ui.withStyleVars({
                WindowRounding = 0,
                WindowBorderSize = 1.0,
                WindowPadding = self.windowPadding,
            }, function()
                ui.setNextWindowPos(self.viewWindowPos, "Always")
                ui.setNextWindowSize(self.viewWindowSize, "Always")
				ui.window("StationView", {"NoResize", "NoTitleBar"}, function()
					local ok, err = ui.pcall(tab.draw, tab)
					if not ok then logWarning(err); tab.showView = false end
				end)
            end)
        end)
    end

    if self.legacyTabView and tab.showView then
        self.legacyTabView.outerBody:Hide()
    elseif self.legacyTabView then
        self.legacyTabView.outerBody:Enable()
    end

    ui.withFont(orbiteer.large.name, orbiteer.large.size * 1.5, function()
        local text_window_padding = 12
        local text_window_size = Vector2(
                ui.calcTextSize(tab.name).x + text_window_padding * 2,
                self.buttonWindowSize.y + 6)
        local text_window_pos = Vector2(ui.screenWidth - text_window_size.x, 3)

        ui.setNextWindowPos(text_window_pos, "Always")
        ui.setNextWindowSize(text_window_size, "Always")
        ui.window("StationViewName", {"NoResize", "NoTitleBar", "NoMove", "NoFocusOnAppearing"}, function()
            ui.text(tab.name)
        end)
    end)

    ui.setNextWindowSize(self.buttonWindowSize, "Always")
    ui.setNextWindowPos(buttonWindowPos, "Always")
    ui.withStyleVars({WindowPadding = mainButtonWindowPadding, ItemSpacing = mainButtonItemSpacing}, function()
        ui.window("StationViewButtons", {"NoResize", "NoTitleBar", "NoMove", "NoFocusOnAppearing", "NoScrollbar"}, function()
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
		self.tabs[self.currentTab]:refresh()
	end
end

return PiGuiTabView
