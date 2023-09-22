-- Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local FlightLog = require 'modules.FlightLog.FlightLog'
local FileSystem = require 'FileSystem'
local Character = require 'Character'
local Lang = require 'Lang'
local l = Lang.GetResource("ui-core")

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
	self.file = nil
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
	self.file = nil
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

local Exporter = {}

---@param included_types table[string,boolean]      Keys are log types to include, set the boolean to true for the ones you want
---@param earliest_first boolean                    Should the log start with the oldest entry or the most recent
---@param player_info    boolean                    Should the log include the current player status at the top?
---@param export_html    boolean                    true for HTML, false for plain text
function Exporter.Export( included_types, earliest_first, player_info, export_html )

	FileSystem.MakeDirectory( "user://player_logs" )

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

	local log_filename = FileSystem.JoinPath( "player_logs", base_save_name .. extension )

	formatter:open( FileSystem.Open( "user://" .. log_filename, "w" ) )

    if player_info then
        formatter:headerText( l.NAME_PERSON, player.name )
        -- TODO: localize
        formatter:headerText( "Title", player.title )
        formatter:headerText( l.RATING, l[player:GetCombatRating()] )
        formatter:headerText( l.KILLS,  string.format('%d',player.killcount) )
        formatter:separator()
        formatter:newline()
    end

	for entry in FlightLog:GetLogEntries(included_types,nil, earliest_first) do

        formatter:write( entry:GetLocalizedName() ):newline()
        for _, pair in pairs( entry:GetDataPairs( earliest_first ) ) do
            formatter:headerText( pair[1], pair[2] )
        end

		if (entry:HasEntry()) then
			formatter:headerText(l.ENTRY, entry:GetEntry(), true)
		end
		formatter:separator()
	end

	formatter:close()

end

return Exporter