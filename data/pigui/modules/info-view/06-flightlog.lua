-- Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = require 'pigui'
local InfoView = require 'pigui.views.info-view'
local Lang = require 'Lang'
local FlightLog = require 'FlightLog'
local Format = require 'Format'
local Color = _G.Color
local Vector2 = _G.Vector2
local FileSystem = require 'FileSystem'
local Character = require 'Character'

local pionillium = ui.fonts.pionillium
local icons = ui.theme.icons

local gray = Color(145, 145, 145)

local l = Lang.GetResource("ui-core")

local iconSize = ui.rescaleUI(Vector2(28, 28))
local buttonSpaceSize = iconSize


local include_player_info = true
local include_custom_log = true
local include_station_log = true
local include_system_log = true
local export_html = true

local function getIncludedSet()
	o = {}
	if include_player_info then o[#o+1] = "CurrentStatus" end
	if include_custom_log then o[#o+1] = "Custom" end
	if include_station_log then o[#o+1] = "Station" end
	if include_system_log then o[#o+1] = "System" end

	return o;
end


function writeLogEntry( entry, formatter, write_header )
	if write_header then
		formatter:write( entry:GetLocalizedName() ):newline()
	end

	for _, pair in pairs( entry:GetDataPairs() ) do
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

local text_formatter = {}

function text_formatter:open( file )
	self.file = file
	return self
end

function text_formatter:write( string )
	self.file:write( string )
	return self
end

function text_formatter:newline()
	self.file:write( "\n" )
	return self
end

function text_formatter:close()
	self.file:close()
end

function text_formatter:headerText(title, text, wrap)
	-- Title text is gray, followed by the variable text:
	if not text then return end
	self:write( string.gsub(title, ":", "") ):write( ": " )

	-- TODO wrap?
	self:write( text ):newline()
	return self
end

function text_formatter:separator()
	self.file:write( "\n----------------------------------------------\n\n" )
end

local html_formatter = {}

function html_formatter:write( string )
	self.file:write( string )
	return self
end

function html_formatter:open( file )
	self.file = file
	self.file:write( "<html>\n" )
	return self
end

function html_formatter:newline()
	self.file:write( "<br>\n" )
	return self
end

function html_formatter:close()
	self.file:write( "</html>" )
	self.file:close()
end

function html_formatter:headerText(title, text, wrap)
	-- Title text is gray, followed by the variable text:
	if not text then return end
	self:write( "<b>" ):write( string.gsub(title, ":", "") ):write( ": </b>" )

	-- TODO wrap?
	self:write( text ):newline()
	return self
end

function html_formatter:separator()
	self.file:write( "\n<hr>\n" )
end

entering_text = false

-- Display Entry text, and Edit button, to update flightlog
function inputText(entry, counter, entering_text, str, clicked)
	if #entry.entry > 0 then
		ui_formatter:headerText(l.ENTRY, entry.entry, true)
	end

	if clicked or entering_text == counter then
		ui.spacing()
		ui.pushItemWidth(-1.0)
		local updated_entry, return_pressed = ui.inputText("##" ..str..counter, entry.entry, {"EnterReturnsTrue"})
		ui.popItemWidth()
		entering_text = counter
		if return_pressed then
			entry:UpdateEntry(updated_entry)
			entering_text = -1
		end
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
	local was_clicked = false
	for entry in FlightLog:GetLogEntries(getIncludedSet()) do
	 	counter = counter + 1
	
		 writeLogEntry( entry, formatter, true )

		 if entry:HasEntry() then
			::input::
			entering_text = inputText(entry, counter,
				entering_text, "custom", was_clicked)
			ui.nextColumn()

			was_clicked = false
			if ui.iconButton(icons.pencil, buttonSpaceSize, l.EDIT .. "##custom"..counter) then
				was_clicked = true
				-- If edit field was clicked, we want to edit _this_ iteration's field,
				-- not next record's. Quick, behind you, velociraptor!
				goto input
			end
		else
			ui.nextColumn()
		end

		if entry:SupportsDelete() then
			if ui.iconButton(icons.trashcan, buttonSpaceSize, l.REMOVE .. "##custom" .. counter) then
				entry:Delete()
				-- if we were already in edit mode, reset it, or else it carries over to next iteration
				entering_text = false
			end
		end

		ui.nextColumn()
		ui.separator()
		ui.spacing()		 
	end
end

local function checkbox(label, checked, tooltip)
--	local color = colors.buttonBlue
--	local changed, ret
--	ui.withStyleColors({["Button"]=color,["ButtonHovered"]=color:tint(0.1),["CheckMark"]=color:tint(0.2)},function()
--		changed, ret = ui.checkbox(label, checked)
--	end)
--	if ui.isItemHovered() and tooltip then
--		Engine.pigui.SetTooltip(tooltip) -- bypass the mouse check, Game.player isn't valid yet
--	end
	
	changed, ret = ui.checkbox(label, checked)
	
	
	return changed, ret
end

local function exportLogs()
	-- TODO localize?
	local foldername = FileSystem.MakeUserDataDirectory( "player_logs" )

    local player = Character.persistent.player

	local base_save_name = player.name

	local formatter
	local extension
	if export_html then
		formatter = html_formatter
		extension = '.html'
	else
		formatter = text_formatter
		extension = '.log'
	end

	local log_filename = FileSystem.JoinPath( foldername, base_save_name .. extension )

	formatter:open( io.open( log_filename, "w" ) )

	for entry in FlightLog:GetLogEntries(getIncludedSet()) do

		writeLogEntry(entry, formatter, true)

		if #entry.entry > 0 then
			formatter:headerText(l.ENTRY, entry.entry, true)
		end
		formatter:separator()
	end

	formatter:close()

end

local function displayFilterOptions()
	ui.spacing()
	local c
	local flight_log = true;

	c,include_player_info = checkbox(l.PERSONAL_INFORMATION, include_player_info)
	c,include_custom_log = checkbox(l.LOG_CUSTOM, include_custom_log)
	c,include_station_log = checkbox(l.LOG_STATION, include_station_log)
	c,include_system_log = checkbox(l.LOG_SYSTEM, include_system_log)
	ui.spacing()
	ui.separator()
	ui.spacing()
	c,export_html = checkbox("HTML", export_html)

	ui.spacing()

	if ui.button(l.SAVE) then
		exportLogs()
	end

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
	end
})
