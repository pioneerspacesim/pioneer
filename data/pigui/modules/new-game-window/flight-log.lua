-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txtl

local ui = require 'pigui'
local Lang = require 'Lang'
local lui = Lang.GetResource("ui-core")
local Format = require 'Format'
local SystemPath = require 'SystemPath'

local Defs = require 'pigui.modules.new-game-window.defs'
local GameParam = require 'pigui.modules.new-game-window.game-param'
local Location = require 'pigui.modules.new-game-window.location'
local Helpers = require 'pigui.modules.new-game-window.helpers'

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
		variant.logmsg and { entry = variant.logmsg }
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
		{ id = "entry",    label = lui.ENTRY,          render = tostring, wrap = true },
	},
	System = {
		{ id = "arrtime",  label = lui.ARRIVAL_DATE,   render = Format.Date },
		{ id = "deptime",  label = lui.DEPARTURE_DATE, render = Format.Date },
		{ id = "path",     label = lui.IN_SYSTEM,      render = asPath },
		{ id = "path",     label = lui.ALLEGIANCE,     render = asFaction },
		{ id = "entry",    label = lui.ENTRY,          render = tostring, wrap = true },
	},
	Station = {
		{ id = "deptime",  label = lui.DATE,           render = Format.Date },
		{ id = "path",     label = lui.STATION,        render = asStation },
		{ id = "path",     label = lui.IN_SYSTEM,      render = asPath },
		{ id = "path",     label = lui.ALLEGIANCE,     render = asFaction },
		{ id = "money",    label = lui.CASH,           render = Format.Money },
		{ id = "entry",    label = lui.ENTRY,          render = tostring, wrap = true },
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

-- an invalid body path will be read as a system path only
-- an invalid system path will be read as a sector path only
local function systemPathFromTable(t)

	-- something is completely wrong
	if not t or #t ~= 5 then return nil end

	local path = SystemPath.New(t[1], t[2], t[3], t[4], t[5])

	if path:IsSectorPath() then return path end

	local system = Location:getGalaxy():GetStarSystem(path)
	if not system then
		return SystemPath:SectorOnly()
	end
	if path:IsSystemPath() then return path end

	local paths = system:GetBodyPaths()
	if t[5] >= #paths then
		return path:SystemOnly()
	end

	local systembody = system:GetBodyByPath(path)
	if systembody.superType ~= 'STARPORT' then
		return path:SystemOnly()
	end

	return path
end

FlightLog.reader = Helpers.versioned {{
	version = 89,
	fnc = function(saveGame)

		local value = { Custom = {}, System = {}, Station = {} }

		local custom, errorString = Helpers.getByPath(saveGame, "lua_modules_json/FlightLog/Custom")
		if errorString then return nil, errorString end
		assert(custom)
		for _, entry in ipairs(custom) do
			local parsed = {}
			if entry then
				parsed.time = entry[2]
				local loc = entry[4]
				parsed.location = loc and { loc[1], loc[2], loc[3] }
				parsed.path = systemPathFromTable(entry[1].inner)
				parsed.money = entry[3]
				parsed.entry = entry[5]
			end
			if not entry or not parsed.time or not parsed.location or not parsed.path or not parsed.money or not parsed.entry then
				return nil, lui.UNKNOWN_CUSTOM_LOG_ENTRY_FORMAT
			end
			table.insert(value.Custom, parsed)
		end

		local system
		system, errorString = Helpers.getByPath(saveGame, "lua_modules_json/FlightLog/System")
		if errorString then return nil, errorString end
		assert(system)
		for _, entry in ipairs(system) do
			local parsed = {}
			if entry then
				parsed.arrtime = entry[2]
				parsed.deptime = entry[3]
				parsed.path = systemPathFromTable(entry[1].inner)
				parsed.entry = entry[4]
			end
			if not entry or not parsed.path then
				return nil, lui.UNKNOWN_SYSTEM_LOG_ENTRY_FORMAT
			end
			table.insert(value.System, parsed)
		end

		local station
		station, errorString = Helpers.getByPath(saveGame, "lua_modules_json/FlightLog/Station")
		if errorString then return nil, errorString end
		assert(station)
		for _, entry in ipairs(station) do
			local parsed = {}
			if entry then
				parsed.deptime = entry[2]
				parsed.path = systemPathFromTable(entry[1].inner)
				parsed.money = entry[3]
				parsed.entry = entry[4]
			end
			if not entry or not parsed.deptime or not parsed.path or not parsed.money then
				return nil, lui.UNKNOWN_STATION_LOG_ENTRY_FORMAT
			end
			table.insert(value.Station, parsed)
		end

		return value
	end

}, {

	version = 90,
	fnc = function(saveGame)

		local function entryWithSafeSystemPath(entry)
			local parsed = {}
			for k,v in pairs(entry) do
				if k == 'systemp' then
					parsed.path = systemPathFromTable(entry.systemp.inner)
				else
					parsed[k] = v
				end
			end
			return parsed
		end

		local value = { Custom = {}, System = {}, Station = {} }

		local logData, errorString = Helpers.getByPath(saveGame, "lua_modules_json/FlightLog/Data")
		if errorString then return nil, errorString end

		for _, entry in ipairs(logData) do
			local entryType = Helpers.getLuaClass(entry)

			if entryType == 'FlightLogEntry.Custom' then
				table.insert(value.Custom, entryWithSafeSystemPath(entry))
			elseif entryType == 'FlightLogEntry.System' then
				table.insert(value.System, entryWithSafeSystemPath(entry))
			elseif entryType == 'FlightLogEntry.Station' then
				table.insert(value.Station, entryWithSafeSystemPath(entry))
			else
				return nil, lui.UNKNOWN_STATION_LOG_ENTRY_FORMAT
			end
		end
		return value
	end
}}

FlightLog.updateLayout = false
FlightLog.updateParams = false

function FlightLog:isValid()
	return true
end

FlightLog.TabName = lui.FLIGHT_LOG

return FlightLog
