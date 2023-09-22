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

-- Sometimes date is empty, e.g. departure date prior to departure
local function formatDate(date)
	return date and Format.Date(date) or nil
end

-- Based on flight state, compose a reasonable string for location
local function composeLocationString(location)
	return string.interp(l["FLIGHTLOG_"..location[1]],
		{ primary_info = location[2],
			secondary_info = location[3] or "",
			tertiary_info = location[4] or "",})
end


function CustomEntryIterator()
	local generator = FlightLog.GetCustomEntry()

	local function my_generator()
		local systemp, time, money, location, entry = generator()
		if systemp ~= nil then
			local entry = {
				type = "custom",
				systemp = systemp,
				time = time,
				money = money,
				location = location,
				entry = entry,
				sort_date = time
			}

			function entry:write_header( formatter )
				formatter:write( l.LOG_CUSTOM ):write(":"):newline()
			end

			function entry:write_details( formatter )
				formatter:headerText(l.DATE, formatDate(self.time))
				formatter:headerText(l.LOCATION, composeLocationString(self.location))
				formatter:headerText(l.IN_SYSTEM, ui.Format.SystemPath(self.systemp))
				formatter:headerText(l.ALLEGIANCE, self.systemp:GetStarSystem().faction.name)
				formatter:headerText(l.CASH, Format.Money(self.money))
			end

			return entry
		end
		return nil
	end

	return my_generator
end

function StationPathIterator()
	local generator = FlightLog.GetStationPaths()

	local function my_generator()
		local systemp, deptime, money, entry = generator()
		if systemp ~= nil then
			local entry = {
				type = "station",
				systemp = systemp,
				deptime = deptime,
				money = money,
				entry = entry,
				sort_date = deptime
			}

			function entry:write_header( formatter )
				formatter:write( l.LOG_STATION ):write(":"):newline()
			end

			function entry:write_details( formatter )
				local station_type = "FLIGHTLOG_" .. self.systemp:GetSystemBody().type
				formatter:headerText(l.DATE, formatDate(self.deptime))
				formatter:headerText(l.STATION, string.interp(l[station_type],
					{	primary_info = self.systemp:GetSystemBody().name,
						secondary_info = self.systemp:GetSystemBody().parent.name }))
				-- formatter:headerText(l.LOCATION, self.systemp:GetSystemBody().parent.name)
				formatter:headerText(l.IN_SYSTEM, ui.Format.SystemPath(self.systemp))
				formatter:headerText(l.ALLEGIANCE, self.systemp:GetStarSystem().faction.name)
				formatter:headerText(l.CASH, Format.Money(self.money))
			end
			return entry
		end
		return nil
	end

	return my_generator
end

function SystemPathIterator()
	local generator = FlightLog.GetSystemPaths()

	local function my_generator()
		local systemp, arrtime, deptime, entry = generator()
		if systemp ~= nil then
			local entry = {
				type = "system",
				systemp = systemp,
				arrtime = arrtime,
				deptime = deptime,
				entry = entry,
				sort_date = arrtime
			}

			if nil == entry.sort_date then
				entry.sort_date = entry.deptime
			end

			function entry:write_header( formatter )
				formatter:write( l.LOG_SYSTEM ):write(":"):newline()
			end

			function entry:write_details( formatter )
				formatter:headerText(l.ARRIVAL_DATE, formatDate(self.arrtime))
				formatter:headerText(l.DEPARTURE_DATE, formatDate(self.deptime))
				formatter:headerText(l.IN_SYSTEM, ui.Format.SystemPath(self.systemp))
				formatter:headerText(l.ALLEGIANCE, self.systemp:GetStarSystem().faction.name)
			end
			return entry
		end
		return nil
	end

	return my_generator
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


entering_text_custom = false
entering_text_system = false
entering_text_station = false

-- Display Entry text, and Edit button, to update flightlog
function inputText(entry, counter, entering_text, log, str, clicked)
	if #entry > 0 then
		ui_formatter:headerText(l.ENTRY, entry, true)
	end

	if clicked or entering_text == counter then
		ui.spacing()
		ui.pushItemWidth(-1.0)
		local updated_entry, return_pressed = ui.inputText("##" ..str..counter, entry, {"EnterReturnsTrue"})
		ui.popItemWidth()
		entering_text = counter
		if return_pressed then
			log(counter, updated_entry)
			entering_text = -1
		end
	end

	ui.spacing()
	return entering_text
end

local function renderCustomLog( formatter )
	local counter = 0
	local was_clicked = false
	for entry in CustomEntryIterator() do
		counter = counter + 1

		entry:write_details( formatter )

		::input::
		entering_text_custom = inputText(entry.entry, counter,
			entering_text_custom, FlightLog.UpdateCustomEntry, "custom", was_clicked)
		ui.nextColumn()

		was_clicked = false
		if ui.iconButton(icons.pencil, buttonSpaceSize, l.EDIT .. "##custom"..counter) then
			was_clicked = true
			-- If edit field was clicked, we want to edit _this_ iteration's field,
			-- not next record's. Quick, behind you, velociraptor!
			goto input
		end

		if ui.iconButton(icons.trashcan, buttonSpaceSize, l.REMOVE .. "##custom" .. counter) then
			FlightLog.DeleteCustomEntry(counter)
			-- if we were already in edit mode, reset it, or else it carries over to next iteration
			entering_text_custom = false
		end

		ui.nextColumn()
		ui.separator()
		ui.spacing()
	end
end

-- See comments on previous function
local function renderStationLog( formatter )
	local counter = 0
	local was_clicked = false
	for entry in StationPathIterator() do
		counter = counter + 1

		entry:write_details( formatter )

		::input::
		entering_text_station = inputText(entry.entry, counter,
			entering_text_station, FlightLog.UpdateStationEntry, "station", was_clicked)
		ui.nextColumn()

		was_clicked = false
		if ui.iconButton(icons.pencil, buttonSpaceSize, l.EDIT .. "##station"..counter) then
			was_clicked = true
			goto input
		end

		ui.nextColumn()
		ui.separator()
	end
end

-- See comments on previous function
local function renderSystemLog( formatter )
	local counter = 0
	local was_clicked = false
	for entry in SystemPathIterator() do
		counter = counter + 1

		entry:write_details( formatter )

		::input::
		entering_text_system = inputText(entry.entry, counter,
			entering_text_system, FlightLog.UpdateSystemEntry, "sys", was_clicked)
		ui.nextColumn()

		was_clicked = false
		if ui.iconButton(icons.pencil, buttonSpaceSize, l.EDIT .. "##system"..counter) then
			was_clicked = true
			goto input
		end

		ui.nextColumn()
		ui.separator()
	end
end

local function displayLog(logFn)
	ui.spacing()
	-- reserve a narrow right column for edit / remove icon
	local width = ui.getContentRegion().x
	ui.columns(2, "##flightLogColumns", false)
	ui.setColumnWidth(0, width - iconSize.x)

	logFn(ui_formatter)
	ui.columns(1)
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

local export_player_info = true
local export_custom_log = true
local export_station_log = true
local export_system_log = true
local export_html = true

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

	if export_player_info then
		formatter:headerText( l.NAME_PERSON, player.name )
		-- TODO: localize
		formatter:headerText( "Title", player.title )
		formatter:headerText( l.RATING, l[player:GetCombatRating()] )
		formatter:headerText( l.KILLS,  string.format('%d',player.killcount) )
		formatter:separator()
		formatter:newline()
	end

	all_entries = {}

	if export_custom_log then
		for entry in CustomEntryIterator() do
			table.insert( all_entries, entry )
		end
	end

	if export_station_log then
		for entry in StationPathIterator() do
			table.insert( all_entries, entry )
		end
	end

	if export_system_log then
		for entry in SystemPathIterator() do
			table.insert( all_entries, entry )
		end
	end

	local function sortf( a, b )
		return a.sort_date < b.sort_date
	end

	table.sort( all_entries, sortf )

	for i, entry in ipairs(all_entries) do
		entry:write_header(formatter)
		entry:write_details(formatter)
		if #entry.entry > 0 then
			formatter:headerText(l.ENTRY, entry.entry, true)
		end
		formatter:separator()
	end

	formatter:close()

end

local function displayExportOptions()
	ui.spacing()
	local c
	local flight_log = true;

	c,export_player_info = checkbox(l.PERSONAL_INFORMATION, export_player_info)
	c,export_custom_log = checkbox(l.LOG_CUSTOM, export_custom_log)
	c,export_station_log = checkbox(l.LOG_STATION, export_station_log)
	c,export_system_log = checkbox(l.LOG_SYSTEM, export_system_log)
	c,export_html = checkbox("HTML", export_html)


	ui.separator()
	ui.spacing()

	if ui.button(l.SAVE) then
		exportLogs()
	end

end


local function drawFlightHistory()
	ui.tabBarFont("#flightlog", {

		{	name = l.LOG_CUSTOM,
			draw = function()
				ui.spacing()
				-- input field for custom log:
				ui_formatter:headerText(l.LOG_NEW, "")
				ui.sameLine()
				local text, changed = ui.inputText("##inputfield", "", {"EnterReturnsTrue"})
				if changed then
					FlightLog.MakeCustomEntry(text)
				end
				ui.separator()
				displayLog(renderCustomLog)
			end },

		{	name = l.LOG_STATION,
			draw = function()
				displayLog(renderStationLog)
			end },

		{	name = l.LOG_SYSTEM,
			draw = function()
				displayLog(renderSystemLog)
			end },

			-- TODO localize
		{	name = "Export",
			draw = function()
				displayExportOptions()
			end }

	}, pionillium.heading)
end

local function drawLog ()
	ui.withFont(pionillium.body, function()
		drawFlightHistory()
	end)
end

InfoView:registerView({
	id = "captainsLog",
	name = l.FLIGHT_LOG,
	icon = ui.theme.icons.bookmark,
	showView = true,
	draw = drawLog,
	refresh = function() end,
	debugReload = function() package.reimport() end
})
