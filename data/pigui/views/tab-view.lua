-- Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local ui = require 'pigui'
local lui = require 'Lang'.GetResource 'ui-core'

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

function PiGuiTabView.New(viewName)
    local self = {
        name = viewName,
        currentTab = 1,
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
            if self.currentTab ~= i then self.tabs[i].refresh() end
            self.currentTab = i
            return
        end
    end
    print("View not found:", id)
end

local staticButtonFlags = ui.WindowFlags {"NoResize", "NoTitleBar", "NoMove", "NoFocusOnAppearing", "NoScrollbar"}
local vCenter = Vector2(0.5, 0.5)

function PiGuiTabView.renderTabView(self)
	local wasActive = self.isActive
	self.isActive = Game.CurrentView() == self.name
    if not self.isActive then return end

	local tab = self.tabs[self.currentTab] or self.tabs[1]
	if not tab then return end

	-- refresh the tab since we're swapping back to the view
	if self.isActive and not wasActive then tab.refresh() end

	local styleColors = {
		["WindowBg"] = colors.blueBackground,
		["Border"] = colors.blueFrame,
	}
	local styleVars = {
		WindowRounding = 0,
		WindowBorderSize = 1.0,
		WindowPadding = self.windowPadding,
	}

    if (tab.showView) then
        ui.withStyleColorsAndVars(styleColors, styleVars, function()
			-- TODO: show an error message dialog here if the tab has errored out.
			ui.setNextWindowPos(self.viewWindowPos, "Always")
			ui.setNextWindowSize(self.viewWindowSize, "Always")
			ui.window("StationView", {"NoResize", "NoTitleBar"}, function()
				tab.showView, tab.err = ui.pcall(tab.draw, tab)
				if not tab.showView then logWarning(err) end
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
					ui.textWrapped(lui.PLEASE_REPORT_THIS_ERROR)
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
    ui.withStyleVars({WindowPadding = mainButtonWindowPadding, ItemSpacing = mainButtonItemSpacing}, function()
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
		self.tabs[self.currentTab]:refresh()
	end
end

return PiGuiTabView
