-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local Event = require 'Event'
local Format = require 'Format'
local SystemPath = require 'SystemPath'
local hyperJumpPlanner = require 'pigui.modules.hyperjump-planner'
local systemEconView = require 'pigui.modules.system-econ-view'.New()

local Lang = require 'Lang'
local lc = Lang.GetResource("core");
local lui = Lang.GetResource("ui-core");
local Vector2 = _G.Vector2
local Color = _G.Color

local ui = require 'pigui'
local layout = require 'pigui.libs.window-layout'
local Sidebar = require 'pigui.libs.sidebar'

local Serializer = require 'Serializer'

local player = nil
local colors = ui.theme.colors
local icons = ui.theme.icons

local orbiteer = ui.fonts.orbiteer
local pionillium = ui.fonts.pionillium

local font = ui.fonts.pionillium.medlarge
local smallfont = ui.fonts.pionillium.medium
local edgePadding = Vector2(font.size)

-- all colors, used in this module
local svColor = {
	LABEL_HIGHLIGHT = colors.sectorMapLabelHighlight,
	LABEL_SHADE = colors.sectorMapLabelShade,
	FONT = colors.font,
	UNKNOWN = colors.unknown,
}

local buttonState = {
	[true]        = nil, -- to use the default color
	[false]       = ui.theme.buttonColors.transparent
}

local settings =
{
	draw_vertical_lines=false,
	draw_out_range_labels=false,
	draw_uninhabited_labels=true,
	automatic_system_selection=true
}

local loaded_data = nil

local leftSidebar = Sidebar.New("SectorMapLeft")

local function textIcon(icon, tooltip)
	ui.icon(icon, Vector2(ui.getTextLineHeight()), svColor.FONT, tooltip)
	ui.sameLine()
end

local function bannerText(pos, text, color)
	local s = ui.calcTextSize(text)

	local padding = ui.theme.styles.ButtonPadding
	s.y = s.y + padding.y * 2
	local max = pos + Vector2(s.x + s.y * 2, s.y)

	local x1 = Vector2(pos.x + s.y, pos.y + s.y)
	local x2 = Vector2(pos.x + s.y, pos.y)
	local x3 = Vector2(pos.x + s.y + s.x, pos.y + s.y)
	local x4 = Vector2(pos.x + s.y + s.x, pos.y)

	ui.addRectFaded(pos, x1, color, 0.0, 0x5)
	ui.addRectFilled(x2, x3, color, 0.0, 0)
	ui.addRectFaded(x4, max, color, 0.0, 0xA)

	ui.setCursorScreenPos(Vector2(x2.x, x2.y + padding.y))
	ui.text(text)
	ui.sameLine(0, 0)
	ui.dummy(Vector2(s.y, s.y))
end

local sectorView
if Game then
	sectorView = Game.sectorView
	hyperJumpPlanner.setSectorView(sectorView)
end -- for hot-reload

local hyperspaceDetailsCache = {}
local prevSystemPath = nil

local onGameStart = function ()
	-- connect to class SectorView
	sectorView = Game.sectorView
	-- connect hyper jump planner to class SectorView
	hyperJumpPlanner.setSectorView(sectorView)
	-- reset hyperspace details cache on new game
	hyperspaceDetailsCache = {}

	-- apply any data loaded earlier
	if loaded_data then
		if loaded_data.jump_targets then
			local targets = loaded_data.jump_targets
			for _, target in pairs( loaded_data.jump_targets) do
				sectorView:AddToRoute(target)
			end
		end
		if loaded_data.settings then
			settings = loaded_data.settings
		end
		loaded_data = nil
	end

	-- update visibility states
	sectorView:SetAutomaticSystemSelection(settings.automatic_system_selection)
	sectorView:SetDrawOutRangeLabels(settings.draw_out_range_labels)
	sectorView:GetMap():SetDrawUninhabitedLabels(settings.draw_uninhabited_labels)
	sectorView:GetMap():SetDrawVerticalLines(settings.draw_vertical_lines)
	sectorView:GetMap():SetLabelParams("orbiteer", font.size, 2.0, svColor.LABEL_HIGHLIGHT, svColor.LABEL_SHADE)

	-- allow hyperjump planner to register its events:
	hyperJumpPlanner.onGameStart()

	-- Reset hyperspace cache when ship equipment changes
	Game.player:GetComponent('EquipSet'):AddListener(function (op, equip, slot)
		hyperspaceDetailsCache = {}
	end)
end

local function getHyperspaceDetails(path)
	local p = path.sectorX .. "/" .. path.sectorY .. "/" .. path.sectorZ .. "/" .. path.systemIndex
	local it = hyperspaceDetailsCache[p]
	if not it then
		local jumpStatus, distance, fuelRequired, duration = player:GetHyperspaceDetails(path)
		it = { jumpStatus = jumpStatus, distance = distance, fuelRequired = fuelRequired, duration = duration, path = path }
		hyperspaceDetailsCache[p] = it
	end
	return it
end

-- all windows in this view
local Windows = {
	hjPlanner = layout.NewWindow("HyperJumpPlanner"), -- hyper jump planner
	systemInfo = layout.NewWindow("SectorMapSystemInfo"), -- selected system information
	edgeButtons = layout.NewWindow("SectorMapEdgeButtons"),
	factions = layout.NewWindow("SectorMapFactions")
}

local statusIcons = {
	OK = { icon = icons.route_destination },
	DRIVE_ACTIVE = { icon = icons.ship },
	CURRENT_SYSTEM = { icon = icons.navtarget },
	INSUFFICIENT_FUEL = { icon = icons.fuel },
	OUT_OF_RANGE = { icon = icons.alert_generic },
	NO_DRIVE = { icon = icons.hyperspace_off }
}

local sectorViewLayout = layout.New(Windows)
sectorViewLayout.mainFont = font

local function draw_jump_status(item)
	textIcon(statusIcons[item.jumpStatus].icon, lui[item.jumpStatus])
	ui.text(string.format("%.2f%s %.1f%s %s",
		item.distance, lc.UNIT_LY, item.fuelRequired, lc.UNIT_TONNES, ui.Format.Duration(item.duration, 2)))
end

local function calc_star_dist(star)
	local dist = 0.0
	while star do
		dist = dist + ((star.apoapsis or 0) + (star.periapsis or 0)) / 2
		star = star.parent
	end
	return dist
end

function Windows.systemInfo:Show()
	local label = lc.SELECTED_SYSTEM
	local current_systempath = sectorView:GetCurrentSystemPath()
	local systempath = sectorView:GetSelectedSystemPath()
	if not systempath then return end
	local starsystem = systempath:GetStarSystem()
	local clicked = false
	ui.withID(label, function()
		-- selected system label
		textIcon(icons.info)
		ui.text(ui.Format.SystemPath(systempath))
		if not sectorView:GetMap():IsCenteredOn(systempath) then
			-- add button to center on the object
			ui.sameLine()
			if ui.inlineIconButton("center", icons.maneuver, lui.CENTER_ON_SYSTEM, ui.theme.buttonColors.transparent) then
				sectorView:GetMap():GotoSystemPath(systempath)
			end
		end

		-- number of stars
		local numstarsText = {
			-- don't ask for the astro description of a gravpoint
			starsystem.numberOfStars == 1 and starsystem.rootSystemBody.astroDescription or "",
			lc.BINARY_SYSTEM,
			lc.TRIPLE_SYSTEM,
			lc.QUADRUPLE_SYSTEM,
			[0] = lui.AN_ERROR_HAS_OCCURRED
		}

		-- description
		ui.withFont(smallfont, function()
			-- jump data
			if not current_systempath:IsSameSystem(systempath) then
				draw_jump_status(getHyperspaceDetails(systempath))
			end
			ui.spacing()

			-- selected system alternative labels
			if next(starsystem.other_names) ~= nil then
				ui.pushTextWrapPos(ui.getContentRegion().x)
				ui.textWrapped(table.concat(starsystem.other_names, ", "))
				ui.popTextWrapPos()
			end

			-- system description
			ui.textWrapped(starsystem.shortDescription)

			ui.spacing()
			ui.withTooltip(lc.GOVERNMENT_TYPE, function()
				textIcon(icons.language)
				ui.textWrapped(starsystem.govDescription)
			end)

			ui.withTooltip(lc.ECONOMY_TYPE, function()
				textIcon(icons.money)
				ui.textWrapped(starsystem.econDescription)
			end)

			local pop = starsystem.population -- population in billion
			local popText
			if pop == 0.0 then
				popText = lc.NO_REGISTERED_INHABITANTS
			elseif pop < 1 / 1000.0 then
				popText = lc.A_FEW_THOUSAND
			else
				popText = ui.Format.NumberAbbv(pop * 1e9)
			end

			ui.withTooltip(lc.POPULATION, function()
				textIcon(icons.personal)
				ui.text(popText)
			end)

			ui.separator()
			ui.spacing()
			ui.text(numstarsText[starsystem.numberOfStars])
			ui.spacing()
		end)

		-- star list
		local stars = starsystem:GetJumpable()
		for _,star in pairs(stars) do
			if ui.selectable("## " .. star.name, star.path == systempath, {}) then
				clicked = star.path
			end
			ui.sameLine(0, 0)
			textIcon(icons.sun)
			ui.text(star.name)
			-- distance from system center
			local dist = calc_star_dist(star)
			if dist > 1 then
				local dist_text = Format.Distance(dist)
				ui.sameLine(ui.getColumnWidth() - ui.calcTextSize(dist_text).x)
				ui.text(dist_text)
			end
		end
		if clicked then
			sectorView:SwitchToPath(clicked)
		end

		-- check if the selected star has changed
		if systempath ~= prevSystemPath then
			-- if so, check the route, and update there if necessary
			hyperJumpPlanner.updateInRoute(systempath)
			prevSystemPath = systempath
		end
	end)
end

function Windows.systemInfo.Dummy()
	ui.text("Selected system")
	ui.text("Distance")
	ui.separator()
	ui.dummy(Vector2(0, ui.getFrameHeightWithSpacing() * 4))
	ui.text("Distance")
	ui.selectable("Star 1", false, {})
	ui.selectable("Star 2", false, {})
	ui.selectable("Star 3", false, {})
	ui.selectable("Star 4", false, {})
end

local function showSettings()
	local changed
	changed, settings.draw_vertical_lines = ui.checkbox(lc.DRAW_VERTICAL_LINES, settings.draw_vertical_lines)
	if changed then
		sectorView:GetMap():SetDrawVerticalLines(settings.draw_vertical_lines)
	end
	changed, settings.draw_out_range_labels = ui.checkbox(lc.DRAW_OUT_RANGE_LABELS, settings.draw_out_range_labels)
	if changed then
		sectorView:SetDrawOutRangeLabels(settings.draw_out_range_labels)
	end
	changed, settings.draw_uninhabited_labels = ui.checkbox(lc.DRAW_UNINHABITED_LABELS, settings.draw_uninhabited_labels)
	if changed then
		sectorView:GetMap():SetDrawUninhabitedLabels(settings.draw_uninhabited_labels)
	end
	changed, settings.automatic_system_selection = ui.checkbox(lc.AUTOMATIC_SYSTEM_SELECTION, settings.automatic_system_selection)
	if changed then
		sectorView:SetAutomaticSystemSelection(settings.automatic_system_selection)
	end
	-- end
end

function Windows.edgeButtons.Show()
	-- view control buttons
	if ui.mainMenuButton(icons.navtarget, lui.CENTER_ON_CURRENT_SYSTEM) then
		sectorView:SwitchToPath(sectorView:GetCurrentSystemPath())
	end

	if ui.mainMenuButton(icons.reset_view, lui.RESET_ORIENTATION_AND_ZOOM) then
		sectorView:ResetView()
	end
	ui.mainMenuButton(icons.rotate_view, lui.ROTATE_VIEW)
	sectorView:GetMap():SetRotateMode(ui.isItemActive())
	ui.mainMenuButton(icons.search_lens, lui.ZOOM)
	sectorView:GetMap():SetZoomMode(ui.isItemActive())
	ui.text("")

	if ui.mainMenuButton(icons.info, lc.OBJECT_INFO, buttonState[Windows.systemInfo.visible]) then
		Windows.systemInfo.visible = not Windows.systemInfo.visible
	end
	-- settings buttons
	if ui.mainMenuButton(icons.settings, lui.SETTINGS) then
		ui.openPopup("sectorViewLabelSettings")
	end
	ui.popup("sectorViewLabelSettings", function()
		showSettings()
	end)

	if ui.mainMenuButton(icons.shield_other, lui.FACTIONS, buttonState[Windows.factions.visible]) then
		Windows.factions.visible = not Windows.factions.visible
	end
	if ui.mainMenuButton(icons.route, lui.HYPERJUMP_ROUTE, buttonState[Windows.hjPlanner.visible]) then
		Windows.hjPlanner.visible = not Windows.hjPlanner.visible
	end
end

local function drawSearchResults(systempaths)
	local data = {}
	for _,path in pairs(systempaths) do table.insert(data, getHyperspaceDetails(path)) end

	ui.child("search_results", function ()
		table.sort(data, function(a,b)
			return a.path ~= b.path and (not a.path:IsSameSystem(b.path)) and a.distance < b.distance
		end)
		for i, item in pairs(data) do
			local system = item.path:GetStarSystem()
			if ui.selectable(system.name .. "##" .. i, false, {}) then
				sectorView:SwitchToPath(item.path)
			end
			ui.sameLine()
			ui.withFont(smallfont, function() draw_jump_status(item) end)
		end
	end)
end

function Windows.factions.Show()
	textIcon(icons.shield)
	ui.text("Factions")
	local factions = sectorView:GetMap():GetFactions()
	for _,f in pairs(factions) do
		local changed, value
		ui.withStyleColors({ Text = Color(f.faction.colour.r, f.faction.colour.g, f.faction.colour.b) }, function()
			changed, value = ui.checkbox(f.faction.name, f.visible)
		end)
		if changed then
			sectorView:GetMap():SetFactionVisible(f.faction, value)
		end
	end
end

Windows.hjPlanner.Show = hyperJumpPlanner.display
Windows.hjPlanner.Dummy = hyperJumpPlanner.Dummy

function sectorViewLayout:onUpdateWindowPivots(w)
	w.hjPlanner.anchors = { ui.anchor.right, ui.anchor.bottom }
	w.systemInfo.anchors = { ui.anchor.right, ui.anchor.bottom }
	w.edgeButtons.anchors = { ui.anchor.right, ui.anchor.top }
	w.factions.anchors = { ui.anchor.right, ui.anchor.top }
end

function sectorViewLayout:onUpdateWindowConstraints(w)
	local rightColWidth = math.max(w.hjPlanner.size.x, w.systemInfo.size.x)
	w.hjPlanner.pos = w.hjPlanner.pos - Vector2(w.edgeButtons.size.x, edgePadding.y)
	w.hjPlanner.size.x = rightColWidth

	w.systemInfo.pos = w.hjPlanner.pos - Vector2(0, w.hjPlanner.size.y)
	w.systemInfo.size.x = rightColWidth

	w.factions.pos = Vector2(w.hjPlanner.pos.x, edgePadding.y)
	w.factions.size = Vector2(rightColWidth, 0.0) -- adaptive height
end

-- System Information bar
---@type UI.Sidebar.Module
local infoView = {
	icon = icons.info,
	title = lui.MORE_INFO,
	tooltip = lui.MORE_INFO,
	exclusive = true,
	drawBody = function(self)
		local description_long = sectorView:GetSelectedSystemPath():GetStarSystem().longDescription

		ui.withFont(pionillium.details, function()
			ui.textWrapped(description_long)
		end)
	end
}

-- Search bar module
---@type UI.Sidebar.Module
local searchBar = {
	searchText = "",
	systemPaths = nil, ---@type table?
	icon = icons.search_lens,
	tooltip = lc.SEARCH,
	exclusive = true,
	drawTitle = function(self)
		ui.withFont(pionillium.body, function()
			ui.addCursorPos(Vector2(0, 0.5 * (ui.getLineHeight() - ui.getFrameHeight())))
			self:updateSearch(ui.inputText("##searchText", self.searchText, lc.SEARCH, {}))
		end)
	end,
	drawBody = function(self)
		if not self.systemPaths or #self.systemPaths == 0 then
			ui.text(lc.NOT_FOUND)
		else
			drawSearchResults(self.systemPaths)
		end
	end,

	---@param search string
	---@param go boolean
	updateSearch = function(self, search, go)
		if go and search ~= "" then
			local path = SystemPath.ParseString(search)

			if path then
				sectorView:GetMap():GotoSectorPath(path)
			end
		end

		if search ~= self.searchText then
			self.searchText = search
			self.systemPaths = nil

			if search ~= "" then
				self.systemPaths = sectorView:GetMap():SearchNearbyStarSystemsByName(search)
			end
		end
	end
}

-- Economy view module
---@type UI.Sidebar.Module
local econView = {
	icon = icons.money,
	tooltip = lui.ECONOMY_TRADE,
	exclusive = true,
	mode = "normal",
	drawTitle = function(self)
		ui.text(lui.ECONOMY_TRADE)
		if ui.altHeld() or self.mode == "debug" then
			local buttonSize = Vector2(ui.getButtonHeight())
			ui.sameLine(ui.getContentRegion().x - buttonSize.x, 0)

			if ui.iconButton("DEBUG", icons.alert1, "Debug Mode", nil, buttonSize) then
				self.mode = self.mode == "debug" and "normal" or "debug"
			end
		end
	end,
	drawBody = function(self)
		local selectedPath = sectorView:GetSelectedSystemPath()
		local currentPath = sectorView:GetCurrentSystemPath()

		if self.mode == "normal" then
			systemEconView:drawSystemComparison(selectedPath:GetStarSystem(), currentPath:GetStarSystem())
		elseif self.mode == "debug" then
			systemEconView:drawSystemFinder()
		end
	end
}

table.insert(leftSidebar.modules, infoView)
table.insert(leftSidebar.modules, searchBar)
table.insert(leftSidebar.modules, econView)

local shouldRefresh = true

-- Renders the current system banner
local function drawCurrentSystemName()
	local window_offset_y = ui.theme.styles.MainButtonSize.y + ui.getWindowPadding().y * 2 + ui.theme.styles.ItemSpacing.y
	ui.setNextWindowPos(Vector2(ui.screenWidth / 2, window_offset_y), "Always", Vector2(0.5, 0))

	ui.window("##CurrentSystem", { "NoDecoration", "NoMove", "AlwaysAutoResize" }, function()
		local path = sectorView:GetCurrentSystemPath()
		ui.withFont(orbiteer.body, function()
			bannerText(ui.getCursorScreenPos(), ui.get_icon_glyph(icons.navtarget) .. " " .. ui.Format.SystemPath(path), colors.lightBlackBackground)
		end)
	end)
end

ui.registerModule("game", { id = 'map-sector-view', draw = function()
	player = Game.player
	if Game.CurrentView() == "SectorView" then

		if shouldRefresh then
			shouldRefresh = false
			leftSidebar:Refresh()
		end

		drawCurrentSystemName()

		leftSidebar:Draw()

		sectorViewLayout:display()

		if ui.isKeyReleased(ui.keys.tab) then
			sectorViewLayout.enabled = not sectorViewLayout.enabled
			sectorView:GetMap():SetLabelsVisibility(not sectorViewLayout.enabled)
		end

		if ui.escapeKeyReleased() then
			Game.SetView("WorldView")
		end

		if ui.ctrlHeld() and ui.isKeyReleased(ui.keys.delete) then
			systemEconView = package.reimport('pigui.modules.system-econ-view').New()
			package.reimport('pigui.modules.hyperjump-planner')
			package.reimport()
		end
	else
		shouldRefresh = true
	end
end})

Event.Register("onGameStart", onGameStart)
Event.Register("onEnterSystem", function(ship)
	hyperJumpPlanner.onEnterSystem(ship)
	if ship:IsPlayer() then hyperspaceDetailsCache = {} end
end)

-- reset cached data
Event.Register("onGameEnd", function()
	leftSidebar:Reset()
	searchBar.searchText = ""
	searchBar.systemPaths = nil

	hyperJumpPlanner.onGameEnd()
end)

Event.Register("onShipTypeChanged", function(ship, ...)
	if ship:IsPlayer() then hyperspaceDetailsCache = {} end
end)


local serialize = function ()

	local data =
	{
		version = 1,
		jump_targets = {},
		settings = settings
	}

	for jumpIndex, jump_sys in pairs(sectorView:GetRoute()) do
		table.insert( data.jump_targets, jump_sys )
	end

	return data
end

local unserialize = function (data)
	loaded_data = data
end

Serializer:Register("HyperJumpPlanner", serialize, unserialize)


return {}
