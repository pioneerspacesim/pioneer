-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Event = require 'Event'
local Input = require 'Input'
local Game = require 'Game'
local utils= require 'utils'
local Vector2 = _G.Vector2

local Lang = require 'Lang'
local lc = Lang.GetResource("core");

local ui = require 'pigui'

local vutil = require 'pigui.libs.view-util'
local Sidebar = require 'pigui.libs.sidebar'

-- cache ui
local pionillium = ui.fonts.pionillium
local colors = ui.theme.colors

local reticuleCircleRadius = math.min(ui.screenWidth, ui.screenHeight) / 8
local reticuleCircleThickness = 2.0

local lastTimeAcceleration

-- for modules
ui.reticuleCircleRadius = reticuleCircleRadius
ui.reticuleCircleThickness = reticuleCircleThickness

-- settings
local IN_SPACE_INDICATOR_SHIP_MAX_DISTANCE = 1000000 -- ships farther away than this don't show up on as in-space indicators

-- cache some data each frame
local gameView = {
	center  = nil,
	player  = nil,
	shouldRefresh = false,

	leftSidebar = Sidebar.New("##SidebarL", "left"),
	rightSidebar = Sidebar.New("##SidebarR", "right"),

	modulesDirty = false,
	modules = {},
	hudModules = {},
	sidebarModules = {},
}

function gameView.debugReload()
	package.reimport('pigui.libs.sidebar')
	Sidebar = require 'pigui.libs.sidebar'

	gameView.leftSidebar = Sidebar.New("##SidebarL", "left")
	gameView.rightSidebar = Sidebar.New("##SidebarR", "right")
end

local function onRegisterSidebar(name, module, idx)
	module.side = module.side or "left"
	module.priority = module.priority or idx
	gameView.modulesDirty = true
end

-- Register a general world-view module that should always be visible
gameView.registerModule = vutil.mixin_modules(gameView.modules)

-- Register a module to be displayed in the left or right "hud" areas under the sidebar
gameView.registerHudModule = vutil.mixin_modules(gameView.hudModules, onRegisterSidebar)

-- Register a module to be displayed in the left or right sidebar panes
gameView.registerSidebarModule = vutil.mixin_modules(gameView.sidebarModules, onRegisterSidebar)

-- Assign hud and sidebar modules to the appropriate sidebar
function gameView:updateModules()
	if not self.modulesDirty then return end
	self.modulesDirty = false

	local assignModules = function(sidebar)
		local priority = function (a, b) return a.priority < b.priority end
		local filter = function(v) return v.side == sidebar.side end
		sidebar.modules = utils.filter_array(self.sidebarModules, filter)
		sidebar.hudModules = utils.filter_array(self.hudModules, filter)

		table.sort(sidebar.modules, priority)
		table.sort(sidebar.hudModules, priority)
	end

	assignModules(self.leftSidebar)
	assignModules(self.rightSidebar)
end

local getBodyIcon = require 'pigui.modules.flight-ui.body-icons'

local function setTarget(body)
	if body:IsShip() or body:IsMissile() then
		gameView.player:SetCombatTarget(body)
	else
		gameView.player:SetNavTarget(body)
		ui.playSfx("OK")
	end
end

local function displayOnScreenObjects()
	local player = gameView.player

	local navTarget = player:GetNavTarget()
	local combatTarget = player:GetCombatTarget()

	local should_show_label = ui.shouldShowLabels()
	local iconsize = Vector2(20, 20)
	local small_iconsize = Vector2(18,18)
	local label_offset = 14 -- enough so that the target rectangle fits
	local cluster_size = iconsize.x -- size of clusters to be collapsed into single bodies
	local click_radius = cluster_size * 0.5
	-- make click_radius sufficiently smaller than the cluster size
	-- to prevent overlap of selection regions

	local bodies_grouped = ui.getProjectedBodiesGrouped(cluster_size, IN_SPACE_INDICATOR_SHIP_MAX_DISTANCE)

	for _,group in ipairs(bodies_grouped) do
		local mainBody = group.mainBody
		local mainCoords = group.screenCoordinates

		ui.addIcon(mainCoords, getBodyIcon(mainBody, true), colors.frame, iconsize, ui.anchor.center, ui.anchor.center)

		if should_show_label then
			local label = mainBody:GetLabel()
			if group.multiple then
				label = label .. " (" .. #group.bodies .. ")"
			end
			ui.addStyledText(mainCoords + Vector2(label_offset,0), ui.anchor.left, ui.anchor.center, label , colors.frame, pionillium.small)
		end
		local mp = ui.getMousePos()
		-- mouse release handler for radial menu
		if (mp - mainCoords):length() < click_radius then
			if ui.canClickOnScreenObjectHere() and ui.isMouseClicked(1) then
				local body = mainBody
				ui.openDefaultRadialMenu("game", body)
			end
		end
		-- mouse release handler
		if (mp - mainCoords):length() < click_radius then
			if ui.canClickOnScreenObjectHere() and ui.isMouseReleased(0) then
				if group.hasNavTarget or combatTarget == mainBody then
					-- if clicked and is target, unset target
					if group.hasNavTarget then
						player:SetNavTarget(nil)
					else
						player:SetCombatTarget(nil)
					end
				elseif not group.multiple then
					-- clicked on single, just set navtarget/combatTarget
					setTarget(mainBody)
					if ui.ctrlHeld() then
						-- also set follow target on ctrl-click
						player:SetFollowTarget(mainBody)
					end
				else
					-- clicked on group, show popup
					ui.openPopup("navtarget" .. mainBody:GetLabel())
				end
			end
		end
		-- popup content
		ui.popup("navtarget" .. mainBody:GetLabel(), function()
			for _,b in pairs(group.bodies) do
				ui.icon(getBodyIcon(b, true), small_iconsize, colors.frame)
				ui.sameLine()
				ui.alignTextToLineHeight(small_iconsize.y)
				if ui.selectable(b:GetLabel(), b == navTarget, {}) then
					if b:IsShip() then
						player:SetCombatTarget(b)
					else
						player:SetNavTarget(b)
						ui.playSfx("OK")
					end
					if ui.ctrlHeld() then
						-- also set follow-target target on ctrl-click
						player:SetFollowTarget(b)
					end
				end
			end
		end)
	end
end

gameView.registerModule("onscreen-objects", {
	showInHyperspace = false,
	draw = function(self, dT)
		displayOnScreenObjects()
	end
})

function gameView:draw()
	if self.shouldRefresh then
		self.leftSidebar:Refresh()
		self.rightSidebar:Refresh()

		self.shouldRefresh = false
	end

	self:updateModules()

	for i, module in ipairs(gameView.modules) do
		local shouldDraw = not Game.InHyperspace() or module.showInHyperspace
		if (not module.disabled) and shouldDraw then
			local ok, err = ui.pcall(module.draw, module, Engine.frameTime)
			if not ok then
				module.disabled = true
				-- TODO: visually notify the user of an error
			end
		end
	end

	self.leftSidebar:Draw()

	self.rightSidebar:Draw()
end

local function displayScreenshotInfo()
	if not Engine.GetDisableScreenshotInfo() then
		local current_system = Game.system
		if current_system then
			local frame = Game.player.frameBody
			if frame then
				local info = frame.label .. ", " .. ui.Format.SystemPath(current_system.path)
				ui.addStyledText(Vector2(20, 20), ui.anchor.left, ui.anchor.top, info , colors.white, pionillium.large)
			end
		end
	end
end

local function callModules(mode)
	for k,v in ipairs(ui.getModules(mode)) do
		if not v.disabled then
			v.disabled = not ui.pcall(v.draw)
		end
	end
end

local drawHUD = ui.makeFullScreenHandler("HUD", function()
	if ui.shouldDrawUI() then
		if Game.CurrentView() == "world" then
			gameView:draw()
		else
			gameView.shouldRefresh = true
		end

		ui.radialMenu("game")
		callModules("game")
	elseif Game.CurrentView() == "world" then
		displayScreenshotInfo()
	end
end)

local debugReload = function(t)
	for i, v in ipairs(t) do
		if type(v) == 'table' and v.debugReload then
			v.debugReload()
		end
	end
end

Event.Register("onGameStart", function()
	gameView.shouldRefresh = true
	gameView.leftSidebar:Reset()
	gameView.rightSidebar:Reset()
end)

Event.Register("onPauseMenuOpen", function()
	lastTimeAcceleration = Game.GetTimeAcceleration() ~= Game.GetRequestedTimeAcceleration() and Game.GetRequestedTimeAcceleration() or Game.GetTimeAcceleration()
	Game.SetTimeAcceleration("paused")
end)

Event.Register("onPauseMenuClosed", function()
	Game.SetTimeAcceleration((lastTimeAcceleration == "paused") and "1x" or lastTimeAcceleration)
	Input.EnableBindings()
end)

ui.registerHandler('game', function(delta_t)
		-- delta_t is ignored for now
		gameView.player = Game.player
		gameView.center = Vector2(ui.screenWidth / 2, ui.screenHeight / 2)

		-- TODO: add a handler mechanism for theme changes
		-- colors = ui.theme.colors -- if the theme changes
		-- icons = ui.theme.icons -- if the theme changes
		-- keep a copy of the current view so modules can react to the escape key and change the view
		-- without triggering the options dialog
		local currentView = Game.CurrentView()

		-- Ensure we're wrapping the whole UI in a font that scales with the rest of the game
		ui.withFont(pionillium.medium, function()
			ui.withStyleColors({ WindowBg = colors.transparent }, drawHUD)
		end)

		-- TODO: dispatch escape key to views and let them handle it
		if currentView == "world" and ui.escapeKeyReleased(true) then
			ui.optionsWindow:changeState()
		end

		callModules('modal')
		callModules('ui-timer')

		if ui.ctrlHeld() and ui.isKeyReleased(ui.keys.delete) then
			gameView.debugReload()

			debugReload(ui.getModules("game"))
			debugReload(gameView.modules)
			debugReload(gameView.hudModules)
			debugReload(gameView.sidebarModules)
		end
end)

return gameView
