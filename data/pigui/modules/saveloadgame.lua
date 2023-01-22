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

local saveFileCache = {}
local selectedSave
local saveIsValid = true

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
			if ui.selectable(f.name, f.name == selectedSave, {"SpanAllColumns", "DontClosePopups"}) then
				selectedSave = f.name
				saveIsValid = pcall(Game.SaveGameStats, f.name)
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
	ui.saveLoadWindow:close()
	ui.saveLoadWindow.mode = nil
	saveFileCache = {}
	popupOpened = false
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

ui.saveLoadWindow = ModalWindow.New("LoadGame", function()
	local saving = ui.saveLoadWindow.mode == "SAVE"
	local other_height = optionButtonSize.y + ui.getItemSpacing().y * 2 + ui.getWindowPadding().y * 2
	ui.child("savefiles", Vector2(-1, winSize.y - other_height), function()
		showSaveFiles()
	end)

	ui.separator()

	local txt_width = winSize.x - (ui.getWindowPadding().x + optionButtonSize.x + ui.getItemSpacing().x) * 2
	if saving then
		-- for vertical center alignment
		local txt_hshift = math.max(0, (optionButtonSize.y - ui.getFrameHeight()) / 2)
		ui.nextItemWidth(txt_width, 0)
		ui.addCursorPos(Vector2(0, txt_hshift))
		selectedSave = ui.inputText("##saveFileName", selectedSave or "", {})
		ui.sameLine(txt_width + ui.getWindowPadding().x + ui.getItemSpacing().x)
		ui.addCursorPos(Vector2(0, -txt_hshift))
	else
		ui.addCursorPos(Vector2(txt_width + ui.getItemSpacing().x, 0))
	end

	local mode = saving and lui.SAVE or lui.LOAD
	optionTextButton(mode, selectedSave ~= nil and selectedSave ~= '' and saveIsValid, closeAndLoadOrSave)
	ui.sameLine()
	optionTextButton(lui.CANCEL, true, closeAndClearCache)

end, function (_, drawPopupFn)
	ui.setNextWindowSize(winSize, "Always")
	ui.setNextWindowPosCenter('Always')
	ui.withStyleColors({ PopupBg = ui.theme.colors.modalBackground }, drawPopupFn)
end)

ui.saveLoadWindow.mode = "LOAD"

return {}
