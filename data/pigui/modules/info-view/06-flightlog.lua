-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = require 'pigui'
local layout = require 'pigui.libs.window-layout'
local InfoView = require 'pigui.views.info-view'
local Lang = require 'Lang'
local FlightLog = require 'modules.FlightLog.FlightLog'
local FlightLogExporter = require 'modules.FlightLog.FlightLogExporter'
local FlightLogRenderer = require 'modules.FlightLog.FlightLogRenderer'

local pionillium = ui.fonts.pionillium
local icons = ui.theme.icons

local l = Lang.GetResource("ui-core")

local exportHtml = true


local Windows = {
	exportButtonWindow = layout.NewWindow(l.LOG_EXPORT, nil, ui.WindowFlags {"AlwaysAutoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus", "NoSavedSettings"}),
}

local flightlogWindowsLayout = layout.New(Windows)
flightlogWindowsLayout.mainFont = pionillium.medium

Windows.exportButtonWindow.anchors = { ui.anchor.right,  ui.anchor.bottom } 

--- start with this window collapsed
Windows.exportButtonWindow:Collapse()

function Windows.exportButtonWindow.ShouldShow()
	return true
end

function Windows.exportButtonWindow.Show()
	local c
	c,exportHtml = ui.checkbox(l.LOG_HTML, exportHtml)
	ui.sameLine()
	if ui.button(l.SAVE) then
		FlightLogExporter.Export( FlightLogRenderer.includedSet, FlightLogRenderer.earliestFirst, true, exportHtml)
	end
end


local function drawScreen()

	local width = ui.getContentRegion().x

	ui.columns(2, "flightLogTop", false)

	ui.setColumnWidth(0, (width*3)/4)

	ui.child( "FlightLogList", function()
		ui.withFont(pionillium.body, function()
			FlightLogRenderer.drawFlightHistory( true )
		end)
	end)
	ui.nextColumn()
	ui.child( "FlightLogConfig", function()
		ui.withFont(pionillium.body, function()
			FlightLogRenderer.displayFilterOptions()
		end) 
	end)

--	exportButtonWindow.Show()

end

InfoView:registerView({
	id = "captainsLog",
	name = l.FLIGHT_LOG,
	icon = ui.theme.icons.bookmark,
	showView = true,
	draw = drawScreen,
	refresh = function() end,
	debugReload = function() 
		package.reimport() 
	end,
	windows = flightlogWindowsLayout
})
