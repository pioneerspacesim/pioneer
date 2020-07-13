-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = require 'pigui'
local InfoView = require 'pigui/views/info-view'
local Lang = require 'Lang'
local FlightLog = require 'FlightLog'
local Format = require 'Format'

local pionillium = ui.fonts.pionillium
local colors = ui.theme.colors
local icons = ui.theme.icons

local gray = Color(145, 145, 145)

local l = Lang.GetResource("ui-core")

local iconSize = Vector2(28, 28) * (ui.screenHeight / 1200)
local buttonSpaceSize = iconSize

-- Sometimes date is empty, e.g. departure date prior to departure
local function formatDate(date)
	return date and Format.Date(date) or nil
end


local function formatCoordinates(sysp)
	return sysp.sectorX ..", "
		.. sysp.sectorY ..", "
		.. sysp.sectorZ
end

-- Format system to be of form: "Sol (0,0,0)"
local function formatsystem(sysp)
	return sysp:GetStarSystem().name .. " (" .. formatCoordinates(sysp) .. ")"
end

-- Title text is gray, followed by the variable text:
local function headerText(title, text, wrap)
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


local entering_text_custom = false
local entering_text_system = false
local entering_text_station = false

-- Display Entry text, and Edit button, to update flightlog
local function inputText(entry, counter, entering_text, log, str, clicked)
	if #entry > 0 then
		headerText(l.ENTRY, entry, true)
	end

	if clicked or entering_text == counter then
		local updated_entry, return_pressed = ui.inputText("##" ..str..counter, entry, {"EnterReturnsTrue"})
		entering_text = counter
		if return_pressed then
			log(counter, updated_entry)
			entering_text = -1
		end
	end
	ui.text("")
	return entering_text
end

-- Based on flight state, compose a reasonable string for location
local function composeLocationString(location)
	return string.interp(l["FLIGHTLOG_"..location[1]],
		{ primary_info = location[2],
		  secondary_info = location[3] or "",
		  tertiary_info = location[4] or "",})
end

local function renderCustomLog()
	local counter = 0
	local was_clicked = false
	for systemp, time, money, location, entry in FlightLog.GetCustomEntry() do
		counter = counter + 1
		-- reserve a narrow right colum for small edit/remove icon,
		ui.columns(2, "custom" .. counter, false)
		ui.setColumnWidth(0, ui.getWindowSize().x - 2*iconSize.x)

		headerText(l.DATE, formatDate(time))
		headerText(l.LOCATION, composeLocationString(location))
		headerText(l.IN_SYSTEM, formatsystem(systemp))
		headerText(l.ALLEGIANCE, systemp:GetStarSystem().faction.name)
		headerText(l.CASH, Format.Money(money))

		::input::
		entering_text_custom = inputText(entry, counter,
			entering_text_custom, FlightLog.UpdateCustomEntry, "custom", was_clicked)
		ui.nextColumn()

		was_clicked = false
		if ui.coloredSelectedIconButton(icons.pencil, buttonSpaceSize, false,
			0, colors.buttonBlue, colors.white, l.EDIT .. "##custom"..counter, iconSize) then
			was_clicked = true
			-- If edit field was clicked, we want to edit _this_ iteration's field,
			-- not next record's. Quick, behind you, velociraptor!
			goto input
		end

		if ui.coloredSelectedIconButton(icons.trashcan, buttonSpaceSize, false,
			0, colors.buttonBlue, colors.white, l.REMOVE .. "##custom" .. counter, iconSize) then
			FlightLog.DeleteCustomEntry(counter)
			-- if we were already in edit mode, reset it, or else it carries over to next iteration
			entering_text_custom = false
		end
		ui.columns(1)
		ui.separator()
	end
end

-- See comments on previous function
local function renderStationLog()
	local counter = 0
	local was_clicked = false
	for systemp, deptime, money, entry in FlightLog.GetStationPaths() do
		counter = counter + 1
		ui.columns(2, "station" .. counter, false)
		ui.setColumnWidth(0, ui.getWindowSize().x - 2*iconSize.x)

		local station_type = "FLIGHTLOG_" .. systemp:GetSystemBody().type
		headerText(l.DATE, formatDate(deptime))
		headerText(l.STATION, string.interp(l[station_type],
			{ primary_info = systemp:GetSystemBody().name, secondary_info = systemp:GetSystemBody().parent.name }))
		-- headerText(l.LOCATION, systemp:GetSystemBody().parent.name)
		headerText(l.IN_SYSTEM, formatsystem(systemp))
		headerText(l.ALLEGIANCE, systemp:GetStarSystem().faction.name)
		headerText(l.CASH, Format.Money(money))

		::input::
		entering_text_station = inputText(entry, counter,
			entering_text_station, FlightLog.UpdateStationEntry, "station", was_clicked)
		ui.nextColumn()

		was_clicked = false
		if ui.coloredSelectedIconButton(icons.pencil, buttonSpaceSize, false,
			0, colors.buttonBlue, colors.white, l.EDIT .. "##station"..counter, iconSize) then
			was_clicked = true
			goto input
		end
		ui.columns(1)
		ui.separator()
	end
end

-- See comments on previous function
local function renderSystemLog()
	local counter = 0
	local was_clicked = false
	for systemp, arrtime, deptime, entry in FlightLog.GetSystemPaths() do
		counter = counter + 1
		ui.columns(2, "system" .. counter, false)
		ui.setColumnWidth(0, ui.getWindowSize().x - 2*iconSize.x)

		headerText(l.ARRIVAL_DATE, formatDate(arrtime))
		headerText(l.DEPARTURE_DATE, formatDate(deptime))
		headerText(l.IN_SYSTEM, formatsystem(systemp))
		headerText(l.ALLEGIANCE, systemp:GetStarSystem().faction.name)

		::input::
		entering_text_system = inputText(entry, counter,
			entering_text_system, FlightLog.UpdateSystemEntry, "sys", was_clicked)
		ui.nextColumn()

		was_clicked = false
		if ui.coloredSelectedIconButton(icons.pencil, buttonSpaceSize, false,
			0, colors.buttonBlue, colors.white, l.EDIT .. "##system"..counter, iconSize) then
			was_clicked = true
			goto input
		end
		ui.columns(1)
		ui.separator()
	end
end

local function getFlightHistory()
	if ui.beginTabBar("mytabbar") then
		if ui.beginTabItem(l.LOG_CUSTOM) then
			-- input field for custom log:
			headerText(l.LOG_NEW, "")
			ui.sameLine()
			text, changed = ui.inputText("##inputfield", "", {"EnterReturnsTrue"})
			if changed then
				FlightLog.MakeCustomEntry(text)
			end
			ui.separator()

			renderCustomLog()
			ui.endTabItem()
		end

		if ui.beginTabItem(l.LOG_STATION) then
			renderStationLog()
			ui.endTabItem()
		end

		if ui.beginTabItem(l.LOG_SYSTEM) then
			renderSystemLog()
			ui.endTabItem()
		end

		ui.endTabBar()
	end
end

local function drawLog ()
	ui.withFont(pionillium.medlarge.name, pionillium.medlarge.size, function()
		getFlightHistory()
	end)
end

InfoView:registerView({
    id = "captainsLog",
    name = l.FLIGHT_LOG,
    icon = ui.theme.icons.bookmark,
    showView = true,
    draw = drawLog,
    refresh = function() end,
})
