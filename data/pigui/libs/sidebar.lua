-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local utils = require 'utils'
local Game  = require 'Game'
local Vector2 = _G.Vector2

local ui = require 'pigui'

local mainButtonSize = ui.theme.styles.MainButtonSize
local mainButtonPadding = ui.theme.styles.MainButtonPadding
local buttonColors = ui.theme.buttonColors

local styles = ui.theme.styles
local colors = ui.theme.colors
local pionillum = ui.fonts.pionillium

local windowFlags = ui.WindowFlags {"NoTitleBar", "NoResize", "NoSavedSettings", "NoFocusOnAppearing", "NoBringToFrontOnFocus", "NoScrollbar"}
local displayFlags = ui.WindowFlags {"NoTitleBar", "NoResize", "AlwaysAutoResize", "NoSavedSettings", "NoFocusOnAppearing", "NoBringToFrontOnFocus", "NoScrollbar"}

local animUp = 6
local animDown = -8

local function animateAlpha(v, opening)
	v = v + Engine.frameTime * (opening and animUp or animDown)
	return math.clamp(v, 0, 1)
end

--
-- Class: UI.Sidebar
--
-- The sidebar is responsible for drawing vertical areas on the left and right
-- side of the screen with toggleable information displays.
--
-- HUD modules are drawn when the sidebar is inactive or collapsed, while
-- regular modules are drawn if they are selected for display by the user.
--

---@class UI.Sidebar
---@field New fun(id, side?, offset?): UI.Sidebar
local Sidebar = utils.class("UI.Sidebar")

function Sidebar:Constructor(id, side, offset)
	self.modules = {}
	self.hudModules = {}

	self.id = id
	self.side = side or "left"
	self.offset = offset or Vector2(0, 0)
	self.pivot = Vector2(0, 0)
	self.active = nil
	self.nextActive = nil
	self.bgColor = colors.overlayWindowBg

	self.buttonPos = Vector2(0, 0)
	self.buttonSize = Vector2(0, 0)

	self.displayPos = Vector2(0, 0)
	self.displaySize = Vector2(0, 0)
	self.displayAlpha = 0.0
	self.hudAlpha = 0.0
end

function Sidebar:SafeCall(module, fn, ...)
	if module.disabled or not fn then return end
	module.disabled = not ui.pcall(fn, module, ...)
end

-- Update positions and sizes for the sidebar windows
function Sidebar:UpdateCoords()
	self.buttonPos.x = self.side == "right" and (ui.screenWidth - self.offset.x) or self.offset.x
	self.buttonPos.y = self.offset.y
	self.pivot.x = self.side == "right" and 1 or 0

	local buttonSize = mainButtonSize.x + mainButtonPadding * 2
	local spacing = ui.getItemSpacing()
	local padding = ui.getWindowPadding()

	local buttonWidth = #self.modules * (buttonSize + spacing.x) - spacing.x

	self.buttonSize = Vector2(buttonWidth, buttonSize) + padding * 2

	self.displaySize.x = ui.screenWidth / 4.5
	self.displayPos.x = self.side == "right" and
		(self.buttonPos.x - padding.x - self.displaySize.x) or
		(self.buttonPos.x + padding.x)

	local reservedSpace = (ui.timeWindowSize or Vector2(0, 0))
	self.displayPos.y = self.buttonPos.y + self.buttonSize.y + spacing.y
	self.displaySize.y = ui.screenHeight - self.displayPos.y - reservedSpace.y - padding.y * 2
end

-- Draw the list of sidebar buttons for active modules
function Sidebar:DrawButtons()
	ui.setNextWindowPos(self.buttonPos, "Always", self.pivot)
	ui.setNextWindowSize(self.buttonSize, "Always")
	ui.window(self.id .. "@Buttons", windowFlags, function()
		for i, v in ipairs(self.modules) do
			if ui.mainMenuButton(v.icon, v.tooltip, v.active) then
				local exclusive = self.active and self.active.exclusive or v.exclusive
				if v ~= self.active and exclusive then
					self.nextActive = v
				else
					v.closing = v.active
					v.active = true

					if not v.closing then self:SafeCall(v, v.refresh) end
				end
			end

			ui.sameLine()
		end
	end)
end

-- Draw inactive / HUD modules when the sidebar is closed
function Sidebar:DrawHudModules()
	-- Make a copy of the size/pos vectors to allow clipping
	local min = Vector2(self.displayPos.x, self.displayPos.y)
	local max = min + self.displaySize

	-- the { min, max } rect should be clipped by modules to subtract the space
	-- they intend to render into
	ui.withStyleVars({ Alpha = self.hudAlpha }, function()
		for i, v in ipairs(self.hudModules) do
			if not Game.InHyperspace() or v.showInHyperspace then
				v:draw(min, max)
			end
		end
	end)
end

-- Draw active display modules
function Sidebar:DrawModule(module)
	local screenPos = ui.getCursorScreenPos()
	local font = pionillum.heading

	local frameSize = font.size + styles.ButtonPadding.y * 2
	local iconSize = frameSize - mainButtonPadding * 2

	local iconBg = buttonColors.selected.normal
	local titleBg = buttonColors.default.normal

	-- Draw backgrounds for title area and icon
	ui.addRectFilled(screenPos, screenPos + Vector2(ui.getContentRegion().x, frameSize), titleBg, 0, 0)
	ui.addRectFilled(screenPos, screenPos + Vector2(frameSize), iconBg, 0, 0)

	local iconPos = screenPos + Vector2(mainButtonPadding)
	ui.addIconSimple(iconPos, module.icon, Vector2(iconSize), colors.white, module.tooltip)

	ui.withFont(font, function()
		ui.dummy(Vector2(frameSize))
		ui.sameLine()
		ui.alignTextToLineHeight()

		if module.drawTitle then
			self:SafeCall(module, module.drawTitle)
		else
			ui.text(module.title or "")
		end
	end)

	ui.newLine()
	ui.setCursorScreenPos(screenPos + Vector2(0, frameSize + styles.WindowPadding.y))

	if module.drawBody then
		ui.addWindowPadding(styles.WindowPadding)

		ui.withFont(pionillum.body, function()
			self:SafeCall(module, module.drawBody)
		end)

		ui.addWindowPadding(-styles.WindowPadding)
	end
end

function Sidebar:Draw()
	local activeModules = utils.filter_array(self.modules, function(v) return v.active and not v.disabled end)

	self:UpdateCoords()

	self:DrawButtons()

	-- If we're changing to or from an exclusive module, close all open modules
	if self.nextActive then
		for i, v in ipairs(activeModules) do v.closing = true end
	end

	local closingModules = utils.filter_array(activeModules, function (v) return v.closing end)

	-- Handle displaying the next module to be visible (when swapping between exclusive modules)
	if self.nextActive and #closingModules == 0 then
		local module = self.nextActive

		module.active = true
		table.insert(activeModules, module)
		self:SafeCall(module, module.refresh)

		if module.exclusive then
			self.active = module
		end

		self.nextActive = nil
	end

	local isActive = #activeModules > 0
	local shouldShow = isActive and #closingModules < #activeModules or self.nextActive
	self.displayAlpha = animateAlpha(self.displayAlpha, shouldShow)

	if isActive or self.nextActive then
		self.hudAlpha = 0.0
	else
		self.hudAlpha = animateAlpha(self.hudAlpha, true)
		self:DrawHudModules()

		return
	end

	-- Display a dividing line when the sidebar is open
	local linePos = self.displayPos - Vector2(0, ui.getWindowPadding().y)
	ui.addLine(linePos, linePos + Vector2(self.displaySize.x, 0), colors.grey:opacity(self.displayAlpha), 1)

	local nextSize = ui.getWindowContentSize(self.id)
	if not self.active then
		-- we add the padding in late, so we only need a single "side" of it
		self.displaySize.y = math.min(nextSize.y + styles.WindowPadding.y, self.displaySize.y)
	end

	ui.setNextWindowPos(self.displayPos, "Always")
	ui.setNextWindowSize(self.displaySize, "Always")

	-- Animate the window's fade in/out
	local a = math.max(self.displayAlpha, 0.001) -- if Alpha == 0, window logic does not run
	ui.withStyleColorsAndVars({ WindowBg = self.bgColor }, { Alpha = a }, function()

		-- Draw the sidebar contents
		ui.setNextWindowPadding(Vector2(0, 0))
		ui.window(self.id, displayFlags, function()

			for i, v in ipairs(activeModules) do
				v.alpha = animateAlpha(v.alpha or 0.0, not v.closing)

				ui.withStyleVars({ Alpha = v.alpha }, function()
					ui.withID(v.id or ("##" .. i), function()
						self:DrawModule(v)
					end)
				end)

				if v.alpha == 0.0 then
					v.active = false
					v.closing = false
					v.alpha = nil

					if v == self.active then
						self.active = nil
					end
				end
			end

		end)

	end)

end

-- Trigger the refresh event for all active sidebar modules
-- (expected to be triggered when the entire sidebar becomes visible)
function Sidebar:Refresh()
	local activeModules = utils.filter_array(self.modules, function(v) return v.active and not v.disabled end)

	for i, v in ipairs(activeModules) do
		self:SafeCall(v, v.refresh)
	end
end

-- Reset all modules to inactive and clear transient sidebar state.
-- This should be called at the start of a new game, etc.
function Sidebar:Reset()
	self.displayAlpha = 0.0
	self.hudAlpha = 0.0
	self.active = nil
	self.nextActive = nil

	local activeModules = utils.filter_array(self.modules, function(v) return v.active and not v.disabled end)
	for i, v in ipairs(activeModules) do
		v.active = false
		v.closing = false
		v.alpha = nil
	end
end

return Sidebar
