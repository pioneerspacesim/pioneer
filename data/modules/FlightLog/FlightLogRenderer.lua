
local ui = require 'pigui'
local FlightLog = require 'modules.FlightLog.FlightLog'
local Vector2 = _G.Vector2
local Color = _G.Color
local Lang = require 'Lang'
local l = Lang.GetResource("ui-core")

local icons = ui.theme.icons

local gray = Color(145, 145, 145)


local iconSize = ui.rescaleUI(Vector2(28, 28))
local buttonSpaceSize = iconSize

local FlightLogRenderer = {}


FlightLogRenderer.earliestFirst = true
FlightLogRenderer.includedSet = { System = true, Custom = true, Station = true }

local function writeLogEntry( entry, formatter, write_header )
	if write_header then
		formatter:write( entry:GetLocalizedName() ):newline()
	end

	for _, pair in ipairs( entry:GetDataPairs( FlightLogRenderer.earliestFirst ) ) do
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

local function renderLog( formatter, interactive )

	ui.spacing()
    if interactive then
        -- input field for custom log:
        ui_formatter:headerText(l.LOG_NEW, "")
        ui.sameLine()
        local text, changed = ui.inputText("##inputfield", "", {"EnterReturnsTrue"})
        if changed then
            FlightLog.MakeCustomEntry(text)
        end
        ui.separator()
    end

	local counter = 0
	for entry in FlightLog:GetLogEntries(FlightLogRenderer.includedSet, nil, FlightLogRenderer.earliestFirst ) do
	 	counter = counter + 1
	
		 writeLogEntry( entry, formatter, true )

         if interactive then
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

            if entry:SupportsDelete() then
                if ui.iconButton(icons.trashcan, buttonSpaceSize, l.REMOVE .. "##custom" .. counter) then
                    entry:Delete()
                    -- if we were already in edit mode, reset it, or else it carries over to next iteration
                    entering_text = false
                end
            end
			ui.nextColumn()
        else
            if entry:HasEntry() then
                ui_formatter:headerText(l.ENTRY, entry:GetEntry(), true)
            end
        end

		ui.separator()
		ui.spacing()
	end
    if counter == 0 then
        ui.text(l.NONE)
    end
end

function FlightLogRenderer.displayFilterOptions()
	ui.spacing()
	local c
	local flight_log = true;

	c,FlightLogRenderer.includedSet.Custom = ui.checkbox(l.LOG_CUSTOM, FlightLogRenderer.includedSet.Custom)
	c,FlightLogRenderer.includedSet.Station = ui.checkbox(l.LOG_STATION, FlightLogRenderer.includedSet.Station)
	c,FlightLogRenderer.includedSet.System = ui.checkbox(l.LOG_SYSTEM, FlightLogRenderer.includedSet.System)
	ui.separator()
	c,FlightLogRenderer.earliestFirst = ui.checkbox(l.LOG_EARLIEST_FIRST, FlightLogRenderer.earliestFirst)
end


function FlightLogRenderer.drawFlightHistory( interactive )

	ui.spacing()

    if interactive then
        -- reserve a narrow right column for edit / remove icon
        local width = ui.getContentRegion().x


        ui.columns(2, "##flightLogColumns", false)
        ui.setColumnWidth(0, width - (iconSize.x*3)/2)
        ui.setColumnWidth(1, (iconSize.x*3)/2)
    end

	renderLog(ui_formatter, interactive)
end


return FlightLogRenderer


