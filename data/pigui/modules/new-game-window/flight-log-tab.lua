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

local FlightLog = require 'modules.FlightLog.FlightLog'

-- TODO: move this into the pygui space - probably!
local FlightLogRenderer = require 'modules.FlightLog.FlightLogRenderer'


local FlightLogTab = GameParam.New(lui.FLIGHT_LOG, "flightlog")

FlightLogTab.version = 1
FlightLogTab.exclude_on_recovery = true

function FlightLogTab:fromStartVariant(variant)
	if ( variant.logmsg ) then 
		 FlightLog.MakeCustomEntry( variant.logmsg )
	end
end

function FlightLogTab:draw()
	local h = ui.getCursorPos().y

	--TODO WE MUST ADD THE FILTERING OPTIONS SOMEHOW?

	-- local changed, newValue = ui.combo("##select_log", cbCurrentValue, cbValues)
	-- if changed then cbCurrentValue = newValue end

	-- local section = sections[cbCurrentValue]
	-- local entries = self.value[section]

	-- ui.separator()

	h = ui.getCursorPos().y - h
	ui.child("flightlog", Vector2(Defs.contentRegion.x, Defs.contentRegion.y - h), function()
		FlightLogRenderer.drawFlightHistory( false )
	end)
end

local function logPathWarning( path, msg )
	local si = path.systemIndex or "nil"
	local bi = path.bodyIndex or "nil"
	logWarning( msg .. "  [" .. path.sectorX .. "," .. path.sectorY .. "," .. path.sectorZ .. ":" .. si .. ":" .. bi  .. "]" )
end

-- an invalid body path will be read as a system path only
-- an invalid system path will be read as a sector path only
local function systemPathFromTable(t)
	if not t then 
		logWarning( "No object found to fetch a path from" )	
		return nil
	end
	t = t.inner	

	-- something is completely wrong
	if not t or #t ~= 5 then 
		logWarning( "Inner path object is missing or too small" )	
		return nil 
	end

	local path = SystemPath.New(t[1], t[2], t[3], t[4], t[5])

	if path:IsSectorPath() then 
		logPathWarning( path, "Returning sector path" )	
		return path 
	end

	local system = Location:getGalaxy():GetStarSystem(path)
	if not system then
		logPathWarning( path, "Reducing system path to sector path" )	
		return SystemPath:SectorOnly()
	end
	if path:IsSystemPath() then
		logPathWarning( path, "Returning system path" )	
		return path 
	end

	local paths = system:GetBodyPaths()
	if t[5] >= #paths then
		logPathWarning( path, "Returning body path as system path" )	
		return path:SystemOnly()
	end

	local systembody = system:GetBodyByPath(path)
	if systembody.superType ~= 'STARPORT' then
		logPathWarning( path, "Returning body path as system path as it's not a starport" )	
		return path:SystemOnly()
	end

	logPathWarning( path, "Returning path" )

	return path
end

FlightLogTab.reader = Helpers.versioned {{
	version = 89,
	fnc = function(saveGame)

		local data, errorString = Helpers.getByPath(saveGame, "lua_modules_json/FlightLog")
		if errorString then return nil, errorString end

		FlightLog.ParseSavedData( data, systemPathFromTable, Helpers.getLuaClass )

		return {}
	end

}, {

	version = 90,
	fnc = function(saveGame)

		local data, errorString = Helpers.getByPath(saveGame, "lua_modules_json/FlightLog")
		if errorString then return nil, errorString end

		FlightLog.ParseSavedData( data, systemPathFromTable, Helpers.getLuaClass )

		return {}

	end
}}

FlightLogTab.updateLayout = false
FlightLogTab.updateParams = false

function FlightLogTab:isValid()
	return true
end

FlightLogTab.TabName = lui.FLIGHT_LOG

return FlightLogTab
