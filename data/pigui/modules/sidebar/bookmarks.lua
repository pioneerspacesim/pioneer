-- Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game        = require 'Game'
local Commodities = require 'Commodities'
local Event       = require 'Event'
local Lang        = require 'Lang'
local PlayerState = require 'PlayerState'

local utils = require 'utils'
local ui = require 'pigui'

local Vector2 = _G.Vector2
local Color = _G.Color

local lc = Lang.GetResource("core");
local lui = Lang.GetResource("ui-core");

local Module      = require 'pigui.libs.module'
local ModalWindow = require 'pigui.libs.modal-win'
local themeStyles = require 'pigui.styles'

local colors = ui.theme.colors
local icons = ui.theme.icons
local pionillium = ui.fonts.pionillium
local orbiteer = ui.fonts.orbiteer

local bookmarkListStyle = themeStyles.sidebar:clone({
	colors = {},
	vars = {
		ItemSpacing = ui.theme.styles.ItemInnerSpacing,
		CellPadding = ui.rescaleUI(Vector2(4, 8))
	}
})

local bookmarkEditStyle = ui.Style:clone({
	colors = {},
	vars = {
		WindowRounding = ui.theme.styles.StyleRounding,
	}
})

local sortModes = {
	DISTANCE = lui.SORT_DISTANCE,
	NAME = lui.SORT_NAME,
	COMMODITY = lui.SORT_COMMODITY,
}

local sortKeys = utils.build_array(utils.keys(sortModes))
table.sort(sortKeys)

local childFlags = ui.ChildFlags { 'AlwaysUseWindowPadding', 'Borders', 'AutoResizeY' }

local bookmarkViews = {}

-- ============================================================================

---@class UI.BookmarkView : UI.Module, UI.Sidebar.Module
---@field systemView boolean
local BookmarkView = utils.class("UI.BookmarkView", Module)

BookmarkView.title = lui.BOOKMARKS
BookmarkView.tooltip = lui.BOOKMARKS
BookmarkView.icon = icons.bookmark
BookmarkView.exclusive = true

BookmarkView.sortMode = "DISTANCE"
BookmarkView.dirty = false

local bookmarkEdit = ModalWindow.New("BookmarkEdit")

bookmarkEdit.centered = true
bookmarkEdit.bookmark = nil
bookmarkEdit.bookmarkId = nil
bookmarkEdit.style = bookmarkEditStyle

function BookmarkView:Constructor()
	Module.Constructor(self)

	---@type BookmarkID[]
	self.bookmarks = {}

	table.insert(bookmarkViews, self)
end

-- ============================================================================

local function get_bookmark_name(path)
	return path:IsBodyPath() and path:GetSystemBody().name or path:GetStarSystem().name
end

---@param path SystemPath
local function make_bookmark_note(path)
	if path:IsBodyPath() then
		local body = path:GetSystemBody()
		return body and body.astroDescription or tostring(path)
	elseif path:IsSystemPath() then
		local system = path:GetStarSystem()
		return system and system.shortDescription or tostring(path)
	end

	return lc.SECTOR_X_Y_Z % path
end

---@return SystemPath
function BookmarkView:getCurrentPath()
	return Game.sectorView:GetCurrentSystemPath()
end

---@return SystemPath
function BookmarkView:getSelectedPath()
	if self.systemView then
		return Game.systemView:GetSelectedObject().ref.path
	else
		return Game.sectorView:GetSelectedSystemPath()
	end
end

---@param path SystemPath
function BookmarkView:selectPath(path)
	if self.systemView then
		-- TODO: system view selection is incredibly complicated
	else
		return Game.sectorView:SwitchToPath(path)
	end
end

function BookmarkView:removeBookmark(id)
	PlayerState.RemoveBookmark(id)
	self.dirty = true
end

function BookmarkView:rebuildBookmarks()
	local bookmarks = PlayerState.GetBookmarks()

	local bookmarkList = utils.build_array(utils.keys(bookmarks))

	local current = self:getCurrentPath()

	local sorts = {}

	function sorts.NAME(a, b)
		local name_a = get_bookmark_name(bookmarks[a].path)
		local name_b = get_bookmark_name(bookmarks[b].path)
		return name_a < name_b or (name_a == name_b and a < b)
	end

	function sorts.DISTANCE(a, b)
		local dist_a = bookmarks[a].path:DistanceTo(current)
		local dist_b = bookmarks[b].path:DistanceTo(current)
		return dist_a < dist_b or (dist_a == dist_b and a < b)
	end

	function sorts.COMMODITY(a, b)
		local comm_a = bookmarks[a].commodity
		local comm_b = bookmarks[b].commodity
		if not comm_a and not comm_b then
			return sorts.NAME(a, b)
		end

		if comm_a ~= comm_b then
			return not comm_b or comm_a and comm_a < comm_b
		end

		local name_a = get_bookmark_name(bookmarks[a].path)
		local name_b = get_bookmark_name(bookmarks[b].path)
		if name_a ~= name_b then
			return name_a < name_b
		end

		local route_a = get_bookmark_name(bookmarks[a].route_to)
		local route_b = get_bookmark_name(bookmarks[b].route_to)
		return route_a < route_b
	end

	table.sort(bookmarkList, sorts[self.sortMode])

	self.bookmarks = bookmarkList
end

function BookmarkView:setSortMode(mode)
	self.sortMode = mode
	self.dirty = true
end

function BookmarkView:refresh()
	self:rebuildBookmarks()
	self.dirty = false
end

function BookmarkView:update()
	Module.update(self)

	if self.dirty then
		self:rebuildBookmarks()
		self.dirty = false
	end
end

-- ============================================================================

function bookmarkEdit:render()

	ui.withFont(orbiteer.body, function()
		ui.text(lui.BOOKMARK .. ":")
		ui.sameLine()
		ui.text(get_bookmark_name(self.bookmark.path))

		ui.spacing()
	end)

	ui.withFont(pionillium.body, function()

		ui.text(lui.EDIT_NOTE)

		ui.nextItemWidth(ui.getContentRegion().x)
		local note, changed = ui.inputText("##Note", self.bookmark.note or "", lui.ENTER_CUSTOM_NOTE, { "EnterReturnsTrue" })

		if changed or ui.isItemDeactivated() then
			if note == "" and not self.bookmark.route_to then
				note = make_bookmark_note(self.bookmark.path)
			end

			self.bookmark.note = (note ~= "" and note or nil)
			PlayerState.UpdateBookmark(self.bookmarkId, self.bookmark)
		end

		if changed or ui.button(lui.CLOSE) or ui.isKeyReleased(ui.keys.escape) then
			self:close()
		end

	end)

end

-- ============================================================================

---@param bookmark SystemBookmark
function BookmarkView:drawBookmark(bookmark, id)
	-- Consume the full width of the table since all of our contents are using ContentRegion to size (feedback loop)
	ui.dummy(Vector2(ui.getWindowSize().x - ui.getWindowPadding().x * 2, 0))
	ui.sameLine(ui.getWindowPadding().x)

	self:drawTitleLine(bookmark, id)

	self:drawDetails(bookmark)
end

---@param bookmark SystemBookmark
function BookmarkView:drawTitleLine(bookmark, id)
	ui.text(get_bookmark_name(bookmark.path))

	-- Compute sizing requirements for distance + buttons
	local distance = bookmark.path:DistanceTo(self:getCurrentPath())
	local dist_text = ui.Format.Number(distance, 1) .. lc.UNIT_LY

	local dist_size = ui.calcTextSize(dist_text, pionillium.details)
	local icon_size = Vector2(ui.getTextLineHeight())

	local line_height = ui.getTextLineHeight()
	local util_width = dist_size.x + (icon_size.x + ui.getItemSpacing().x) * 3

	-- Small system name after body name
	if bookmark.path:IsBodyPath() then
		local name = bookmark.path:GetStarSystem().name
		local elided = false

		ui.withFont(pionillium.details, function()
			ui.withStyleColors({ Text = colors.fontDim }, function()
				ui.sameLine()
				ui.alignTextToLineHeight(line_height, 1.0)
				elided = ui.textEllipsis(name, ui.getContentRegion().x - util_width)
			end)
		end)

		if elided then ui.setItemTooltip(name) end
	end

	ui.sameLine(-util_width)

	-- Render distance text + buttons
	ui.horizontalGroup(function()

		ui.withFont(pionillium.details, function()
			ui.alignTextToLineHeight(line_height, 1.0)
			ui.text(dist_text)
		end)

		if ui.iconButton("view_" .. id, icons.view_internal, lui.CENTER_ON_SYSTEM, nil, icon_size) then
			self:selectPath(bookmark.path)
		end

		if ui.iconButton("edit_" .. id, icons.pencil, lui.EDIT_BOOKMARK, nil, icon_size) then
			local size = Vector2(ui.screenWidth / 3, 0)

			bookmarkEdit.size = size
			bookmarkEdit.bookmark = bookmark
			bookmarkEdit.bookmarkId = id

			bookmarkEdit:open()
		end

		if ui.iconButton("delete_" .. id, icons.cross, lui.DELETE_BOOKMARK, nil, icon_size) then
			self:message("removeBookmark", id)
		end

	end)
end

---@param bookmark SystemBookmark
function BookmarkView:drawDetails(bookmark)

	ui.withFont(pionillium.details, function()

		if bookmark.route_to then

			if bookmark.commodity then
				ui.text(Commodities[bookmark.commodity]:GetName())
				ui.sameLine()
			end

			ui.text(ui.get_icon_glyph(icons.route))
			ui.sameLine()
			ui.textEllipsis(bookmark.route_to:GetStarSystem().name)

		end

		if bookmark.note then

			local elided = false
			ui.withStyleColors({ Text = colors.fontDim }, function()
				elided = ui.textEllipsis(bookmark.note)
			end)

			if elided then
				ui.setItemTooltip(bookmark.note)
			end

		end

	end)

end

function BookmarkView:drawBody()
	local bookmarks = PlayerState.GetBookmarks()

	themeStyles.sidebar:push()

	themeStyles.combo:withStyle(function()
		ui.alignTextToFramePadding()
		ui.text(lui.SORT_MODE .. ":")

		ui.sameLine(-ui.getContentRegion().x * 0.5)
		ui.nextItemWidth(ui.getContentRegion().x)

		ui.comboBox("##SortMode", sortModes[self.sortMode], function()

			for _, mode in ipairs(sortKeys) do

				if ui.selectable(sortModes[mode]) then
					self:message("setSortMode", mode)
				end

			end

		end)
	end)

	ui.child("BookmarkList", Vector2(-1, -1), nil, childFlags, function()

		bookmarkListStyle:withStyle(function()

			if ui.beginTable("##bookmarks", 1, "BordersInnerH") then

				for _, id in ipairs(self.bookmarks) do
					local bookmark = bookmarks[id]

					ui.tableNextRow()
					ui.tableNextColumn()

					self:drawBookmark(bookmark, id)

				end

				ui.endTable()

			end

		end)

		if #self.bookmarks == 0 then
			ui.withFont(orbiteer.body, function()
				ui.textAligned(lui.NO_BOOKMARKS_FOUND, 0.5)
			end)
		end

	end)

	themeStyles.sidebar:pop()
end

function BookmarkView:debugReload()
	bookmarkEdit:close()
	package.reimport()
end

Event.Register("onBookmarkChanged", function()
	for _, view in ipairs(bookmarkViews) do
		view.dirty = true
	end
end)

return BookmarkView
