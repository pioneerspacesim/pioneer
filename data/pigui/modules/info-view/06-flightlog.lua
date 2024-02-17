-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = require 'pigui'
local layout = require 'pigui.libs.window-layout'
local InfoView = require 'pigui.views.info-view'
local Lang = require 'Lang'
local FlightLog = require 'modules.FlightLog.FlightLog'
local FlightLogExporter = require 'modules.FlightLog.FlightLogExporter'
local Format = require 'Format'
local Color = _G.Color
local Vector2 = _G.Vector2

local pionillium = ui.fonts.pionillium
local icons = ui.theme.icons

local gray = Color(145, 145, 145)

local l = Lang.GetResource("ui-core")

local iconSize = ui.rescaleUI(Vector2(28, 28))
local buttonSpaceSize = iconSize

local earliestFirst = true
local exportHtml = true
local includedSet = { System = true, Custom = true, Station = true }

local function writeLogEntry( entry, formatter, write_header )
	if write_header then
		formatter:write( entry:GetLocalizedName() ):newline()
	end

	for _, pair in ipairs( entry:GetDataPairs( earliestFirst ) ) do
		formatter:headerText( pair[1], pair[2] )
	end
end

local ui_formatter = {}
function ui_formatter:headerText(title, text, wrap)
	-- Title text is gray, followed by the variable text:
	if not text then return end
	ui.textColored(gray, string.gsub(title, ":", "") .. ":")
	ui.sameLine()
	if wrap then
		-- TODO: limit width of text box!
		ui.textWrapped(text)
	else
		ui.text(text)
	end
end

function ui_formatter:write( string )
	ui.text( string )
	ui.sameLine()
	return self
end

function ui_formatter:newline()
	ui.text( "" )
	return self
end

	
local entering_text = false

-- Display Entry text, and Edit button, to update flightlog
local function inputText(entry, counter, entering_text, str)

	if entering_text == counter then
		ui.spacing()
		ui.pushItemWidth(-1.0)
		local updated_entry, return_pressed = ui.inputText("##" ..str..counter, entry:GetEntry(), {"EnterReturnsTrue"})
		ui.popItemWidth()
		if return_pressed then
			entry:UpdateEntry(updated_entry)
			entering_text = -1
		end
	elseif entry:HasEntry() then
		ui_formatter:headerText(l.ENTRY, entry:GetEntry(), true)
	end

	ui.spacing()
	return entering_text
end

local function renderLog( formatter )

	ui.spacing()
	-- input field for custom log:
	ui_formatter:headerText(l.LOG_NEW, "")
	ui.sameLine()
	local text, changed = ui.inputText("##inputfield", "", {"EnterReturnsTrue"})
	if changed then
		FlightLog.MakeCustomEntry(text)
	end
	ui.separator()

	local counter = 0
	for entry in FlightLog:GetLogEntries(includedSet, nil, earliestFirst ) do
	 	counter = counter + 1
	
		 writeLogEntry( entry, formatter, true )

		 if entry:CanHaveEntry() then
			entering_text = inputText(entry, counter,
				entering_text, "custom")
			ui.nextColumn()

			if ui.iconButton(icons.pencil, buttonSpaceSize, l.EDIT .. "##custom"..counter) then
				entering_text = counter
			end
		else
			ui.nextColumn()
		end

		if entry:CanBeRemoved() then
			if ui.iconButton(icons.trashcan, buttonSpaceSize, l.REMOVE .. "##custom" .. counter) then
				FlightLog:RemoveEntry( entry )
				-- if we were already in edit mode, reset it, or else it carries over to next iteration
				entering_text = false
			end
		end

		ui.nextColumn()
		ui.separator()
		ui.spacing()
	end
end

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
		FlightLogExporter.Export( includedSet, earliestFirst, true, exportHtml)
	end
end


local function displayFilterOptions()
	ui.spacing()
	local c
	local flight_log = true;

	c,includedSet.Custom = ui.checkbox(l.LOG_CUSTOM, includedSet.Custom)
	c,includedSet.Station = ui.checkbox(l.LOG_STATION, includedSet.Station)
	c,includedSet.System = ui.checkbox(l.LOG_SYSTEM, includedSet.System)
	ui.separator()
	c,earliestFirst = ui.checkbox(l.LOG_EARLIEST_FIRST, earliestFirst)
end


local function drawFlightHistory()

	ui.spacing()
	-- reserve a narrow right column for edit / remove icon
	local width = ui.getContentRegion().x


	ui.columns(2, "##flightLogColumns", false)
	ui.setColumnWidth(0, width - (iconSize.x*3)/2)
	ui.setColumnWidth(1, (iconSize.x*3)/2)

	renderLog(ui_formatter)
end

local function drawScreen()

	local width = ui.getContentRegion().x

	ui.columns(2, "flightLogTop", false)

	ui.setColumnWidth(0, (width*3)/4)

	ui.child( "FlightLogList", function()
		ui.withFont(pionillium.body, function()
			drawFlightHistory()
		end)
	end)
	ui.nextColumn()
	ui.child( "FlightLogConfig", function()
		ui.withFont(pionillium.body, function()
			displayFilterOptions()
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
