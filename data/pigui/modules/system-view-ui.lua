-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local Engine = require 'Engine'
local Event = require 'Event'
local Lang = require 'Lang'
local ui = require 'pigui'
local Format = require 'Format'
local SpaceStation = require 'SpaceStation'
local Constants = _G.Constants

local Vector2 = _G.Vector2
local lc = Lang.GetResource("core")
local luc = Lang.GetResource("ui-core")
local layout = require 'pigui.libs.window-layout'
local Sidebar = require 'pigui.libs.sidebar'

local systemEconView = require 'pigui.modules.system-econ-view'.New()

local player = nil
local colors = ui.theme.colors
local icons = ui.theme.icons
local styles = ui.theme.styles

local systemView = Game and Game.systemView -- for hot-reload

local indicatorSize = Vector2(30, 30)
local bodyIconSize = Vector2(18, 18)

local selectedObject -- object, centered in SystemView

local hudfont = ui.fonts.pionillium.small
local hudfont_highlight = ui.fonts.pionillium.medium
local detailfont = ui.fonts.pionillium.medium
local winfont = ui.fonts.pionillium.medlarge

local atlasfont = ui.fonts.pionillium.medium
local atlasfont_highlight = ui.fonts.pionillium.medlarge

local atlas_line_length = ui.rescaleUI(24)
local atlas_label_offset = ui.rescaleUI(Vector2(12, -8))

local comboStyle = require 'pigui.styles'.combo

--load enums Projectable::types and Projectable::bases in one table "Projectable"
local Projectable = {}
for _, key in pairs(Constants.ProjectableTypes) do Projectable[key] = Engine.GetEnumValue("ProjectableTypes", key) end
for _, key in pairs(Constants.ProjectableBases) do Projectable[key] = Engine.GetEnumValue("ProjectableBases", key) end

-- all colors, used in this module
local svColor = {
	COMBAT_TARGET = colors.combatTarget,
	FONT = colors.font,
	GRID = colors.systemMapGrid,
	GRID_LEG = colors.systemMapGridLeg,
	LAGRANGE = colors.systemMapLagrangePoint,
	NAV_TARGET = colors.navTarget,
	OBJECT = colors.systemMapObject,
	PLANNER = colors.systemMapPlanner,
	PLANNER_ORBIT = colors.systemMapPlannerOrbit,
	PLAYER = colors.systemMapPlayer,
	PLAYER_ORBIT = colors.systemMapPlayerOrbit,
	SELECTED = ui.theme.colors.font,
	SELECTED_SHIP_ORBIT = colors.systemMapSelectedShipOrbit,
	SHIP = colors.systemMapShip,
	SHIP_ORBIT = colors.systemMapShipOrbit,
	SYSTEMBODY = colors.systemMapSystemBody,
	SYSTEMBODY_ICON = colors.systemMapSystemBodyIcon,
	SYSTEMBODY_ORBIT = colors.systemMapSystemBodyOrbit,
	UNKNOWN = colors.unknown
}

-- button states
local colorset = ui.theme.buttonColors

local buttonState = {
	[true]        = {                                  state = colorset.default },
	[false]       = {                                  state = colorset.transparent },
	DISABLED      = {                                  state = colorset.dark }
}

--- @param s string A string to test
--- @return boolean if string "s" is nil or the empty string
local function isEmptyString(s)
	return s == nil or s == ""
end

--- A helper-class to represent a combo dropdown state used to control the
--- visibility of something in the system view. Whenever the update() function
--- is called the visibility of the managed item is updated in the system view.
--- Options in the drop-down map to direct systemview visibility settings and
--- optionally there is support for a separate on/off toggle as well.
local SystemViewComboState = {
	new = function(self, o)
		o = o or {}
		setmetatable(o, self)
		self.__index = self
		return o
	end,

	--- The currently selected item
	selectedItem = 0,
	--- The human-readable strings to be displayed in the combo box
	items = {
	},
	--- The system view display modes corresponding to each item in items.
	displayModes = {
	},
	--- Whether the system view item should be displayed or not. This is only
	--- used when "displayModeOff" is not nil or empty.
	--- If "displayModeOff" is nil or empty, the "Off" system view mode should
	--- be part of "displayModes".
	doDisplay = false,
	--- The system view setting for when the item should not be displayed. When
	--- this is set, "configKeyDoDisplay" should also be set in order to
	--- correctly save the configuration.
	displayModeOff = nil,

	--- The game configuration key
	configKeySelectedItem = "",
	--- The game configuration key for doDisplay, should only be set if
	--- "displayModeOff" has been set.
	configKeyDoDisplay = nil,

	-- Returns the currently-selected mode
	getMode = function(self)
		if not self.doDisplay and not isEmptyString(self.displayModeOff) then
			return self.displayModeOff
		end
		return self.displayModes[self.selectedItem+1]
	end,

	-- update current mode and update the system view
	update = function(self, args)
		if args.doDisplay ~= nil then
			self.doDisplay = args.doDisplay
		end
		self.selectedItem = args.selectedItem or self.selectedItem
		systemView:SetVisibility(self:getMode())
	end,

	-- loads the configuration and applies it
	loadConfig = function(self)
		local selectedItemSetting = Game.GetConfigInt("SystemView", self.configKeySelectedItem)
		local doDisplaySetting = false
		if not isEmptyString(self.configKeyDoDisplay) then
			doDisplaySetting = Game.GetConfigBool("SystemView", self.configKeyDoDisplay)
		end
		self:update{
			doDisplay = doDisplaySetting,
			selectedItem = selectedItemSetting
		}
	end,

	-- saves the configuration
	saveConfig = function(self)
		Game.SetConfigInt("SystemView", self.configKeySelectedItem, self.selectedItem)
		if not isEmptyString(self.configKeyDoDisplay) then
			Game.SetConfigBool("SystemView", self.configKeyDoDisplay, self.doDisplay)
		end
	end,

	--- Toggle visibility on/off
	--- This function is only active if the "displayModeOff" member is set
	toggleDisplay = function(self)
		self:update{doDisplay = not self.doDisplay}
	end
}

local shipDisplayMode = SystemViewComboState:new{
	items = {
		lc.SHIPS_DISPLAY_MODE_SHIPS_ONLY,
		lc.SHIPS_DISPLAY_MODE_SHIPS_ORBITS
	},
	displayModes = {
		"SHIPS_ON",
		"SHIPS_ORBITS"
	},
	displayModeOff = "SHIPS_OFF",
	configKeySelectedItem = "ShipDisplayMode",
	configKeyDoDisplay = "ShipDisplayModeOn"
}

local cloudDisplayMode = SystemViewComboState:new{
	items = {
		lc.HYPERSPACE_CLOUDS_DISPLAY_MODE_ALL,
		lc.HYPERSPACE_CLOUDS_DISPLAY_MODE_ARRIVAL_ONLY,
		lc.HYPERSPACE_CLOUDS_DISPLAY_MODE_DEPARTURE_ONLY
	},
	displayModes = {
		"CLOUDS_ON",
		"CLOUDS_ARRIVAL",
		"CLOUDS_DEPARTURE"
	},
	displayModeOff = "CLOUDS_OFF",
	configKeySelectedItem = "CloudDisplayMode",
	configKeyDoDisplay = "CloudDisplayModeOn"
}

local gridDisplayMode = SystemViewComboState:new{
	items = {
		lc.GRID_DISPLAY_MODE_OFF,
		lc.GRID_DISPLAY_MODE_GRID_ONLY,
		lc.GRID_DISPLAY_MODE_GRID_AND_LEGS
	},
	displayModes = {
		"GRID_OFF",
		"GRID_ON",
		"GRID_AND_LEGS"
	},
	configKeySelectedItem = "GridDisplayMode",
}

local l4l5DisplayMode = SystemViewComboState:new{
	items = {
		lc.L4L5_DISPLAY_MODE_OFF,
		lc.L4L5_DISPLAY_MODE_ICONS_ONLY,
		lc.L4L5_DISPLAY_MODE_ICONS_AND_TEXT
	},
	displayModes = {
		"LAG_OFF",
		"LAG_ICON",
		"LAG_ICONTEXT"
	},
	configKeySelectedItem = "L4L5DisplayMode",
}

local onGameStart = function ()
	--connect to class SystemView
	systemView = Game.systemView
	--export several colors to class SystemView (only those which mentioned in the enum SystemViewColorIndex)
	for _, key in pairs(Constants.SystemViewColorIndex) do
		systemView:SetColor(key, svColor[key])
	end
	-- update visibility states
	shipDisplayMode:loadConfig()
	cloudDisplayMode:loadConfig()
	gridDisplayMode:loadConfig()
	l4l5DisplayMode:loadConfig()
end

local onGameEnd = function ()
	-- save visibility states
	shipDisplayMode:saveConfig()
	cloudDisplayMode:saveConfig()
	gridDisplayMode:saveConfig()
	l4l5DisplayMode:saveConfig()
	Engine.SaveSettings()
end

local onEnterSystem = function (ship)
	Game.systemView:SetVisibility("RESET_VIEW");
end

-- all windows in this view
local Windows = {
	systemName = layout.NewWindow("SystemMapSystemName"),
	edgeButtons = layout.NewWindow("SystemMapEdgeButtons"),
	timeButtons = layout.NewWindow("SystemMapTimeButtons"),
	unexplored = layout.NewWindow("SystemMapUnexplored")
}

Windows.systemName.style_colors["WindowBg"] = colors.transparent

local systemViewLayout = layout.New(Windows)
systemViewLayout.mainFont = winfont

local leftSidebar = Sidebar.New("##SystemMapLeft", "left")
local rightSidebar = Sidebar.New("##SystemMapRight", "right")

local systemOverviewWidget = require 'pigui.modules.system-overview-window'.New()
systemOverviewWidget.visible = true

function systemOverviewWidget:onBodySelected(sBody)
	systemView:SetSelectedObject(Projectable.OBJECT, Projectable.SYSTEMBODY, sBody)
end

function systemOverviewWidget:onBodyDoubleClicked(sBody)
	systemView:ViewSelectedObject()
end

--
-- Helper functions
--

local function textIcon(icon, tooltip)
	ui.icon(icon, Vector2(ui.getTextLineHeight()), svColor.FONT, tooltip)
	ui.sameLine()
end

local function showDvLine(leftIcon, resetIcon, rightIcon, key, Formatter, leftTooltip, resetTooltip, rightTooltip)
	local wheel = function()
		if ui.isItemHovered() then
			local w = ui.getMouseWheel()
			if w ~= 0 then
				systemView:TransferPlannerAdd(key, w * 10)
			end
		end
	end
	local id =  "##" .. key

	local press = ui.mainMenuButton(leftIcon, leftTooltip..id)
	if press or (key ~= "factor" and ui.isItemActive()) then
		systemView:TransferPlannerAdd(key, -10)
	end
	wheel()
	ui.sameLine()
	if ui.mainMenuButton(resetIcon, resetTooltip..id) then
		systemView:TransferPlannerReset(key)
	end
	wheel()
	ui.sameLine()
	press = ui.mainMenuButton(rightIcon, rightTooltip..id)
	if press or (key ~= "factor" and ui.isItemActive()) then
		systemView:TransferPlannerAdd(key, 10)
	end
	wheel()
	ui.sameLine()
	local speed, speed_unit = Formatter(systemView:TransferPlannerGet(key))
	ui.text(speed .. " " .. speed_unit)
	return 0
end

local time_selected_button_icon = icons.time_center

local function timeButton(icon, tooltip, factor)
	if ui.mainMenuButton(icon, tooltip) then
		time_selected_button_icon = icon
	end
	local active = ui.isItemActive()
	if active then
		systemView:AccelerateTime(factor)
	end
	ui.sameLine()
	return active
end

-- Format the mass in tonnes
-- The ui.Format() function reformats the units whereas we always want to
-- display using tonnes.
local function formatMass(massInTonnes)
	return string.format("%d %s", massInTonnes or 0, lc.UNIT_TONNES)
end

-- cache some data to reduce per-frame overheads
local data_cache = {
	body = {},
	prev_body = nil
}
-- populate a data structure with information about the currently selected body
local function getObjectData(obj)
	local isSystemBody = obj.base == Projectable.SYSTEMBODY
	local body = obj.ref

	local data = {}

	--SystemBody data is static so we use cache
	--Ship data migh be dynamic in the future
	if isSystemBody and data_cache.prev_body == body then
		data = data_cache.body
	else
		data_cache.prev_body = body

		if isSystemBody then -- system body
			local parent = body.parent
			local starport = body.superType == "STARPORT"
			local surface = body.type == "STARPORT_SURFACE"
			local sma = body.semiMajorAxis
			local semimajoraxis = nil
			if sma and sma > 0 then
				semimajoraxis = ui.Format.Distance(sma)
			end
			local rp = body.rotationPeriod * 24 * 60 * 60
			local op = body.orbitPeriod * 24 * 60 * 60
			local pop = math.round(body.population * 1e9)
			local techLevel = starport and SpaceStation.GetTechLevel(body) or nil
			if techLevel == 11 then
				techLevel = luc.MILITARY
			end
			data = {
				{ name = lc.MASS, icon = icons.body_radius,
					value = (not starport) and ui.Format.Mass(body.mass) or nil },
				{ name = lc.RADIUS, icon = icons.body_radius,
					value = (not starport) and ui.Format.Distance(body.radius) or nil },
				{ name = lc.SURFACE_TEMPERATURE, icon = icons.temperature,
					value = (not starport) and ui.Format.Temperature(body.averageTemp) or nil },
				{ name = lc.SURFACE_PRESSURE, icon = icons.pressure,
					value = (not starport) and ui.Format.Pressure(body.surfacePressure) or nil },
				{ name = lc.SURFACE_GRAVITY, icon = icons.body_radius,
					value = (not starport) and ui.Format.Speed(body.gravity, true).."²"..
												" ("..ui.Format.Gravity(body.gravity / 9.80665)..")" or nil },
				{ name = lc.ESCAPE_VELOCITY, icon = icons.body_radius,
					value = (not starport) and ui.Format.Speed(body.escapeVelocity , true) or nil },
				{ name = lc.MEAN_DENSITY, icon = icons.body_radius,
					value = (not starport) and ui.Format.Number(body.meanDensity, 0).." "..lc.UNIT_DENSITY or nil },
				{ name = lc.ORBITAL_PERIOD, icon = icons.body_orbit_period,
					value = op and op > 0 and ui.Format.Duration(op, 2) or nil },
				{ name = lc.DAY_LENGTH, icon = icons.body_day_length,
					value = rp > 0 and ui.Format.Duration(rp, 2) or nil },
				{ name = luc.ORBIT_APOAPSIS, icon = icons.body_semi_major_axis,
					value = (parent and not surface) and ui.Format.Distance(body.apoapsis) or nil },
				{ name = luc.ORBIT_PERIAPSIS, icon = icons.body_semi_major_axis,
					value = (parent and not surface) and ui.Format.Distance(body.periapsis) or nil },
				{ name = lc.SEMI_MAJOR_AXIS, icon = icons.body_semi_major_axis,
					value = semimajoraxis },
				{ name = lc.ECCENTRICITY, icon = icons.body_semi_major_axis,
					value = (parent and not surface) and string.format("%0.2f", body.eccentricity) or nil },
				{ name = lc.AXIAL_TILT, icon = icons.body_semi_major_axis,
					value = (not starport) and string.format("%0.2f", body.axialTilt) or nil },
				{ name = lc.POPULATION, icon = icons.personal,
					value = pop > 0 and ui.Format.NumberAbbv(pop) or nil },
				{ name = luc.TECH_LEVEL, icon = icons.equipment,
					value = starport and techLevel or nil }
			}

			--change the internal cached data only when new is fully built
			--prevents additional flickering
			data_cache.body = data

		elseif obj.ref:IsShip() then -- physical body
			---@cast body Ship
			-- TODO: the advanced target scanner should add additional data here,
			-- but we really do not want to hardcode that here. there should be
			-- some kind of hook that the target scanner can hook into to display
			-- more info here.
			-- This is what should be inserted:
			table.insert(data, { name = luc.SHIP_TYPE, value = body:GetShipType() })
			if (player["target_scanner_level_cap"] or 0) > 0 then
				local hd = body:GetInstalledHyperdrive()
				table.insert(data, { name = luc.HYPERDRIVE, value = hd and hd:GetName() or lc.NO_HYPERDRIVE })
				table.insert(data, { name = luc.MASS, value = Format.MassTonnes(body.staticMass) })
				table.insert(data, { name = luc.CARGO, value = Format.MassTonnes(body.usedCargo) })
			end
		elseif obj.ref:IsHyperspaceCloud() then
			---@cast body HyperspaceCloud
			local hypercloud_level = (player["hypercloud_analyzer_cap"] or 0)
			local ship = body:GetShip()
			-- TODO: different levels of hypercloud analyser should provide
			-- increasingly detailed amounts of information.
			if hypercloud_level > 0 and ship then
				local _,systemName = ship:GetHyperspaceDestination()
				local systemLabel = body:IsArrival() and luc.HUD_HYPERSPACE_ORIGIN or luc.HUD_HYPERSPACE_DESTINATION

				table.insert(data, { name = luc.SHIP_TYPE, value = ship:GetShipType() })
				table.insert(data, { name = luc.HUD_MASS, value = formatMass(ship.staticMass) })
				table.insert(data, { name = systemLabel, value = systemName })
				table.insert(data, { name = luc.HUD_ARRIVAL_DATE, value = ui.Format.Datetime(body:GetDueDate()) })
			end
		else
			data = {}
		end
	end
	return data
end

-- render a two-column dataset
local function tabular(data, maxSize)
	if data and #data > 0 then
		ui.columns(2, "Attributes", false)
		local nameWidth = 0
		local valueWidth = 0

		for _,item in pairs(data) do
			if item.value then
				local nWidth = ui.calcTextSize(item.name).x + ui.getItemSpacing().x
				local vWidth = ui.calcTextSize(item.value).x + ui.getItemSpacing().x
				if ui.getColumnWidth() < nWidth then
					textIcon(item.icon or icons.info, item.name)
				else
					ui.text(item.name)
				end
				ui.nextColumn()
				ui.text(item.value)
				ui.nextColumn()

				nameWidth = math.max(nameWidth, nWidth)
				valueWidth = math.max(valueWidth, vWidth)
			end
		end
		if maxSize > 0 and nameWidth + valueWidth > maxSize then
			-- first of all, we want to see the values, but the keys should not be too small either
			nameWidth = math.max(maxSize - valueWidth, maxSize * 0.1)
		end
		ui.setColumnWidth(0, nameWidth)

		-- reset columns
		ui.columns(1)
	end
end

local _getBodyIcon = require 'pigui.modules.flight-ui.body-icons'
local function getBodyIcon(obj, forWorld, isOrrery)
	if obj.type == Projectable.APOAPSIS then return icons.apoapsis
	elseif obj.type == Projectable.PERIAPSIS then return icons.periapsis
	elseif obj.type == Projectable.L4 then return icons.lagrange_marker
	elseif obj.type == Projectable.L5 then return icons.lagrange_marker
	elseif obj.base == Projectable.PLAYER or obj.base == Projectable.PLANNER then
		local shipClass = obj.ref:GetShipClass()
		if icons[shipClass] then
			return icons[shipClass]
		else
			return icons.ship
		end
	elseif forWorld and not isOrrery and obj.ref.superType ~= "STARPORT" and obj.ref.type ~= "PLANET_ASTEROID" then
		return icons.empty
	else
		return _getBodyIcon(obj.ref, forWorld)
	end
end


--
-- The sidebar views
--

---@type UI.Sidebar.Module
local overviewView = {
	showInHyperspace = false,
	icon = icons.system_overview,
	tooltip = luc.TOGGLE_OVERVIEW_WINDOW,
	exclusive = true,

	drawTitle = function()
		if Windows.unexplored.visible then
			ui.text(lc.UNEXPLORED_SYSTEM_NO_SYSTEM_VIEW)
		else
			systemOverviewWidget:displaySidebarTitle(systemView:GetSystem())
		end
	end,

	drawBody = function()
		local system = systemView:GetSystem() ---@type StarSystem
		if not system or Windows.unexplored.visible then
			return
		end

		systemOverviewWidget:displaySearch()

		systemOverviewWidget.size.y = math.max(ui.getContentRegion().y, ui.screenHeight / 1.6)

		local root = system.rootSystemBody
		local selected = { [systemView:GetSelectedObject().ref or 0] = true }

		systemOverviewWidget:display(Game.system, root, selected)
	end
}

---@type UI.Sidebar.Module
local economyView = {
	showInHyperspace = false,
	icon = icons.money,
	tooltip = luc.ECONOMY_TRADE,
	exclusive = true,

	drawTitle = function()
		ui.text(luc.ECONOMY_TRADE)
	end,

	drawBody = function()
		local selected = systemView:GetSelectedObject().ref ---@type SystemBody?
		local docked = Game.player:GetDockedWith()
		local current

		if docked then
			if not selected or not selected.isStation then
				selected = docked:GetSystemBody()
			else
				current = docked:GetSystemBody()
			end
		end

		if not selected or not selected.isStation then
			systemEconView:drawSystemComparison(systemView:GetSystem())
		else
			systemEconView:drawStationComparison(selected, current)
		end
	end
}

---@type UI.Sidebar.Module
local settingsView = {
	icon = icons.settings,
	tooltip = luc.SETTINGS,
	title = luc.SETTINGS,

	drawBody = function(self)
		if systemView:GetDisplayMode() == "Orrery" then
			comboStyle:withStyle(function()
				local c,ret

				ui.text(lc.SHIPS_DISPLAY_MODE)
				c,ret = ui.combo("##Ships", shipDisplayMode.selectedItem, shipDisplayMode.items)
				if c then
					shipDisplayMode:update{selectedItem = ret}
				end

				ui.text(lc.L4L5_DISPLAY_MODE)
				c,ret = ui.combo("##LagrangePoint", l4l5DisplayMode.selectedItem, l4l5DisplayMode.items)
				if c then
					l4l5DisplayMode:update{selectedItem = ret}
				end

				ui.text(lc.GRID_DISPLAY_MODE)
				c,ret = ui.combo("##Grid", gridDisplayMode.selectedItem, gridDisplayMode.items)
				if c then
					gridDisplayMode:update{selectedItem = ret}
				end

				ui.text(lc.HYPERSPACE_CLOUDS_DISPLAY_MODE)
				c,ret = ui.combo("##Cloud", cloudDisplayMode.selectedItem, cloudDisplayMode.items)
				if c then
					cloudDisplayMode:update{selectedItem = ret}
				end
			end)
		end
	end
}

---@type UI.Sidebar.Module
local infoView = {
	icon = icons.info,
	tooltip = lc.OBJECT_INFO,
	showInHyperspace = false,
	--exclusive = true,

	drawTitle = function()
		local obj = systemView:GetSelectedObject()
		if obj ~= nil and obj.ref ~= nil then
			local isSystemBody = obj.base == Projectable.SYSTEMBODY
			local body = obj.ref
			ui.text(ui.get_icon_glyph(getBodyIcon(obj)) .. " " .. (isSystemBody and body.name or body.label))
		end
	end,

	drawBody = function()
		local obj = systemView:GetSelectedObject()

		if obj ~= nil and obj.ref ~= nil then
			local isSystemBody = obj.base == Projectable.SYSTEMBODY
			local body = obj.ref

			if isSystemBody then
				ui.textWrapped(body.astroDescription)

				ui.separator()
				ui.spacing()
			end

			local data = getObjectData(obj)
			tabular(data, 0)
		end
	end
}

---@type UI.Sidebar.Module
local plannerView = {
	icon = icons.semi_major_axis,
	tooltip = lc.ORBIT_PLANNER,
	title = lc.ORBIT_PLANNER,
	disabled = false,

	drawBody = function(self)
		showDvLine(icons.decrease, icons.delta, icons.increase, "factor", function(i) return i, "x" end, luc.DECREASE, lc.PLANNER_RESET_FACTOR, luc.INCREASE)
		showDvLine(icons.decrease, icons.clock, icons.increase, "starttime",
			function(_)
				local now = Game.time
				local start = systemView:GetOrbitPlannerStartTime()
				if start then
					return ui.Format.Duration(math.floor(start - now)), ""
				else
					return lc.NOW, ""
				end
			end,
			luc.DECREASE, lc.PLANNER_RESET_START, luc.INCREASE)
		showDvLine(icons.decrease, icons.orbit_prograde, icons.increase, "prograde", ui.Format.SpeedUnit, luc.DECREASE, lc.PLANNER_RESET_PROGRADE, luc.INCREASE)
		showDvLine(icons.decrease, icons.orbit_normal, icons.increase, "normal", ui.Format.SpeedUnit, luc.DECREASE, lc.PLANNER_RESET_NORMAL, luc.INCREASE)
		showDvLine(icons.decrease, icons.orbit_radial, icons.increase, "radial", ui.Format.SpeedUnit, luc.DECREASE, lc.PLANNER_RESET_RADIAL, luc.INCREASE)
	end
}

table.insert(leftSidebar.modules, infoView)
table.insert(leftSidebar.modules, economyView)
table.insert(leftSidebar.modules, settingsView)

table.insert(rightSidebar.modules, overviewView)
table.insert(rightSidebar.modules, plannerView)

--
-- Window functions
--

function Windows.edgeButtons.ShouldShow()
	return true
end

function Windows.timeButtons.ShouldShow()
	return systemView:GetDisplayMode() == 'Orrery'
end

function Windows.edgeButtons.Show()
	local isCurrent = systemView:GetSystemSelectionMode() == "CURRENT_SYSTEM"
	local isOrrery = systemView:GetDisplayMode() == "Orrery"

	ui.horizontalGroup(function()
		-- system selection button (current/selected)
		if not isCurrent and ui.mainMenuButton(icons.planet_grid, luc.HUD_BUTTON_SWITCH_TO_CURRENT_SYSTEM) then
			systemView:SetSystemSelectionMode("CURRENT_SYSTEM")
		end
		if isCurrent and ui.mainMenuButton(icons.galaxy_map, luc.HUD_BUTTON_SWITCH_TO_SELECTED_SYSTEM) then
			systemView:SetSystemSelectionMode("SELECTED_SYSTEM")
		end

		-- view control buttons (reset, rotate, zoom)
		if ui.mainMenuButton(icons.reset_view, luc.RESET_ORIENTATION_AND_ZOOM) then
			systemView:SetVisibility("RESET_VIEW")
		end
		if isOrrery then
			ui.mainMenuButton(icons.rotate_view, luc.ROTATE_VIEW)
		else
			ui.mainMenuButton(icons.distance, luc.TRANSLATE_VIEW)
		end
		systemView:SetRotateMode(ui.isItemActive())
		ui.mainMenuButton(icons.search_lens, luc.ZOOM)
		systemView:SetZoomMode(ui.isItemActive())

		-- view settings buttons
		if isOrrery then
			ui.spacing()
			if ui.mainMenuButton(icons.ships_no_orbits, lc.SHIPS_DISPLAY_MODE_TOGGLE, buttonState[shipDisplayMode.doDisplay].state) then
				shipDisplayMode:toggleDisplay()
			end
			if ui.mainMenuButton(icons.hyperspace, lc.HYPERSPACE_CLOUDS_DISPLAY_MODE_TOGGLE, buttonState[cloudDisplayMode.doDisplay].state) then
				cloudDisplayMode:toggleDisplay()
			end
		end
	end)
end

-- time conntrol buttons
function Windows.timeButtons.Show()
	local t = systemView:GetOrbitPlannerTime()
	ui.text(t and ui.Format.Datetime(t) or lc.NOW)
	local r = false
	r = timeButton(icons.time_backward_100x, "-10,000,000x",-10000000) or r
	r = timeButton(icons.time_backward_10x, "-100,000x", -100000) or r
	r = timeButton(icons.time_backward_1x, "-1,000x", -1000) or r
	r = timeButton(icons.time_center, lc.NOW, nil) or r
	r = timeButton(icons.time_forward_1x, "1,000x", 1000) or r
	r = timeButton(icons.time_forward_10x, "100,000x", 100000) or r
	r = timeButton(icons.time_forward_100x, "10,000,000x", 10000000) or r
	if not r then
		if time_selected_button_icon == icons.time_center then
			systemView:AccelerateTime(nil)
		else
			systemView:AccelerateTime(0.0)
		end
	end
end

local function getLabel(obj)
	if obj.type == Projectable.OBJECT then
		if obj.base == Projectable.SYSTEMBODY then return obj.ref.name
		elseif obj.base == Projectable.PLANNER then return ""
		else return obj.ref:GetLabel() end
	--elseif obj.type == Projectable.L4 and show_lagrange == "LAG_ICONTEXT" then return "L4"
	elseif obj.type == Projectable.L4 and l4l5DisplayMode:getMode() == "LAG_ICONTEXT" then return "L4"
	--elseif obj.type == Projectable.L4 and show_lagrange == "LAG_ICONTEXT" then return "L4"
	elseif obj.type == Projectable.L5 and l4l5DisplayMode:getMode() == "LAG_ICONTEXT" then return "L5"
	else return ""
	end
end

local function getColor(obj)
	if obj.type == Projectable.OBJECT then
		if obj.base == Projectable.SYSTEMBODY then return svColor.SYSTEMBODY_ICON
		elseif obj.base == Projectable.SHIP then return svColor.SHIP
		elseif obj.base == Projectable.PLAYER then return svColor.PLAYER
		elseif obj.base == Projectable.PLANNER then return svColor.PLANNER
		else return svColor.OBJECT
		end
	elseif obj.type == Projectable.APOAPSIS or obj.type == Projectable.PERIAPSIS then
		if obj.base == Projectable.SYSTEMBODY then return svColor.SYSTEMBODY_ORBIT
		elseif obj.base == Projectable.SHIP then
			if obj.ref == selectedObject then return svColor.SELECTED_SHIP_ORBIT
			else return svColor.SHIP_ORBIT
			end
		elseif obj.base == Projectable.PLAYER then return svColor.PLAYER_ORBIT
		elseif obj.base == Projectable.PLANNER then return svColor.PLANNER_ORBIT
		else return svColor.UNKNOWN -- unknown base
		end
	elseif obj.type == Projectable.L4 or obj.type == Projectable.L5 then return svColor.LAGRANGE
	else return svColor.UNKNOWN
	end
end

function Windows.systemName.Show()
	local path
	if systemView:GetSystemSelectionMode() == "SELECTED_SYSTEM" then
		path = Game.sectorView:GetSelectedSystemPath()
	else
		path = systemView:GetSystem().path
	end
	ui.text(ui.Format.SystemPath(path))
end

local function drawGroupIcons(coords, icon, color, iconSize, group, isSelected)
	-- indicators
	local stackedSize = indicatorSize
	local stackStep = Vector2(10, 10)
	if isSelected then
		ui.addIcon(coords, icons.square, svColor.SELECTED, stackedSize, ui.anchor.center, ui.anchor.center)
		stackedSize = stackedSize + stackStep
	end
	if group.hasPlayer then
		ui.addIcon(coords, icons.square, svColor.PLAYER, stackedSize, ui.anchor.center, ui.anchor.center)
		stackedSize = stackedSize + stackStep
	end
	if group.hasNavTarget then
		ui.addIcon(coords, icons.square, svColor.NAV_TARGET, stackedSize, ui.anchor.center, ui.anchor.center)
		stackedSize = stackedSize + stackStep
	end
	if group.hasCombatTarget then
		ui.addIcon(coords, icons.square, svColor.COMBAT_TARGET, stackedSize, ui.anchor.center, ui.anchor.center)
		stackedSize = stackedSize + stackStep
	end
	if group.hasPlanner then
		ui.addIcon(coords, icons.square, svColor.PLANNER, stackedSize, ui.anchor.center, ui.anchor.center)
		stackedSize = stackedSize + stackStep
	end

	ui.addIcon(coords, icon, color, iconSize, ui.anchor.center, ui.anchor.center)
end

-- handle positioning and drawing a label for the given object in Atlas mode
local function drawAtlasBodyLabel(label, screenSize, mainCoords, isHovered, isSelected)
	-- Larger font for hovered bodies, slight emphasis on the selected body
	local font = isHovered and atlasfont_highlight or atlasfont
	local fontColor = isSelected and colors.systemAtlasLabelActive or colors.systemAtlasLabel
	local lineColor = isSelected and colors.systemAtlasLineActive or colors.systemAtlasLine

	local textSize = ui.calcTextSize(label, font)
	-- lineOffset is half the screen-size radius of the body
	local lineOffsetSize = math.max(screenSize * 0.66, bodyIconSize.x * 0.5) -- most icons use about 60% of the actual radius
	-- lineLength is how long to draw the "pointer" line between the label and the edge of the body
	local lineLength = (atlas_line_length / math.max(systemView:GetZoom(), 1.0)) * (isHovered and 1.0 or 0.6)

	local lineStartPos = mainCoords + Vector2(lineOffsetSize, -lineOffsetSize * 0.667)
	local lineEndPos = lineStartPos + Vector2(lineLength, -lineLength)
	local underlinePos = lineEndPos + Vector2(textSize.x + atlas_label_offset.x * 2, 0)

	-- draw a background behind the label, then an indicator line
	if isHovered then
		ui.addRectFilled(lineEndPos - Vector2(0, -atlas_label_offset.y + textSize.y), underlinePos, colors.lightBlackBackground, 4, ui.RoundCornersNone)
		ui.addLine(lineStartPos, lineEndPos, lineColor, 2)
		ui.addLine(lineEndPos, underlinePos, lineColor, 3)
	end

	-- draw the label and it's shadow for clarity
	local labelPos = (isHovered and lineEndPos or lineStartPos) + atlas_label_offset
	local shadowPos = labelPos + Vector2(2, 1)

	ui.addStyledText(shadowPos, ui.anchor.left, ui.anchor.baseline, label, colors.black, font)
	ui.addStyledText(labelPos, ui.anchor.left, ui.anchor.baseline, label, fontColor, font)
end

Windows.unexplored.visible = false
function Windows.unexplored.Show()
	ui.text(lc.UNEXPLORED_SYSTEM_NO_SYSTEM_VIEW)
end

local function checkSurfPorts(sbody)
	local children = sbody.children
	if #children == 0 then return nil end
	local ports = {}
	local has_indicators
	local navTarget = Game.player:GetNavTarget()
	local selected = systemView:GetSelectedObject().ref
	for _, child in ipairs(children) do
		if child.type == "STARPORT_SURFACE" then
			table.insert(ports, child)
			local body = child.physicsBody
			has_indicators = has_indicators or body and body == navTarget or child == selected
		end
	end
	if #ports == 0 then return nil end
	return ports, has_indicators
end

local function displaySurfPorts(center, radius, ports, mouseover)
	local nports = #ports
	if nports == 0 then return nil end
	local icon_size = bodyIconSize.x
	-- pretty rough estimate
	local min_rad = icon_size * nports / ui.twoPi
	local pixPerUnit = systemView:AtlasViewPixelPerUnit()
	local gap = systemView:AtlasViewPlanetGap(radius / pixPerUnit) * pixPerUnit
	local icons_radius = math.max(icon_size, min_rad, radius + icon_size / 2)
	local can_show_ring = icons_radius + icon_size / 2 < radius + gap or nports < 3

	local start_ang = math.deg2rad(225)
	local step_ang = ui.twoPi / nports
	local hover_i = 0
	local hover_center
	local select_i = 0
	local select_center
	local navtarget_in_ring = false
	for i = 1, nports do
		local ang = start_ang + step_ang * (i - 1)
		local p_center = center + Vector2(math.cos(ang), math.sin(ang)) * icons_radius
		-- HACK we pretend to be a group, to use drawGroupIcons
		local body = ports[i].physicsBody
		local group = { hasNavTarget = body and body == Game.player:GetNavTarget() }
		local isSelected = ports[i] == systemView:GetSelectedObject().ref
		if can_show_ring then
			drawGroupIcons(p_center, _getBodyIcon(ports[i]), svColor.SYSTEMBODY_ICON, bodyIconSize, group, isSelected)
			-- hover mouse
			if (ui.getMousePos() - p_center):length() < icon_size / 2 then
				hover_i = i
				hover_center = p_center
			end
		else
			navtarget_in_ring = navtarget_in_ring or group.hasNavTarget
		end
		if isSelected then
			select_i = i
			select_center = p_center
		end
	end
	if not can_show_ring then
		local p_center = center + Vector2(math.cos(start_ang), math.sin(start_ang)) * icons_radius
		local group = { hasNavTarget = navtarget_in_ring }
		drawGroupIcons(p_center, icons.plus, svColor.SYSTEMBODY_ICON, bodyIconSize, group, select_i ~= 0)
		return nil
	end
	if hover_i ~= 0 then
		return ports[hover_i], hover_center
	elseif select_i ~= 0 and not mouseover then
		return ports[select_i], select_center
	else
		return nil
	end
end

local popup_object
function makePopup()
	ui.popup("system-view-ui-popup", function()
		local isOrrery = systemView:GetDisplayMode() == "Orrery"
		local combatTarget = player:GetCombatTarget()
		local navTarget = Game.player:GetNavTarget()
		local isObject = popup_object.type == Projectable.OBJECT
		local isSystemBody = isObject and popup_object.base == Projectable.SYSTEMBODY
		local isShip = isObject and not isSystemBody and popup_object.ref:IsShip()
		local isCloud = isObject and not isSystemBody and popup_object.ref:IsHyperspaceCloud()
		ui.text(getLabel(popup_object))
		ui.separator()
		if isOrrery and ui.selectable(lc.CENTER, false, {}) then
			selectedObject = popup_object.ref
			systemView:SetSelectedObject(popup_object.type, popup_object.base, popup_object.ref)
			systemView:ViewSelectedObject()
		end
		if (isShip or isCloud or isSystemBody and popup_object.ref.physicsBody) and ui.selectable(lc.SET_AS_TARGET, false, {}) then
			if isSystemBody then
				player:SetNavTarget(popup_object.ref.physicsBody)
				ui.playSfx("OK")
			elseif isCloud then
				player:SetNavTarget(popup_object.ref)
				ui.playSfx("OK")
			else
				if combatTarget == popup_object.ref then player:SetCombatTarget(nil) end
				player:SetNavTarget(popup_object.ref)
				ui.playSfx("OK")
			end
		end
		if isShip and ui.selectable(lc.SET_AS_COMBAT_TARGET, false, {}) then
			if navTarget == popup_object.ref then player:SetNavTarget(nil) end
			player:SetCombatTarget(popup_object.ref)
		end
	end)
end

local expanded_hover_body = nil -- body with expanded hover area (atlas view only)
-- forked from data/pigui/views/game.lua
local function displayOnScreenObjects()
	local isOrrery = systemView:GetDisplayMode() == 'Orrery'

	local should_show_label = isOrrery and ui.shouldShowLabels()

	local label_offset = 14 -- enough so that the target rectangle fits
	local collapse = bodyIconSize -- size of clusters to be collapsed into single bodies
	local click_radius = collapse:length() * 0.5
	if not isOrrery then
		click_radius = collapse:length() * 0.8 / systemView:GetZoom()
	end
	-- make click_radius sufficiently smaller than the cluster size
	-- to prevent overlap of selection regions
	local objectCounter = 0
	local objects_grouped = systemView:GetProjectedGrouped(collapse, 1e64)

	-- if there's nothing to display, we're an unexplored system
	Windows.unexplored.visible = #objects_grouped == 0
	if Windows.unexplored.visible then return end

	local hoveredObject = nil
	local was_hovered = false
	local atlas_label_objects = {}

	for _,group in ipairs(objects_grouped) do
		local mainObject = group.mainObject

		local mainCoords = Vector2(group.screenCoordinates.x, group.screenCoordinates.y)
		local isSelected = mainObject.type == Projectable.OBJECT and mainObject.ref == systemView:GetSelectedObject().ref
		group.hasPlanner = mainObject.type == Projectable.OBJECT and mainObject.base == Projectable.PLANNER
		local hoverSize = group.screenSize

		local ports = {} -- ground ports
		local should_show_ports = false -- if we want to show ports even if we are not hovered
		if not isOrrery and mainObject.type == Projectable.OBJECT and mainObject.base == Projectable.SYSTEMBODY then
			-- should show ports if there is some indicators
			ports, should_show_ports = checkSurfPorts(mainObject.ref)
			-- or if the central body is selected
			should_show_ports = should_show_ports or isSelected
			-- if we hover over a body with ports, we expand its hover radius to make it easier to hover ports
			if mainObject.ref == expanded_hover_body or should_show_ports then hoverSize = group.screenSize + bodyIconSize.x end
		end

		drawGroupIcons(mainCoords, getBodyIcon(mainObject, true, isOrrery), getColor(mainObject), bodyIconSize, group, isSelected)

		local mp = ui.getMousePos()
		local label = getLabel(mainObject)
		local mouseover = not ui.isAnyWindowHovered() and
			(mp - mainCoords):length() < (isOrrery and click_radius or math.max(click_radius, hoverSize))

		if #label > 0 and (should_show_label or mouseover or should_show_ports) then
			if group.objects then
				label = label .. " (" .. #group.objects .. ")"
			end

			if isOrrery then
				local pos = mainCoords + Vector2(label_offset, 0)
				local hovered = mouseover and mainObject.type == Projectable.OBJECT
				local font = (hovered or isSelected) and hudfont_highlight or hudfont
				ui.addStyledText(pos + Vector2(2, 1), ui.anchor.left, ui.anchor.center, label , ui.theme.colors.black, font)
				ui.addStyledText(pos, ui.anchor.left, ui.anchor.center, label , getColor(mainObject), font)
			else
				local labeled = nil -- hovered or selected
				local labeled_center = nil
				if ports then
					if mouseover then
						was_hovered = true
						expanded_hover_body = mainObject.ref
					end
					labeled, labeled_center = displaySurfPorts(mainCoords, group.screenSize, ports, mouseover)
				end
				if labeled then
					-- HACK we just replace the current object with its selected or hovered ground port (if has)
					mainObject.type = Projectable.OBJECT
					mainObject.base = Projectable.SYSTEMBODY
					mainObject.ref = labeled
					mainCoords = labeled_center
					group.screenSize = bodyIconSize.x
					isSelected = labeled == systemView:GetSelectedObject().ref
					label = labeled.name
				end
				table.insert(atlas_label_objects, { label, group.screenSize, mainCoords, mouseover, isSelected })
			end
		end

		if mainObject.type == Projectable.OBJECT and (mainObject.base == Projectable.SYSTEMBODY or mainObject.base == Projectable.SHIP or mainObject.base == Projectable.PLAYER or mainObject.base == Projectable.OBJECT) then
			-- mouse release handler for right button
			if mouseover then
				if not ui.isAnyWindowHovered() and ui.isMouseReleased(1) then
					popup_object = mainObject
					ui.openPopup("system-view-ui-popup")
				end
			end
		end
		-- mouse release handler for left button
		if mouseover and mainObject.type == Projectable.OBJECT then
			hoveredObject = mainObject
		end
		objectCounter = objectCounter + 1
	end
	makePopup()
	if not was_hovered then expanded_hover_body = nil end

	-- atlas body labels have to be drawn after icons for proper ordering
	for _, v in ipairs(atlas_label_objects) do
		drawAtlasBodyLabel(table.unpack(v))
	end

	-- click once: select or deselect a body
	-- double click: zoom to body or reset viewpoint
	local clicked = not ui.isAnyWindowHovered()
	                and not ui.ctrlHeld()
	                and (ui.isMouseClicked(0) or ui.isMouseDoubleClicked(0))
	if clicked then
		if hoveredObject then
			selectedObject = hoveredObject.ref
			systemView:SetSelectedObject(hoveredObject.type, hoveredObject.base, hoveredObject.ref)
			if ui.isMouseDoubleClicked(0) then systemView:ViewSelectedObject() end
		else
			selectedObject = nil
			systemView:ClearSelectedObject()
			if ui.isMouseDoubleClicked(0) then systemView:ResetViewpoint() end
		end
	end
end

function systemViewLayout:onUpdateWindowPivots(w)
	w.systemName.anchors   = { ui.anchor.center, ui.anchor.top }
	w.edgeButtons.anchors  = { ui.anchor.center,  ui.anchor.bottom }
	w.timeButtons.anchors  = { ui.anchor.right,  ui.anchor.bottom }
	w.unexplored.anchors   = { ui.anchor.center, ui.anchor.center }
end

function systemViewLayout:onUpdateWindowConstraints(w)
	-- resizing, aligning windows - static
	w.systemName.pos.x = ui.screenWidth * 0.5
	w.systemName.pos.y = styles.MainButtonSize.y + styles.WindowPadding.y * 2 -- matches fx-window.lua
	w.systemName.size.x = 0 -- adaptive width

	w.edgeButtons.size.x = 0 -- adaptive width
end

local function displaySystemViewUI()
	if not systemView then onGameStart() end

	if not ui.shouldDrawUI() then return end

	player = Game.player
	if Game.CurrentView() == "SystemView" then
		if ui.isKeyReleased(ui.keys.tab) then
			systemViewLayout.enabled = not systemViewLayout.enabled
		end

		plannerView.disabled = systemView:GetDisplayMode() ~= "Orrery"
		plannerView.icon = plannerView.disabled and icons.square_dashed or icons.semi_major_axis

		systemViewLayout:display()
		if systemViewLayout.enabled then
			ui.withStyleColors({ WindowBg = colors.transparent }, function()
				leftSidebar:Draw()
				rightSidebar:Draw()
			end)
		end

		displayOnScreenObjects()

		if ui.escapeKeyReleased() then
			Game.SetView("SectorView")
		end

		if ui.ctrlHeld() and ui.isKeyReleased(ui.keys.delete) then
			package.reimport 'pigui.modules.system-overview-window'
			systemEconView = package.reimport('pigui.modules.system-econ-view').New()
			package.reimport()
		end
	end
end

Event.Register("onGameStart", onGameStart)
Event.Register("onGameEnd", onGameEnd)
Event.Register("onEnterSystem", onEnterSystem)
ui.registerHandler("SystemView", ui.makeFullScreenHandler("SystemView", displaySystemViewUI))

return {}
