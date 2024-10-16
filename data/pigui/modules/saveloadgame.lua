-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game       = require 'Game'
local Event      = require 'Event'
local FileSystem = require 'FileSystem'
local Lang       = require 'Lang'
local ShipDef    = require 'ShipDef'
local Vector2    = _G.Vector2

local utils = require 'utils'

local lc  = Lang.GetResource("core")
local lui = Lang.GetResource("ui-core")

local ui = require 'pigui'

local Notification = require 'pigui.libs.notification'
local ModalWindow = require 'pigui.libs.modal-win'
local MessageBox = require 'pigui.libs.message-box'
local NewGameWindow = require 'pigui.modules.new-game-window.class'

local minSearchTextLength = 1

local headingFont = ui.fonts.orbiteer.title
local mainButtonFont = ui.fonts.pionillium.body
local bodyFont = ui.fonts.pionillium.body
local detailsFont = ui.fonts.pionillium.details

local colors = ui.theme.colors
local icons = ui.theme.icons
local cardBackgroundCol = ui.theme.buttonColors.card
local cardSelectedCol = ui.theme.buttonColors.card_selected

local style = ui.rescaleUI {
	windowPadding = Vector2(16, 16),
	popupPadding = Vector2(120, 90),
	itemSpacing = Vector2(8, 16),
	mainButtonSize = Vector2(100, 0) -- button height will be calculated from the font height
}

local iconColor = colors.fontDim

--==============================================================================

local SaveGameEntry = utils.proto()

SaveGameEntry.name = ""
SaveGameEntry.character = lc.UNKNOWN
SaveGameEntry.shipName = ""
SaveGameEntry.shipHull = lc.UNKNOWN
SaveGameEntry.credits = 0
SaveGameEntry.locationName = lc.UNKNOWN
SaveGameEntry.gameTime = 0
SaveGameEntry.duration = 0
SaveGameEntry.timestamp = 0
SaveGameEntry.isAutosave = false
SaveGameEntry.compatible = true

--==============================================================================

local SaveLoadWindow = ModalWindow.New("SaveGameWindow")

SaveLoadWindow.Modes = {
	Load = "LOAD",
	Save = "SAVE"
}

SaveLoadWindow.mode = SaveLoadWindow.Modes.Load
SaveLoadWindow.selectedFile = nil
SaveLoadWindow.searchStr = ""
SaveLoadWindow.savePath = ""
SaveLoadWindow.entryCache = {}
SaveLoadWindow.caseSensitive = false
SaveLoadWindow.showAutosaves = false

--==============================================================================

local function mainButton(label, enabled, tooltip)
	local variant = not enabled and ui.theme.buttonColors.disabled or nil
	local ret = ui.button(label, style.mainButtonSize, variant, tooltip)
	return enabled and ret
end

local function drawSaveEntryDetails(entry)
	local iconSize = Vector2(detailsFont.size)

	-- Ship and current credits
	ui.tableSetColumnIndex(1)
	ui.withFont(detailsFont, function()
		ui.alignTextToLineHeight(bodyFont.size)
		ui.icon(icons.ships_no_orbits, iconSize, iconColor)
		ui.sameLine()
		if #entry.shipName > 0 then
			ui.text(entry.shipHull .. ": " .. entry.shipName)
		else
			ui.text(entry.shipHull)
		end

		ui.icon(icons.money, iconSize, iconColor)
		ui.sameLine()
		ui.text(ui.Format.Number(entry.credits, 0) .. " " .. lc.UNIT_CREDITS)
	end)

	-- Location and time
	ui.tableSetColumnIndex(2)
	ui.withFont(detailsFont, function()
		ui.alignTextToLineHeight(bodyFont.size)
		ui.icon(icons.navtarget, iconSize, iconColor)
		ui.sameLine()
		ui.text(entry.locationName)

		ui.icon(icons.eta, iconSize, iconColor)
		ui.sameLine()
		ui.text(ui.Format.Datetime(entry.gameTime))
	end)

	-- Time spent playing the savefile and last played date
	ui.tableSetColumnIndex(3)
	ui.withFont(detailsFont, function()
		ui.alignTextToLineHeight(bodyFont.size)
		ui.icon(icons.eta, iconSize, iconColor)
		ui.sameLine()
		ui.text(ui.Format.Duration(entry.duration))

		ui.icon(icons.new, iconSize, iconColor)
		ui.sameLine()
		ui.text(ui.Format.Datetime(entry.timestamp))
	end)
end

local function drawSaveEntryRow(entry, selected)
	local pos = ui.getCursorScreenPos()
	local size = ui.getContentRegion()
	local padding = ui.theme.styles.ButtonPadding
	local spacing = ui.theme.styles.ItemInnerSpacing

	local contentHeight = spacing.y + ui.fonts.pionillium.body.size + ui.fonts.pionillium.details.size
	size.y = padding.y * 2 + contentHeight

	local clicked = ui.invisibleButton("##SaveGameEntry#" .. entry.name, size, { "PressedOnClickRelease", "PressedOnDoubleClick" })
	local hovered = ui.isItemHovered()
	local active = ui.isItemActive()

	ui.setCursorScreenPos(pos)

	-- Draw the background of the whole save file
	local backgroundColor = selected and cardSelectedCol or cardBackgroundCol
	ui.addRectFilled(pos, pos + size, ui.getButtonColor(backgroundColor, hovered, active), 0, 0)

	local indicatorColor = ui.theme.styleColors.danger_900

	if entry.compatible then
		indicatorColor = entry.isAutosave and ui.theme.styleColors.gray_400 or ui.theme.styleColors.primary_400
	end

	-- Indicator bar for compatible / autosave / incompatible saves
	ui.addRectFilled(pos, pos + Vector2(spacing.x, size.y), indicatorColor, 0, 0)

	ui.withStyleVars({ CellPadding = padding, ItemSpacing = ui.theme.styles.ItemInnerSpacing }, function()

		if ui.beginTable("##SaveGameEntry", 4) then

			ui.tableNextRow()

			ui.tableSetColumnIndex(0)

			local iconStart = ui.getCursorScreenPos()

			-- Draw the leading icon spanning both rows of details
			ui.addIconSimple(iconStart + Vector2(padding.x, 0), icons.medium_courier, Vector2(contentHeight, contentHeight), colors.font)
			ui.dummy(Vector2(padding.x + contentHeight, contentHeight))
			ui.sameLine()

			-- Draw the save name and character name
			ui.group(function()
				ui.withFont(bodyFont, function()
					ui.text(entry.name)
				end)

				ui.withFont(detailsFont, function()
					ui.icon(icons.personal, Vector2(detailsFont.size), iconColor)
					ui.sameLine()
					ui.text(entry.character)
				end)
			end)

			-- Draw all of the other details
			drawSaveEntryDetails(entry)

			ui.endTable()
		end

	end)

	return clicked
end

--=============================================================================

-- Event callback once the savegame information has been loaded
-- Updates the entryCache with the full details of the savegame.
function SaveLoadWindow:onSaveGameStats(saveInfo)
	-- local profileEndScope = utils.profile("SaveLoadWindow:onSaveGameStats()")
	local location = saveInfo.system or lc.UNKNOWN
	if saveInfo.docked_at then
		location = location .. ", " .. saveInfo.docked_at
	end

	-- Old saves store only the name of the ship's *model* file for some dumb reason
	-- Treat the model name as the ship id and otherwise ignore it if we have proper data
	local shipHull
	if not saveInfo.shipHull then
		local shipDef = ShipDef[saveInfo.ship]
		if shipDef then
			shipHull = shipDef.name
		end
	else
		shipHull = saveInfo.shipHull
	end

	local entry = self.entryCache[saveInfo.filename]
	entry.character = saveInfo.character
	entry.compatible = saveInfo.compatible
	entry.credits = saveInfo.credits
	entry.duration = saveInfo.duration
	entry.gameTime = saveInfo.time
	entry.locationName = location
	entry.shipName = saveInfo.shipName
	entry.shipHull = shipHull

	-- profileEndScope()
end

local function onSaveGameStats(saveInfo)
	ui.saveLoadWindow:onSaveGameStats(saveInfo)
end

-- Trigger load of savegame information and return bare-bones entry
local function makeEntryForSave(file)
	-- local profileEndScope = utils.profile("makeEntryForSave()")
	Game.SaveGameStats(file.name)

	local saveEntry = SaveGameEntry:clone({
		name = file.name,
		isAutosave = file.name:sub(1, 1) == "_",
		timestamp = file.mtime.timestamp,
	})

	-- profileEndScope()
	return saveEntry
end

function SaveLoadWindow:getSaveEntry(filename)
	return self.entryCache[filename] or SaveGameEntry
end

--=============================================================================

function SaveLoadWindow:makeFilteredList()
	-- local profileEndScope = utils.profile("SaveLoadWindow::makeFilteredList()")
	local shouldShow = function(f)
		if not self.showAutosaves and f.name:sub(1, 1) == "_" then
			return false
		end

		if #self.searchStr < minSearchTextLength then
			return true
		end

		if self.caseSensitive then
			return f.name:find(self.searchStr, 1, true) ~= nil
		else
			return f.name:lower():find(self.searchStr:lower(), 1, true) ~= nil
		end
	end

	local isSelectedFile = function(f) return f.name == self.selectedFile end

	self.filteredFiles = utils.filter_array(self.files, shouldShow)

	if not utils.contains_if(self.filteredFiles, isSelectedFile) then
		self.selectedFile = nil
	end
	-- profileEndScope()
end

function SaveLoadWindow:makeFileList()
	-- local profileEndScope = utils.profile("SaveLoadWindow::makeFileList()")
	local ok, files = pcall(Game.ListSaves)

	if not ok then
		Notification.add(Notification.Type.Error, lui.COULD_NOT_LOAD_SAVE_FILES, files --[[@as string]])
		self.files = {}
	else
		self.files = files
	end

	table.sort(self.files, function(a, b)
		return a.mtime.timestamp > b.mtime.timestamp
	end)

	-- Cache details about each savefile
	for _, file in ipairs(self.files) do
		if not self.entryCache[file.name] or self.entryCache[file.name].timestamp ~= file.mtime.timestamp then
			self.entryCache[file.name] = makeEntryForSave(file)
		end
	end

	self:makeFilteredList()
	-- profileEndScope()
end

function SaveLoadWindow:loadSelectedSave()
	local success, err = pcall(Game.LoadGame, self.selectedFile)

	if not success then
		Notification.add(Notification.Type.Error, lui.COULD_NOT_LOAD_GAME .. self.selectedFile, err)
	else
		self:close()
	end
end

function SaveLoadWindow:recoverSelectedSave()
	local ok, report = NewGameWindow.restoreSaveGame(self.selectedFile)

	if ok then
		self:close()
		NewGameWindow.mode = 'RECOVER'
		NewGameWindow:open()
	end

	MessageBox.OK(report)
end

function SaveLoadWindow:saveToFilePath()
	local success, err = pcall(Game.SaveGame, self.savePath)

	if not success then
		Notification.add(Notification.Type.Error, lui.COULD_NOT_SAVE_GAME .. self.savePath, err)
	else
		self:close()
	end
end

function SaveLoadWindow:deleteSelectedSave()
	local savefile = self.selectedFile

	MessageBox.OK_CANCEL(lui.DELETE_SAVE_CONFIRMATION, function()
		if Game.DeleteSave(savefile) then
			Notification.add(Notification.Type.Info, lui.SAVE_DELETED_SUCCESSFULLY)
		else
			Notification.add(Notification.Type.Error, lui.COULD_NOT_DELETE_SAVE)
		end

		-- List of files changed on disk, need to update our copy of it
		self:makeFileList()
	end)
end

--=============================================================================

function SaveLoadWindow:onOpen()
	self.selectedFile = nil
	self.savePath = ""
	self:makeFileList()
end

function SaveLoadWindow:onFileSelected(filename)
	self.selectedFile = filename
	self.savePath = filename
end

function SaveLoadWindow:onSavePathChanged(savePath)
	self.savePath = savePath
	self.selectedFile = nil
end

function SaveLoadWindow:onFilterChanged(filter)
	self.searchStr = filter
	self:makeFilteredList()
end

function SaveLoadWindow:onSetCaseSensitive(cs)
	self.caseSensitive = cs

	if #self.searchStr >= minSearchTextLength then
		self:makeFilteredList()
	end
end

function SaveLoadWindow:onSetShowAutosaves(sa)
	self.showAutosaves = sa
	self:makeFilteredList()
end

--=============================================================================

function SaveLoadWindow:drawSearchHeader()
	ui.withFont(headingFont, function()
		ui.text(lui.SAVED_GAMES)
	end)

	-- Draw search bar icon
	ui.sameLine(0, style.windowPadding.x)
	ui.icon(icons.search_lens, Vector2(headingFont.size), colors.font, lc.SEARCH)

	local icon_size = Vector2(headingFont.size)
	local icon_size_spacing = icon_size.x + ui.getItemSpacing().x

	-- Draw search bar text entry
	ui.withFont(bodyFont, function()
		local height = ui.getFrameHeight()

		ui.sameLine()
		ui.addCursorPos(Vector2(0, (ui.getLineHeight() - height) / 2.0))

		ui.nextItemWidth(ui.getContentRegion().x - (icon_size_spacing * 2))
		local searchStr, changed = ui.inputText("##searchStr", self.searchStr, {})

		if changed then
			self:message("onFilterChanged", searchStr)
		end
	end)

	ui.sameLine()

	-- Draw case-sensitive toggle
	local case_sens_variant = not self.caseSensitive and ui.theme.buttonColors.transparent
	if ui.iconButton("case_sensitive", icons.case_sensitive, lui.CASE_SENSITIVE, case_sens_variant, icon_size) then
		self:message("onSetCaseSensitive", not self.caseSensitive)
	end

	ui.sameLine()

	local autosave_variant = not self.showAutosaves and ui.theme.buttonColors.transparent
	if ui.iconButton("show_autosaves", icons.view_internal, lui.SHOW_AUTOSAVES, autosave_variant, icon_size) then
		self:message("onSetShowAutosaves", not self.showAutosaves)
	end
end

function SaveLoadWindow:drawSaveFooter()
	local buttonWidths = (style.itemSpacing.x + style.mainButtonSize.x) * 3

	-- Draw savefile text entry
	if self.mode == SaveLoadWindow.Modes.Save then
		ui.alignTextToButtonPadding()
		ui.text(lui.SAVE_AS .. ":")

		ui.sameLine()
		ui.nextItemWidth(ui.getContentRegion().x - buttonWidths)

		local savePath, confirmed

		ui.withStyleVars({ FramePadding = ui.theme.styles.ButtonPadding }, function()
			savePath, confirmed = ui.inputText("##savePath", self.savePath, { "EnterReturnsTrue" })
		end)

		if savePath ~= self.savePath then
			self:message("onSavePathChanged", savePath)
		end

		if confirmed then
			self:message("saveToFilePath")
		end
	else
		ui.dummy(Vector2(ui.getContentRegion().x - buttonWidths, 0))
	end

	ui.sameLine()

	-- Draw load/recover button
	if self.mode == SaveLoadWindow.Modes.Load then
		local wantRecovery = ui.ctrlHeld()

		if self.selectedFile then
			wantRecovery = wantRecovery or not self:getSaveEntry(self.selectedFile).compatible
		end

		if mainButton(wantRecovery and lui.RECOVER or lui.LOAD, self.selectedFile) then
			self:message(wantRecovery and "recoverSelectedSave" or "loadSelectedSave")
		end
	else
		local active = self.savePath and #self.savePath > 0

		if mainButton(lui.SAVE, active) then
			self:message("saveToFilePath")
		end
	end

	ui.sameLine()
	if mainButton(lui.DELETE, self.selectedFile) then
		self:message("deleteSelectedSave")
	end

	ui.sameLine()
	if ui.button(lui.CANCEL, style.mainButtonSize) then
		self:message("close")
	end
end

function SaveLoadWindow:drawSaveList()
	for _, f in ipairs(self.filteredFiles) do
		local entry = self:getSaveEntry(f.name)

		if entry then
			local selected = drawSaveEntryRow(entry, f.name == self.selectedFile)

			if selected then
				self:message("onFileSelected", f.name)

				if ui.isMouseDoubleClicked(0) then
					self:message("loadSelectedSave")
				end
			end
		end
	end
end

function SaveLoadWindow:outerHandler(drawPopupFn)
	local size = ui.screenSize() - style.popupPadding * 2
	ui.setNextWindowSize(size, "Always")
	ui.setNextWindowPosCenter('Always')
	ui.withStyleColorsAndVars({ PopupBg = ui.theme.colors.modalBackground }, { WindowPadding = style.windowPadding }, drawPopupFn)

	-- Debug reloading
	if ui.ctrlHeld() and ui.isKeyReleased(ui.keys.delete) then
		self:message("close")
		package.reimport()
	end
end

function SaveLoadWindow:render()
	local windowSpacing = Vector2(ui.theme.styles.ItemSpacing.x, style.windowPadding.y)

	ui.withStyleVars({ WindowPadding = ui.theme.styles.WindowPadding, ItemSpacing = windowSpacing }, function()

		self:drawSearchHeader()

		ui.separator()

		local bottomLineHeight = ui.getButtonHeight(mainButtonFont) + windowSpacing.y * 2
		local saveListSize = Vector2(0, ui.getContentRegion().y - bottomLineHeight)

		ui.child("##SaveFileList", saveListSize, function()
			self:drawSaveList()
		end)

		ui.separator()

		ui.withStyleVars({ ItemSpacing = style.itemSpacing }, function()
			ui.withFont(mainButtonFont, function()
				self:drawSaveFooter()
			end)
		end)

	end)

	if ui.isKeyReleased(ui.keys.escape) then
		self:close()
	end
end

--=============================================================================

Event.Register("onSaveGameStats", onSaveGameStats)

ui.saveLoadWindow = SaveLoadWindow

return {}
