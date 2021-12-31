-- Copyright © 2008-2022 Pioneer Developers. See AUTHORS.txt for details
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

		headerText(l.DATE, formatDate(time))
		headerText(l.LOCATION, composeLocationString(location))
		headerText(l.IN_SYSTEM, ui.Format.SystemPath(systemp))
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

		ui.nextColumn()
		ui.separator()
		ui.spacing()
	end
end

-- See comments on previous function
local function renderStationLog()
	local counter = 0
	local was_clicked = false
	for systemp, deptime, money, entry in FlightLog.GetStationPaths() do
		counter = counter + 1

		local station_type = "FLIGHTLOG_" .. systemp:GetSystemBody().type
		headerText(l.DATE, formatDate(deptime))
		headerText(l.STATION, string.interp(l[station_type],
			{ primary_info = systemp:GetSystemBody().name, secondary_info = systemp:GetSystemBody().parent.name }))
		-- headerText(l.LOCATION, systemp:GetSystemBody().parent.name)
		headerText(l.IN_SYSTEM, ui.Format.SystemPath(systemp))
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

		ui.nextColumn()
		ui.separator()
	end
end

-- See comments on previous function
local function renderSystemLog()
	local counter = 0
	local was_clicked = false
	for systemp, arrtime, deptime, entry in FlightLog.GetSystemPaths() do
		counter = counter + 1

		headerText(l.ARRIVAL_DATE, formatDate(arrtime))
		headerText(l.DEPARTURE_DATE, formatDate(deptime))
		headerText(l.IN_SYSTEM, ui.Format.SystemPath(systemp))
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

	logFn()
	ui.columns(1)
end

local function getFlightHistory()
	if ui.beginTabBar("mytabbar") then
		if ui.beginTabItem(l.LOG_CUSTOM) then
			ui.spacing()
			-- input field for custom log:
			headerText(l.LOG_NEW, "")
			ui.sameLine()
			local text, changed = ui.inputText("##inputfield", "", {"EnterReturnsTrue"})
			if changed then
				FlightLog.MakeCustomEntry(text)
			end
			ui.separator()

			displayLog(renderCustomLog)
			ui.endTabItem()
		end

		if ui.beginTabItem(l.LOG_STATION) then
			displayLog(renderStationLog)
			ui.endTabItem()
		end

		if ui.beginTabItem(l.LOG_SYSTEM) then
			displayLog(renderSystemLog)
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
	debugReload = function() package.reimport() end
})
