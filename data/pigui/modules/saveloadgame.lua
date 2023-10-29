-- Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Game = require 'Game'
local ShipDef = require 'ShipDef'
local FileSystem = require 'FileSystem'
local Format = require 'Format'

local Lang = require 'Lang'
local lc = Lang.GetResource("core")
local lui = Lang.GetResource("ui-core")
local Vector2 = _G.Vector2
local Color = _G.Color

local ui = require 'pigui'
local ModalWindow = require 'pigui.libs.modal-win'

local optionButtonSize = ui.rescaleUI(Vector2(100, 32))
local winSize = Vector2(ui.screenWidth * 0.4, ui.screenHeight * 0.6)
local pionillium = ui.fonts.pionillium

local searchText = lc.SEARCH .. ':'
local saveText = lui.SAVE .. ':'
local caseSensitiveText = lui.CASE_SENSITIVE

local saveFileCache = {}
local selectedSave
local saveIsValid = true

local minSearchTextLength = 1
local searchSave = ""
local caseSensitive = false

local function optionTextButton(label, enabled, callback)
	local variant = not enabled and ui.theme.buttonColors.disabled or nil
	local button
	ui.withFont(pionillium.medium.name, pionillium.medium.size, function()
		button = ui.button(label, optionButtonSize, variant)
	end)
	if button then
		callback(button)
	end
end

local function getSaveTooltip(name)
	local ret
	local stats
	if not saveFileCache[name] then
		_, saveFileCache[name] = pcall(Game.SaveGameStats, name)
	end
	stats = saveFileCache[name]
	if (type(stats) == "string") then -- file could not be loaded, this is the error
		return stats
	end
	ret = lui.GAME_TIME..":    " .. Format.Date(stats.time)
	local ship = stats.ship and ShipDef[stats.ship]

	if stats.system then ret = ret .. "\n"..lc.SYSTEM..": " .. stats.system end
	if stats.credits then ret = ret .. "\n"..lui.CREDITS..": " .. Format.Money(stats.credits) end

	if ship then
		ret = ret .. "\n"..lc.SHIP..": " .. ship.name
	else
		ret = ret .. "\n" .. lc.SHIP .. ": " .. lc.UNKNOWN
	end


	if stats.flight_state then
		ret = ret .. "\n"..lui.FLIGHT_STATE..": "
		ret = ret .. (rawget(lc, string.upper(stats.flight_state)) or
		rawget(lui, string.upper(stats.flight_state)) or
		lc.UNKNOWN)
	end

	if stats.docked_at then ret = ret .. "\n"..lui.DOCKED_AT..": " .. stats.docked_at end
	if stats.frame then ret = ret .. "\n"..lui.VICINITY_OF..": " .. stats.frame end

	saveFileCache[name].ret = ret
	return ret
end

local function shouldDisplayThisSave(f)
    if(string.len(searchSave) < minSearchTextLength) then
	   return true
	end

	return not caseSensitive and  string.find(string.lower(f.name), string.lower(searchSave), 1, true) ~= nil or
	string.find(f.name, searchSave, 1, true) ~= nil
end


local function closeAndClearCache()
	ui.saveLoadWindow:close()
	ui.saveLoadWindow.mode = nil
	saveFileCache = {}
	searchSave = ""
end

local function closeAndLoadOrSave()
	if selectedSave ~= nil and selectedSave ~= '' then
		if ui.saveLoadWindow.mode == "LOAD" and saveIsValid then
			Game.LoadGame(selectedSave)
			closeAndClearCache()
		elseif ui.saveLoadWindow.mode == "SAVE" then
			Game.SaveGame(selectedSave)
			closeAndClearCache()
		end
	end
end


local function displaySave(f)
	if ui.selectable(f.name, f.name == selectedSave, {"SpanAllColumns", "DontClosePopups", "AllowDoubleClick"}) then
		selectedSave = f.name
	 	saveIsValid = pcall(Game.SaveGameStats, f.name)
	 	if ui.isMouseDoubleClicked(0) then
			closeAndLoadOrSave()	
		end
	end

	if ui.isItemHovered("ForTooltip") then
		ui.setTooltip(getSaveTooltip(f.name))
	end


	ui.nextColumn()
	ui.text(Format.Date(f.mtime.timestamp))
	ui.nextColumn()
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
		    if(shouldDisplayThisSave(f)) then
				displaySave(f)
			end
		end
		ui.columns(1,"",false)
	end
end

local function drawSearchHeader(txt_width)
	ui.withFont(pionillium.medium.name, pionillium.medium.size, function()
		ui.text(searchText)
		ui.nextItemWidth(txt_width, 0)
		searchSave, _ = ui.inputText("##searchSave", searchSave, {})
		ui.sameLine()
		local ch, value = ui.checkbox(caseSensitiveText, caseSensitive)
		if ch then
			caseSensitive = value
		end
	end)
end

local function drawOptionButtons(txt_width, saving)

	-- for vertical center alignment
	local txt_hshift = math.max(0, (optionButtonSize.y - ui.getFrameHeight()) / 2)
	local mode = saving and lui.SAVE or lui.LOAD
	ui.sameLine(txt_width + ui.getWindowPadding().x + ui.getItemSpacing().x)
	ui.addCursorPos(Vector2(0, saving and -txt_hshift or txt_hshift))
	optionTextButton(mode, selectedSave ~= nil and selectedSave ~= '' and saveIsValid, closeAndLoadOrSave)
	ui.sameLine()
	ui.addCursorPos(Vector2(0, saving and -txt_hshift or txt_hshift))
	optionTextButton(lui.CANCEL, true, closeAndClearCache)
end

ui.saveLoadWindow = ModalWindow.New("LoadGame", function()
	local saving = ui.saveLoadWindow.mode == "SAVE"
	local searchTextSize = ui.calcTextSize(searchText, pionillium.medium.name, pionillium.medium.size)

	local txt_width = winSize.x - (ui.getWindowPadding().x + optionButtonSize.x + ui.getItemSpacing().x) * 2

	drawSearchHeader(txt_width)

	ui.separator()

	local saveFilesSearchHeaderHeight = (searchTextSize.y * 2 + ui.getItemSpacing().y * 2 + ui.getWindowPadding().y * 2)
	local saveFilesChildWindowHeight = (optionButtonSize.y + (saving and searchTextSize.y or 0) + ui.getItemSpacing().y * 2 + ui.getWindowPadding().y * 2)

	local saveFilesChildWindowSize = Vector2(0, (winSize.y - saveFilesChildWindowHeight) - saveFilesSearchHeaderHeight)

	ui.child("savefiles", saveFilesChildWindowSize, function()
		showSaveFiles()
	end)

	ui.separator()

	-- a little padding just before the window border, so that the cancel button will not be cut out
	txt_width = txt_width / 1.03
	if saving then
		ui.withFont(pionillium.medium.name, pionillium.medium.size, function()
			ui.text(saveText)
			ui.nextItemWidth(txt_width, 0)
			selectedSave = ui.inputText("##saveFileName", selectedSave or "", {})
		end)
	end
	drawOptionButtons(txt_width, saving)

end, function (_, drawPopupFn)
	ui.setNextWindowSize(winSize, "Always")
	ui.setNextWindowPosCenter('Always')
	ui.withStyleColors({ PopupBg = ui.theme.colors.modalBackground }, drawPopupFn)
end)

ui.saveLoadWindow.mode = "LOAD"

return {}
