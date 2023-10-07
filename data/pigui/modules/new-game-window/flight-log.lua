-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txtl

local ui = require 'pigui'
local Lang = require 'Lang'
local lui = Lang.GetResource("ui-core")
local Format = require 'Format'

local Defs = require 'pigui.modules.new-game-window.defs'
local GameParam = require 'pigui.modules.new-game-window.game-param'
local Location = require 'pigui.modules.new-game-window.location'

local FlightLog = GameParam.New(lui.FLIGHT_LOG, "flightlog")

FlightLog.value = {
	Custom = {},
	System = {},
	Station = {}
}

FlightLog.version = 1

function FlightLog:fromStartVariant(variant)
	self.value.System = {}
	self.value.Station = {}
	self.value.Custom = {
		{
			text = variant.logmsg
		}
	}
end

-- Title text is gray, followed by the variable text:
local function headerText(title, text, wrap)
	if not text then return end
	ui.textColored(ui.theme.colors.grey, string.gsub(title, ":", "") .. ":")
	ui.sameLine()
	if wrap then
		ui.textWrapped(text)
	else
		ui.text(text)
	end
end

local function asFaction(path)
	if path:IsSectorPath() then return lui.UNKNOWN_FACTION end
	return Location:getGalaxy():GetStarSystem(path).faction.name
end

local function asStation(path)
	if not path:IsBodyPath() then return lui.NO_AVAILABLE_DATA end
	local system = Location:getGalaxy():GetStarSystem(path)
	local systembody = system:GetBodyByPath(path)
	local station_type = "FLIGHTLOG_" .. systembody.type
	return string.interp(lui[station_type], {
		primary_info = systembody.name,
		secondary_info = systembody.parent.name
	})
end

local function asPath(path)
	local sectorString = "(" .. path.sectorX .. ", " .. path.sectorY .. ", " .. path.sectorZ .. ")"
	if path:IsSectorPath() then
		return lui.UNKNOWN_LOCATION_IN_SECTOR_X:interp{ sector = sectorString }
	end
	local system = Location:getGalaxy():GetStarSystem(path)
	return system.name .. " " .. sectorString
end

-- Based on flight state, compose a reasonable string for location
local function composeLocationString(location)
	return string.interp(lui["FLIGHTLOG_"..location[1]], {
		primary_info = location[2],
		secondary_info = location[3] or "",
		tertiary_info = location[4] or "",
	})
end

local cbCurrentValue = 0
local cbValues = { lui.LOG_CUSTOM, lui.LOG_SYSTEM, lui.LOG_STATION }
local sections = { [0] = "Custom", [1] = "System", [2] = "Station" }
local entryTemplates = {
	Custom = {
		{ id = "time",     label = lui.DATE,           render = Format.Date },
		{ id = "location", label = lui.LOCATION,       render = composeLocationString },
		{ id = "path",     label = lui.IN_SYSTEM,      render = asPath },
		{ id = "path",     label = lui.ALLEGIANCE,     render = asFaction },
		{ id = "money",    label = lui.CASH,           render = Format.Money },
		{ id = "text",     label = lui.ENTRY,          render = tostring, wrap = true },
	},
	System = {
		{ id = "arrtime",  label = lui.ARRIVAL_DATE,   render = Format.Date },
		{ id = "deptime",  label = lui.DEPARTURE_DATE, render = Format.Date },
		{ id = "path",     label = lui.IN_SYSTEM,      render = asPath },
		{ id = "path",     label = lui.ALLEGIANCE,     render = asFaction },
		{ id = "text",     label = lui.ENTRY,          render = tostring, wrap = true },
	},
	Station = {
		{ id = "time",     label = lui.DATE,           render = Format.Date },
		{ id = "path",     label = lui.STATION,        render = asStation },
		{ id = "path",     label = lui.IN_SYSTEM,      render = asPath },
		{ id = "path",     label = lui.ALLEGIANCE,     render = asFaction },
		{ id = "money",    label = lui.CASH,           render = Format.Money },
		{ id = "text",     label = lui.ENTRY,          render = tostring, wrap = true },
	},
}

local function renderEntry(template, entry)
	for _, param in ipairs(template) do
		local value = entry[param.id]
		if value and value ~= "" then -- do not show an empty entry too
			headerText(param.label, param.render(value), param.wrap)
		end
	end
end

function FlightLog:draw()
	local h = ui.getCursorPos().y
	local changed, newValue = ui.combo("##select_log", cbCurrentValue, cbValues)
	if changed then cbCurrentValue = newValue end

	local section = sections[cbCurrentValue]
	local entries = self.value[section]

	ui.separator()

	h = ui.getCursorPos().y - h
	ui.child("flightlog", Vector2(Defs.contentRegion.x, Defs.contentRegion.y - h), function()
		if #entries == 0 then
			ui.text(lui.NONE)
			return
		end
		-- draw separators only between elements
		local sep = function() end
		for _, entry in ipairs(entries) do
			sep()
			renderEntry(entryTemplates[section], entry)
			sep = ui.separator
		end
	end)
end

FlightLog.updateLayout = false
FlightLog.updateView = false

function FlightLog:isValid()
	return true
end

FlightLog.TabName = lui.FLIGHT_LOG

return FlightLog
