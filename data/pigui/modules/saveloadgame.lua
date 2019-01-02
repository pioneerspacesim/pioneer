-- Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import('Engine')
local Game = import('Game')
local ui = import('pigui/pigui.lua')
local Vector = import('Vector')
local Event = import('Event')
local Lang = import("Lang")
local Color = import("Color")
local lc = Lang.GetResource("core")
local lui = Lang.GetResource("ui-core")
local FileSystem = import("FileSystem")
local Format = import("Format")

local utils = import("utils")

local player = nil
local colors = ui.theme.colors
local icons = ui.theme.icons
local pionillium = ui.fonts.pionillium

local mainButtonSize = Vector(40,40) * (ui.screenHeight / 1200)
local optionButtonSize = Vector(125,40) * (ui.screenHeight / 1200)
local bindingButtonSize = Vector(135,25) * (ui.screenHeight / 1200)
local mainButtonFramePadding = 3

local saveFileCache = {}
local selectedSave

local function optionTextButton(label, tooltip, enabled, callback)
	local bgcolor = enabled and colors.buttonBlue or colors.grey

    local button
	ui.withFont(pionillium.medium.name, pionillium.medium.size, function()
		button = ui.coloredSelectedButton(label, optionButtonSize, false, bgcolor, tooltip, enabled)
	end)
    if button then
        callback(button)
    end
end

local function getSaveTooltip(name)
	local ret
	local stats
	if not saveFileCache[name] then
        local ok
        ok, saveFileCache[name] = pcall(Game.SaveGameStats, name)
	end
    stats = saveFileCache[name]
    if (type(stats) == "string") then -- file could not be loaded, this is the error
        return stats
    end
	ret = lui.GAME_TIME..":    " .. Format.Date(stats.time)
	if stats.system then    ret = ret .. "\n"..lc.SYSTEM..": " .. stats.system end
	if stats.credits then   ret = ret .. "\n"..lui.CREDITS..": " .. Format.Money(stats.credits) end
	if stats.ship   then    ret = ret .. "\n"..lc.SHIP..": " .. stats.ship end
	if stats.flight_state then
		ret = ret .. "\n"..lui.FLIGHT_STATE..": "
		if stats.flight_state == "docked" then ret = ret .. lc.DOCKED
		elseif stats.flight_state == "docking" then ret = ret .. lc.DOCKING
		elseif stats.flight_state == "flying" then ret = ret .. lui.FLYING
		elseif stats.flight_state == "hyperspace" then ret = ret .. lc.HYPERSPACE
		elseif stats.flight_state == "jumping" then ret = ret .. lui.JUMPING
		elseif stats.flight_state == "landed" then ret = ret .. lc.LANDED
		elseif stats.flight_state == "undocking" then ret = ret .. lc.UNDOCKING
		else ret = ret .. lc.UNKNOWN end
	end

	if stats.docked_at then ret = ret .. "\n"..lui.DOCKED_AT..": " .. stats.docked_at end
	if stats.frame then ret = ret .. "\n"..lui.VICINITY_OF..": " .. stats.frame end

	return ret
end

local function showSaveFiles()
	local ok, files, _ = pcall(FileSystem.ReadDirectory, "USER","savefiles")
	if not ok then
		print('Error: ' .. files)
		saveFileCache = {}
	else
		table.sort(files, function(a,b) return (a.mtime.timestamp > b.mtime.timestamp) end)
		ui.columns(2,"##saved_games",true)
		for _,f in pairs(files) do
			if ui.selectable(f.name, f.name == selectedSave, {"SpanAllColumns"}) then
				selectedSave = f.name
			end
			if Engine.pigui.IsItemHovered() then
				local tooltip = getSaveTooltip(f.name)
				Engine.pigui.SetTooltip(tooltip)
			end

			ui.nextColumn()
			ui.text(Format.Date(f.mtime.timestamp))
			ui.nextColumn()
		end
		ui.columns(1,"",false)
	end
end

local function closeAndClearCache()
	ui.showSavedGameWindow = nil
	saveFileCache = {}
end

local function closeAndLoadOrSave()
	if selectedSave ~= nil and selectedSave ~= '' then
		if ui.showSavedGameWindow == "LOAD" then
			Game.LoadGame(selectedSave)
		elseif ui.showSavedGameWindow == "SAVE" then
			Game.SaveGame(selectedSave)
		end
		closeAndClearCache()
	end
end

local function savedGameWindow()
	if ui.showSavedGameWindow then
		ui.setNextWindowPosCenter('Always')
		ui.withStyleColors({["WindowBg"] = Color(20, 20, 80, 230)}, function()
			-- TODO: this window should be ShowBorders
			ui.window("LoadGame", {"NoTitleBar", "NoResize", "AlwaysAutoResize"}, function()
				local mode
				mode = ui.showSavedGameWindow == 'SAVE' and lui.SAVE or lui.LOAD
				optionTextButton(mode, nil, selectedSave~=nil, closeAndLoadOrSave)
				ui.sameLine()
				optionTextButton(lui.CANCEL, nil, true, closeAndClearCache)

				if ui.showSavedGameWindow == "SAVE" then
					selectedSave = ui.inputText("##saveFileName", selectedSave or "", {})
				end

				showSaveFiles()
			end)
		end)
	end
end

ui.registerModule("game", savedGameWindow)
ui.registerModule("mainMenu", savedGameWindow)

return {}
