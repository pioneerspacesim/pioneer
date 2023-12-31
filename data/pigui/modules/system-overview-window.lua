-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local Format = require 'Format'
local Lang = require 'Lang'
local utils = require "libs.utils"

local lc = Lang.GetResource("core");
local lui = Lang.GetResource("ui-core");

local ui = require 'pigui'

local getBodyIcon = require 'pigui.modules.flight-ui.body-icons'

local colors = ui.theme.colors
local icons = ui.theme.icons
local styles = ui.theme.styles
local Vector2 = _G.Vector2

local style = ui.rescaleUI {
	iconSize = Vector2(24,24),
	bodyIconSize = Vector2(18,18),
	buttonSize = Vector2(32,32)
}

-- Reusable widget to list the static contents of a system.
-- Intended to be used by flight-ui as a list of nav targets,
-- and the system info view as a list of bodies in-system

---@class SystemOverviewWidget
---@field New fun(): SystemOverviewWidget
local SystemOverviewWidget = utils.class('ui.SystemOverviewWidget')

function SystemOverviewWidget:Constructor()
	self.shouldDisplayPlayerDistance = false
	self.shouldSortByPlayerDistance = false
	self.shouldShowStations = false
	self.shouldShowMoons = false
	self.focusSearchResults = false
	self.visible = false
	self.filterText = ""
	self.size = Vector2(0, 0)
	self.buttonSize = nil
end

local function sortByPlayerDistance(a,b)
	return (a.body and b.body) and a.body:DistanceTo(Game.player) < b.body:DistanceTo(Game.player)
end

local function sortBySystemDistance(a,b)
	return (a.systemBody.periapsis + a.systemBody.apoapsis) < (b.systemBody.periapsis + b.systemBody.apoapsis)
end

local function make_result(systemBody, label, isSelected)
	return {
		systemBody = systemBody,
		body = systemBody.body,
		label = label,
		children = {},
		visible = true,
		children_visible = isSelected,
		selected = isSelected,
		has_space_stations = false,
		has_ground_stations = false,
		has_moons = false,
	}
end

-- Returns a table of entries.
-- Each entry will have { children_visible = true } if they are a parent of, or are a selected object
-- Entries that are excluded by the current filter will have { visible = false }
---@return table @ information about a given SystemBody
local function calculateEntry(systemBody, parent, selected, filter)
	local result = nil
	local isSelected = selected[systemBody] or (systemBody.body and selected[systemBody.body]) or false

	result = make_result(systemBody, systemBody.name, isSelected)
	result.visible = isSelected or filter(systemBody)

	-- Set show-flags on the direct parent of this body
	if systemBody.isSpaceStation then
		parent.has_space_stations = true
	elseif systemBody.isGroundStation then
		parent.has_ground_stations = true
	elseif systemBody.isMoon then
		parent.has_moons = true
	end

	for _, child in pairs(systemBody.children or {}) do
		table.insert(result.children, calculateEntry(child, result, selected, filter))
	end

	-- propagate children_visible and visible upwards
	if parent then
		if result.visible then parent.visible = true end
		if result.children_visible then parent.children_visible = true end
	end

	return result
end

function SystemOverviewWidget:onBodySelected(sBody, body)
	-- override me
end

function SystemOverviewWidget:onBodyDoubleClicked(sBody, body)
	-- override me
end

function SystemOverviewWidget:onBodyContextMenu(sBody, body)
	-- override me
end

-- Render a row for an entry in the system overview
function SystemOverviewWidget:renderEntry(entry, indent)
	local sbody = entry.systemBody
	local label = entry.label or "UNKNOWN"

	ui.dummy(Vector2(style.iconSize.x * indent / 2.0, style.iconSize.y))
	ui.sameLine()
	ui.icon(getBodyIcon(sbody), style.iconSize, colors.font)
	ui.sameLine()

	local pos = ui.getCursorPos()
	if ui.selectable("##" .. label, entry.selected, {"SpanAllColumns"}, Vector2(0, style.iconSize.y)) then
		self:onBodySelected(sbody, entry.body)
	end
	if ui.isItemHovered() and ui.isMouseDoubleClicked(0) then
		self:onBodyDoubleClicked(sbody, entry.body)
	end
	if ui.isItemHovered() and ui.isMouseClicked(1) then
		self:onBodyContextMenu(sbody, entry.body)
	end

	ui.setCursorPos(pos)
	ui.alignTextToLineHeight(style.iconSize.y)
	ui.text(label)
	ui.sameLine()

	if entry.has_moons then
		ui.icon(icons.moon, style.bodyIconSize, colors.font)
		ui.sameLine(0,0.01)
	end
	if entry.has_ground_stations then
		ui.icon(icons.starport, style.bodyIconSize, colors.font)
		ui.sameLine(0,0.01)
	end
	if entry.has_space_stations then
		ui.icon(icons.spacestation, style.bodyIconSize, colors.font)
		ui.sameLine(0,0.01)
	end

	ui.nextColumn()
	ui.dummy(Vector2(0, style.iconSize.y))
	ui.sameLine()
	ui.alignTextToLineHeight(style.iconSize.y)

	local distance
	if entry.body and self.shouldDisplayPlayerDistance then
		distance = entry.body:DistanceTo(Game.player)
	else
		distance = (sbody.apoapsis + sbody.periapsis) / 2.0
	end
	ui.text(Format.Distance(distance))
	ui.nextColumn()
end

function SystemOverviewWidget:showEntry(entry, indent, sortFunction)
	if entry.systemBody.type ~= 'GRAVPOINT' then self:renderEntry(entry, indent) end

	local isVisible = entry.children_visible and not self.focusSearchResults
	table.sort(entry.children, sortFunction)
	for _, v in pairs(entry.children) do
		if v.visible or isVisible then
			self:showEntry(v, indent + 1, sortFunction)
		end
	end
end

function SystemOverviewWidget:drawControlButtons()
	local buttonSize = self.buttonSize or styles.MainButtonSize

	if self.shouldDisplayPlayerDistance then
		if ui.mainMenuButton(icons.distance, lui.TOGGLE_OVERVIEW_SORT_BY_PLAYER_DISTANCE, self.shouldSortByPlayerDistance, buttonSize) then
			self.shouldSortByPlayerDistance = not self.shouldSortByPlayerDistance
		end
		ui.sameLine()
	end

	if ui.mainMenuButton(icons.moon, lui.TOGGLE_OVERVIEW_SHOW_MOONS, self.shouldShowMoons, buttonSize) then
		self.shouldShowMoons = not self.shouldShowMoons
	end
	ui.sameLine()
	if ui.mainMenuButton(icons.filter_stations, lui.TOGGLE_OVERVIEW_SHOW_STATIONS, self.shouldShowStations, buttonSize) then
		self.shouldShowStations = not self.shouldShowStations
	end
end

function SystemOverviewWidget:displaySearch()
	local filterText = ui.inputText("##FilterText", self.filterText, {})
	self.filterText = filterText
	self.focusSearchResults = filterText and filterText ~= ""

	ui.sameLine()
	ui.icon(icons.filter_bodies, style.buttonSize, colors.frame, lui.OVERVIEW_NAME_FILTER)
end

function SystemOverviewWidget:display(system, root, selected)
	root = root or system.rootSystemBody

	local sortFunction = self.shouldSortByPlayerDistance and sortByPlayerDistance or sortBySystemDistance

	local filterText = self.filterText
	local filterFunction = function(systemBody)
		-- only plain text matches, no regexes
		if filterText ~= "" and filterText ~= nil and not string.find(systemBody.name:lower(), filterText:lower(), 1, true) then
			return false
		end
		if not (self.shouldShowMoons or self.focusSearchResults) and systemBody.isMoon then
			return false
		elseif not (self.shouldShowStations or self.focusSearchResults) and systemBody.isStation then
			return false
		end
		return true
	end

	ui.child("spaceTargets", self.size, function()
		local tree = calculateEntry(root, nil, selected, filterFunction)

		if tree then
			ui.columns(2, "spaceTargetColumnsOn", false) -- no border
			ui.setColumnOffset(1, ui.getWindowSize().x * 0.66)
			self:showEntry(tree, 0, sortFunction)
			ui.columns(1, "spaceTargetColumnsOff", false) -- no border
			ui.radialMenu("systemoverviewspacetargets")
		else
			ui.text(lui.NO_FILTER_MATCHES)
		end
	end)
end

function SystemOverviewWidget:displaySidebarTitle(system)
	local spacing = styles.ItemInnerSpacing
	local numButtons = self.shouldDisplayPlayerDistance and 3 or 2
	local button_width = (ui.getLineHeight() + spacing.x) * numButtons - spacing.x

	local pos = ui.getCursorPos() + Vector2(ui.getContentRegion().x - button_width, 0)
	self.buttonSize = Vector2(ui.getLineHeight() - styles.MainButtonPadding * 2)

	if system then
		ui.text(system.name)

		ui.setCursorPos(pos)
		ui.withStyleVars({ ItemSpacing = spacing }, function()
			ui.withFont(ui.fonts.pionillium.medium, function()
				self:drawControlButtons()
			end)
		end)
	else
		ui.text(lc.HYPERSPACE)
	end
end

return SystemOverviewWidget
