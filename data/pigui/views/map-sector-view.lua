-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
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

local Serializer = require 'Serializer'

local player = nil
local colors = ui.theme.colors
local icons = ui.theme.icons


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


local function textIcon(icon, tooltip)
	ui.icon(icon, Vector2(ui.getTextLineHeight()), svColor.FONT, tooltip)
	ui.sameLine()
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
	current = layout.NewWindow("SectorMapCurrentSystem"), -- current system string
	hjPlanner = layout.NewWindow("HyperJumpPlanner"), -- hyper jump planner
	systemInfo = layout.NewWindow("SectorMapSystemInfo"), -- selected system information
	searchBar = layout.NewWindow("SectorMapSearchBar"),
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
	ui.text(string.format("%.2f%s %d%s %s",
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
			if ui.iconButton(icons.maneuver, Vector2(ui.getTextLineHeight()), lui.CENTER_ON_SYSTEM, ui.theme.buttonColors.transparent) then
				sectorView:GetMap():GotoSystemPath(systempath)
			end
		end

		-- number of stars
		local numstarsText = {
			-- don't ask for the astro description of a gravpoint
			starsystem.numberOfStars == 1 and starsystem.rootSystemBody.astroDescription or "",
			lc.BINARY_SYSTEM,
			lc.TRIPLE_SYSTEM,
			lc.QUADRUPLE_SYSTEM
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
	if ui.mainMenuButton(icons.reset_view, lui.RESET_ORIENTATION_AND_ZOOM) then
		sectorView:ResetView()
	end
	ui.mainMenuButton(icons.rotate_view, lui.ROTATE_VIEW)
	sectorView:GetMap():SetRotateMode(ui.isItemActive())
	ui.mainMenuButton(icons.search_lens, lui.ZOOM)
	sectorView:GetMap():SetZoomMode(ui.isItemActive())
	ui.text("")
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
	if ui.mainMenuButton(icons.info, lc.OBJECT_INFO, buttonState[Windows.systemInfo.visible]) then
		Windows.systemInfo.visible = not Windows.systemInfo.visible
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

local searchString, systemPaths = "", nil
local leftBarMode = "SEARCH"

function Windows.searchBar:Show()
	if ui.mainMenuButton(icons.search_lens, lc.SEARCH) then leftBarMode = "SEARCH" end
	ui.sameLine()
	if ui.mainMenuButton(icons.money, lui.ECONOMY_TRADE) then
		self.size.y = self.fullHeight
		if ui.altHeld() then
			leftBarMode = "TRADE_COMPUTER"
		else
			leftBarMode = "ECONOMY"
		end
	end

	ui.spacing()

	if leftBarMode == "SEARCH" then
		ui.text(lc.SEARCH)
		local search_text, changed = ui.inputText("##searchText", searchString, {})
		ui.spacing()
		local parsedSystem = changed and search_text ~= "" and SystemPath.ParseString(search_text)
		if parsedSystem and parsedSystem ~= nil then
			sectorView:GetMap():GotoSectorPath(parsedSystem)
		end

		if search_text ~= searchString then
			systemPaths = search_text ~= "" and sectorView:GetMap():SearchNearbyStarSystemsByName(search_text)
			searchString = search_text
		end

		if not systemPaths or #systemPaths == 0 then
			ui.text(lc.NOT_FOUND)
			self.size.y = self.collapsedHeight
		else
			drawSearchResults(systemPaths)
			self.size.y = self.fullHeight
		end

	elseif leftBarMode == "ECONOMY" then
		local selectedPath = sectorView:GetSelectedSystemPath()
		local currentPath = sectorView:GetCurrentSystemPath()

		systemEconView:drawSystemComparison(selectedPath:GetStarSystem(), currentPath:GetStarSystem())
	elseif leftBarMode == "TRADE_COMPUTER" then
		systemEconView:drawSystemFinder()
	end
end

function Windows.searchBar.Dummy()
	ui.mainMenuButton(icons.search_lens, lc.SEARCH)
	ui.spacing()
	ui.text("***************")
	ui.inputText("##searchText", "", {})
	ui.spacing()
	ui.text("***************")
end

function Windows.current.Show()
	local path = sectorView:GetCurrentSystemPath()
	textIcon(icons.navtarget)
	if ui.selectable(' ' .. ui.Format.SystemPath(path)) then
		sectorView:SwitchToPath(path)
	end
end

function Windows.factions.Show()
	textIcon(icons.shield)
	ui.text("Factions")
	local factions = sectorView:GetMap():GetFactions()
	for _,f in pairs(factions) do
		local changed, value
		ui.withStyleColors({ ["Text"] = Color(f.faction.colour.r, f.faction.colour.g, f.faction.colour.b) }, function()
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
	w.edgeButtons.anchors = { ui.anchor.right, ui.anchor.center }
	w.factions.anchors = { ui.anchor.right, ui.anchor.top }
end

function sectorViewLayout:onUpdateWindowConstraints(w)
	-- resizing, aligning windows - static
	w.current.pos = edgePadding
	w.current.size.x = 0 -- adaptive width

	w.searchBar.pos = w.current.pos + w.current.size
	w.searchBar.collapsedHeight = w.searchBar.size.y
	w.searchBar.fullHeight = ui.screenHeight - w.searchBar.pos.y - edgePadding.y - ui.timeWindowSize.y
	w.searchBar.size.y = w.searchBar.fullHeight

	local rightColWidth = math.max(w.hjPlanner.size.x, w.systemInfo.size.x)
	w.hjPlanner.pos = w.hjPlanner.pos - Vector2(w.edgeButtons.size.x, edgePadding.y)
	w.hjPlanner.size.x = rightColWidth

	w.systemInfo.pos = w.hjPlanner.pos - Vector2(0, w.hjPlanner.size.y)
	w.systemInfo.size.x = rightColWidth

	w.factions.pos = Vector2(w.hjPlanner.pos.x, edgePadding.y)
	w.factions.size = Vector2(rightColWidth, 0.0) -- adaptive height
end

ui.registerModule("game", { id = 'map-sector-view', draw = function()
	player = Game.player
	if Game.CurrentView() == "sector" then
		sectorViewLayout:display()

		if ui.isKeyReleased(ui.keys.tab) then
			sectorViewLayout.enabled = not sectorViewLayout.enabled
			sectorView:GetMap():SetLabelsVisibility(not sectorViewLayout.enabled)
		end

		if ui.escapeKeyReleased() then
			Game.SetView("world")
		end

		if ui.ctrlHeld() and ui.isKeyReleased(ui.keys.delete) then
			systemEconView = package.reimport('pigui.modules.system-econ-view').New()
		end
	end
end})

Event.Register("onGameStart", onGameStart)
Event.Register("onEnterSystem", function(ship)
	hyperJumpPlanner.onEnterSystem(ship)
	if ship:IsPlayer() then hyperspaceDetailsCache = {} end
end)

-- reset cached data
Event.Register("onGameEnd", function()
	searchString = ""
	systemPaths = nil
	leftBarMode = "SEARCH"

	hyperJumpPlanner.onGameEnd()
end)

Event.Register("onShipEquipmentChanged", function(ship, ...)
	if ship:IsPlayer() then hyperspaceDetailsCache = {} end
	hyperJumpPlanner.onShipEquipmentChanged(ship, ...)
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
