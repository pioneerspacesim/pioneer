-- Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local Event = require 'Event'
local Format = require 'Format'
local SystemPath = require 'SystemPath'
local hyperJumpPlanner = require 'pigui.modules.hyperjump-planner'
local systemEconView = require 'pigui.modules.system-econ-view'.New()
local PlayerState    = require 'PlayerState'

local BookmarkView = require 'pigui.modules.sidebar.bookmarks'

local Lang = require 'Lang'
local lc = Lang.GetResource("core");
local lui = Lang.GetResource("ui-core");

local Vector2 = _G.Vector2
local Color = _G.Color

local ui = require 'pigui'
local Sidebar = require 'pigui.libs.sidebar'

local Serializer = require 'Serializer'

local player = nil ---@type Player

local colors = ui.theme.colors
local icons = ui.theme.icons

local orbiteer = ui.fonts.orbiteer
local pionillium = ui.fonts.pionillium

local font = ui.fonts.pionillium.medlarge
local smallfont = ui.fonts.pionillium.medium

local sidebarStyle = require 'pigui.styles'.sidebar

local frameStyle = ui.Style:clone {
	colors = {
		Header = colors.FrameBg,
		HeaderActive = colors.FrameBgActive,
		HeaderHovered = colors.FrameBgHovered,
	},
	vars = {}
}

local childFlags = ui.ChildFlags { 'AlwaysUseWindowPadding', 'Borders', 'AutoResizeY' }

-- all colors, used in this module
local svColor = {
	LABEL_HIGHLIGHT = colors.sectorMapLabelHighlight,
	LABEL_SHADE = colors.sectorMapLabelShade,
	FONT = colors.font,
	UNKNOWN = colors.unknown,
}

local settings =
{
	draw_vertical_lines=false,
	draw_out_range_labels=false,
	draw_uninhabited_labels=true,
	automatic_system_selection=true
}

local loaded_data = nil
local ui_visible = true

local leftSidebar = Sidebar.New("SectorMapLeft")
local rightSidebar = Sidebar.New("SectorMapRight", "right")

local function textIcon(icon, tooltip)
	ui.text(ui.get_icon_glyph(icon))
	if tooltip then ui.setItemTooltip(tooltip) end
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

	ui.addRectFaded(pos, x1, color, 0.0, ui.RoundCornersLeft)
	ui.addRectFilled(x2, x3, color, 0.0, ui.RoundCornersNone)
	ui.addRectFaded(x4, max, color, 0.0, ui.RoundCornersRight)

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

local statusIcons = {
	OK = { icon = icons.route_destination },
	DRIVE_ACTIVE = { icon = icons.ship },
	CURRENT_SYSTEM = { icon = icons.navtarget },
	INSUFFICIENT_FUEL = { icon = icons.fuel },
	OUT_OF_RANGE = { icon = icons.alert_generic },
	NO_DRIVE = { icon = icons.hyperspace_off }
}

local function draw_jump_status(item)
	textIcon(statusIcons[item.jumpStatus].icon, lui[item.jumpStatus])
	ui.text(string.format("%.2f%s  %s %.1f%s  %s %s",
		item.distance, lc.UNIT_LY,
		ui.get_icon_glyph(icons.fuel), item.fuelRequired, lc.UNIT_TONNES,
		ui.get_icon_glyph(icons.clock), ui.Format.Duration(item.duration, 2)))
end

local function calc_star_dist(star)
	local dist = 0.0
	while star do
		dist = dist + ((star.apoapsis or 0) + (star.periapsis or 0)) / 2
		star = star.parent
	end
	return dist
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

---@param jumpables SystemBody[]
---@param path SystemPath
local function drawJumpableList(jumpables, path)
	if ui.beginTable("#Jumpables", 3) then
		ui.tableSetupColumn("Name", "WidthStretch")
		ui.tableSetupColumn("Distance", "WidthFixed")
		ui.tableSetupColumn("Select", "WidthFixed")

		---@type SystemPath[]
		local routes = sectorView:GetRoute()
		local index = nil

		for i, route in ipairs(routes) do
			if route:IsSameSystem(path) then
				index = i
				break
			end
		end

		for i, star in ipairs(jumpables) do
			ui.tableNextRow()
			ui.tableNextColumn()

			textIcon(icons.sun)
			ui.text(star.name)

			local is_active_route = index and routes[index] == star.path

			if is_active_route then
				ui.sameLine(0, ui.theme.styles.ItemInnerSpacing.x)

				ui.withTooltip(lui.BODY_IN_ROUTE, function()
					textIcon(icons.route_destination)
				end)
			end

			ui.tableNextColumn()
			local dist = calc_star_dist(star)
			if dist > 0 then
				ui.text(Format.Distance(dist))
			end

			ui.tableNextColumn()

			local iconSize = Vector2(ui.getTextLineHeight())

			ui.withStyleVars({ ItemSpacing = ui.theme.styles.ItemInnerSpacing }, function()

				if ui.iconButton("Bookmark" .. i, icons.bookmark, lui.ADD_BOOKMARK, nil, iconSize) then
					PlayerState.AddBookmark(star.path, { note = star.astroDescription })
				end

				ui.sameLine()

				local routeIcon = is_active_route and icons.map_checkmark or index and icons.map_selectsystem or icons.plus
				local tooltip = is_active_route and lui.REMOVE_JUMP or index and lui.SET_JUMP_TARGET or lui.ADD_JUMP

				if ui.iconButton("Select" .. i, routeIcon, tooltip, nil, iconSize, ui.theme.styles.InlineIconPadding) then
					if is_active_route then
						hyperJumpPlanner.removeJump(index)
					elseif index then
						hyperJumpPlanner.updateInRoute(star.path)
					else
						hyperJumpPlanner.addJump(star.path)
					end
				end

			end)

		end

		ui.endTable()
	end
end

-- System Information bar
---@type UI.Sidebar.Module
local infoView = {
	active = true,
	icon = icons.info,
	tooltip = lui.SYSTEM_INFORMATION,

	drawTitle = function(self)

		local iconSize = Vector2(ui.getButtonHeight())
		local path = sectorView:GetSelectedSystemPath() ---@type SystemPath

		ui.text(path:GetStarSystem().name)
		ui.sameLine(ui.getContentRegion().x - iconSize.x * 2, 0)

		local padding = ui.theme.styles.ItemInnerSpacing

		ui.withStyleVars({ ItemSpacing = Vector2(0, 0) }, function()
			ui.horizontalGroup(function()

				if ui.iconButton("bookmark", icons.bookmark, lui.ADD_BOOKMARK, nil, iconSize, padding) then
					PlayerState.AddBookmark(path:SystemOnly(), { note = path:GetStarSystem().shortDescription })
				end

				if ui.iconButton("focus", icons.maneuver, lui.CENTER_ON_SYSTEM, nil, iconSize, padding) then
					sectorView:GetMap():GotoSystemPath(path)
				end

			end)
		end)

	end,

	drawBody = function(self)

		local path = sectorView:GetSelectedSystemPath()
		if not path then return end

		sidebarStyle:push()

		---@type StarSystem
		local system = path:GetStarSystem()
		local current = sectorView:GetCurrentSystemPath()

		-- number of stars
		local numstarsText = {
			-- don't ask for the astro description of a gravpoint
			system.numberOfStars == 1 and system.rootSystemBody.astroDescription or "",
			lc.BINARY_SYSTEM,
			lc.TRIPLE_SYSTEM,
			lc.QUADRUPLE_SYSTEM,
			[0] = lui.AN_ERROR_HAS_OCCURRED
		}

		ui.textWrapped(system.shortDescription)

		if #system.other_names > 0 then
			ui.withFont(pionillium.details, function()
				ui.textWrapped(lui.KNOWN_AS .. ": " .. table.concat(system.other_names, ", ") .. ".")
			end)
		end

		local description = system.longDescription

		ui.withFont(pionillium.details, function()
			frameStyle:withStyle(function()

				if #description > 0 and ui.collapsingHeader(lui.MORE_INFO) then

					ui.setNextWindowSizeConstraints(Vector2(0), Vector2(-1, ui.getTextLineHeight() * 6 + ui.getWindowPadding().y * 2))
					ui.child("##longDescription", Vector2(0), nil, childFlags, function()
						ui.textWrapped(description)
					end)

				end

			end)
		end)

		ui.separatorText(lui.SYSTEM_INFORMATION)

		ui.withFont(pionillium.details, function()

			textIcon(icons.maneuver)
			ui.text(lc.SECTOR_X_Y_Z % path)

			textIcon(icons.shield, lui.FACTION)
			ui.text(system.faction.name)

			textIcon(icons.galaxy_map)
			ui.text(numstarsText[system.numberOfStars])

			ui.withTooltip(lc.GOVERNMENT_TYPE, function()
				textIcon(icons.language)
				ui.textWrapped(system.govDescription)
			end)

			ui.withTooltip(lc.ECONOMY_TYPE, function()
				textIcon(icons.money)
				ui.textWrapped(system.econDescription)
			end)

			local pop = system.population -- population in billion
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

			textIcon(icons.starport)
			ui.text(lc.STARPORTS .. ": " .. system.numberOfStations)

		end)

		ui.separatorText(lui.HYPERJUMP_ROUTING)

		ui.withFont(pionillium.details, function()

			if not path:IsSameSystem(current) then

				ui.alignTextToFramePadding()
				draw_jump_status(getHyperspaceDetails(path))

				local padding = ui.theme.styles.FramePadding
				local size = ui.calcTextSize(lui.AUTO_ROUTE).x + padding.x * 2

				ui.sameLine(ui.getContentRegion().x - size, 0.0)

				if ui.button(lui.AUTO_ROUTE, Vector2(0, 0), nil, nil, padding) then
					hyperJumpPlanner.autoRoute(current, path)
				end

			end

		end)

		-- star list
		local stars = system:GetJumpable()
		local maxNumStars = math.min(#stars, 4)

		local rowHeight = ui.getTextLineHeight() + 2 * 2 -- CellPadding * 2
		local listSize = Vector2(0.0, rowHeight * maxNumStars + ui.getWindowPadding().y * 2)

		ui.child("##JumpList", listSize, nil, childFlags, function()
			drawJumpableList(stars, path)
		end)

		sidebarStyle:pop()

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

local function drawRouteList(route)
	if ui.beginTable("##RouteList", 4) then
		ui.tableSetupColumn("Index", "WidthFixed")
		ui.tableSetupColumn("Name", "WidthStretch")
		ui.tableSetupColumn("Distance", "WidthFixed")
		ui.tableSetupColumn("Fuel", "WidthFixed")

		local active = hyperJumpPlanner.getSelectedJump()

		for i, jump in ipairs(route) do
			ui.tableNextRow()
			ui.tableNextColumn()

			local clicked = ui.selectable("##Test" .. i, i == active, { "SpanAllColumns", "AllowDoubleClick" })
			ui.sameLine(0, ui.theme.styles.ItemInnerSpacing.x)

			if clicked then
				sectorView:SetSelectedPath(jump.path)
				hyperJumpPlanner.setSelectedJump(i)
			end

			if clicked and ui.isMouseDoubleClicked(0) then
				sectorView:GetMap():GotoSystemPath(jump.path)
			end

			ui.text(i)
			ui.tableNextColumn()

			ui.textColored(jump.color, jump.path:GetSystemBody().name)
			ui.tableNextColumn()

			ui.withFont(pionillium.details, function()
				ui.text(ui.get_icon_glyph(icons.route_dist) .. ui.Format.Number(jump.distance, 2) .. " " .. lc.UNIT_LY)
				if not jump.reachable then
					ui.sameLine()
					textIcon(icons.alert_generic, lui.INSUFFICIENT_FUEL)
				end

				ui.tableNextColumn()
				ui.text(ui.get_icon_glyph(icons.fuel) .. ui.Format.Number(jump.fuel, 1) .. " " .. lc.UNIT_TONNES)
			end)
		end

		ui.endTable()
	end
end

-- Route Planner UI
---@type UI.Sidebar.Module
local routeView = {
	icon = icons.route,
	tooltip = lui.HYPERJUMP_ROUTE,
	title = lui.ROUTE_INFO,
	active = true,

	currentFuel = 0.0,
	refresh = function(self)
		local engine = Game.player:GetInstalledHyperdrive()
		if not engine then
			self.currentFuel = 0.0
		else
			self.currentFuel = engine.storedFuel +
				Game.player:GetComponent('CargoManager'):CountCommodity(engine.fuel)
		end
	end,

	drawBody = function(self)

		local route = hyperJumpPlanner.getJumpRouteList()
		---@type SystemPath?
		local current = sectorView:GetCurrentSystemPath()
		if not current then return end

		sidebarStyle:push()

		local fuel, duration, distance = hyperJumpPlanner.getRouteStats()

		ui.withFont(pionillium.details, function()

		textIcon(icons.navtarget, lui.CURRENT_SYSTEM)
		if current then
			if ui.selectable(ui.Format.SystemPath(current)) then
				sectorView:SwitchToPath(current)
			end
		else
			ui.text("---")
		end

		textIcon(icons.route_destination, lui.FINAL_TARGET)
		if #route > 0 then
			local destination = route[#route]

			if ui.selectable(ui.Format.SystemPath(destination.path)) then
				sectorView:SwitchToPath(destination.path)
			end
		else
			ui.text("---")
		end

		textIcon(icons.fuel, lui.REQUIRED_FUEL)
		ui.horizontalGroup(function()
			ui.text(ui.Format.Mass(fuel * 1000, 1))

			ui.text("[")

			textIcon(icons.hull, lui.CURRENT_FUEL)
			ui.text(ui.Format.Mass(self.currentFuel * 1000, 1))

			ui.text("]")
		end)

		textIcon(icons.eta, lui.TOTAL_DURATION)
		ui.text(ui.Format.Duration(duration, 2))

		ui.sameLine()

		textIcon(icons.route_dist, lui.TOTAL_DISTANCE)
		ui.text(ui.Format.Number(distance, 2) .. " " .. lc.UNIT_LY)

		end)

		ui.spacing()

		ui.separatorText(lui.ROUTE_JUMPS)

		local jumpIndex = hyperJumpPlanner.getSelectedJump()

		ui.horizontalGroup(function()

			if ui.iconButton("ViewSystem", icons.view_internal, lui.CENTER_ON_SYSTEM) then
				if route[jumpIndex] then
					sectorView:SwitchToPath(route[jumpIndex].path)
					sectorView:GetMap():GotoSystemPath(route[jumpIndex].path)
				end
			end

			if ui.iconButton("MoveUp", icons.chevron_up, lui.MOVE_UP) then
				if route[jumpIndex] then
					hyperJumpPlanner.moveItemUp(jumpIndex)
				end
			end

			if ui.iconButton("MoveDown", icons.chevron_down, lui.MOVE_DOWN) then
				if route[jumpIndex] then
					hyperJumpPlanner.moveItemDown(jumpIndex)
				end
			end

			if ui.iconButton("Remove", icons.cross, lui.REMOVE_JUMP) then
				if route[jumpIndex] then
					hyperJumpPlanner.removeJump(jumpIndex)
				end
			end

		end)

		local padding = ui.getWindowPadding()
		ui.setNextWindowSizeConstraints(Vector2(0), Vector2(-1, (ui.getTextLineHeight() + 4) * 8 + padding.y * 2))

		ui.child("##routeJumps", Vector2(0, 0), nil, childFlags, function()

			ui.withStyleVars({ ItemSpacing = Vector2(4, 4) }, function()
				drawRouteList(route)
			end)

		end)

		sidebarStyle:pop()

	end
}

---@type UI.Sidebar.Module
local factionView = {
	icon = icons.shield,
	tooltip = lui.FACTIONS,
	title = lui.FACTIONS,
	drawBody = function(self)
		sidebarStyle:push()

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

		sidebarStyle:pop()
	end
}

---@type UI.Sidebar.Module
local optionView = {
	icon = icons.settings,
	tooltip = lui.SETTINGS,
	title = lui.SETTINGS,
	drawBody = function(self)
		sidebarStyle:push()

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

		sidebarStyle:pop()
	end
}

local bookmarkView = BookmarkView.New()

table.insert(leftSidebar.modules, infoView)
table.insert(leftSidebar.modules, econView)
table.insert(leftSidebar.modules, factionView)
table.insert(leftSidebar.modules, optionView)

table.insert(rightSidebar.modules, routeView)
table.insert(rightSidebar.modules, searchBar)
table.insert(rightSidebar.modules, bookmarkView)

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

local wnd_pivot_bottom = Vector2(0.5, 1.0)
local button_wnd_flags = ui.WindowFlags { "NoTitleBar", "NoMove", "AlwaysAutoResize", "NoScrollbar" }

local function drawEdgeButtons()

	local windowPos = Vector2(ui.screenWidth / 2, ui.screenHeight)
	ui.setNextWindowPos(windowPos, "Always", wnd_pivot_bottom)

	ui.window("##EdgeButtons", button_wnd_flags, function()
		ui.horizontalGroup(function()

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

			local target = player:GetHyperspaceTarget()
			local canJump = player:IsHyperjumpAllowed() and target and player:GetFlightState() == "FLYING" and player:CanHyperjumpTo(target)

			if not canJump then
				ui.mainMenuButton(icons.hyperspace, lui.HUD_BUTTON_HYPERDRIVE_DISABLED, ui.theme.buttonColors.disabled)
			elseif player:IsHyperspaceActive() then
				if ui.mainMenuButton(icons.hyperspace, lc.HYPERSPACE_JUMP_ABORT, ui.theme.buttonColors.selected) then
					player:AbortHyperjump()
				end
			else
				if ui.mainMenuButton(icons.hyperspace, lc.HYPERSPACE_JUMP_ENGAGE) then
					player:HyperjumpTo(target)
				end
			end

		end)
	end)

end

ui.registerModule("game", { id = 'map-sector-view', draw = function()
	player = Game.player
	if Game.CurrentView() == "SectorView" then

		if shouldRefresh then
			shouldRefresh = false
			leftSidebar:Refresh()
			rightSidebar:Refresh()
			hyperJumpPlanner.updateRouteList()
		end

		drawCurrentSystemName()

		ui.withFont(pionillium.body, function()
			leftSidebar:Draw()
			rightSidebar:Draw()

			ui.withStyleColors({
				WindowBg = colors.lightBlackBackground
			}, function()
				drawEdgeButtons()
			end)
		end)


		if ui.isKeyReleased(ui.keys.tab) then
			ui_visible = not ui_visible
			-- FIXME: label visibility is logically inverted from the parameter
			sectorView:GetMap():SetLabelsVisibility(not ui_visible)
		end

		if ui.escapeKeyReleased() then
			Game.SetView("WorldView")
		end

		if ui.ctrlHeld() and ui.isKeyReleased(ui.keys.delete) then
			package.reimport('pigui.modules.system-econ-view')
			bookmarkView:debugReload()
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
	hyperspaceDetailsCache = {}
	shouldRefresh = true
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

	for _, jump_sys in ipairs(sectorView:GetRoute()) do
		table.insert( data.jump_targets, jump_sys )
	end

	return data
end

local unserialize = function (data)
	loaded_data = data
end

Serializer:Register("HyperJumpPlanner", serialize, unserialize)


return {}
