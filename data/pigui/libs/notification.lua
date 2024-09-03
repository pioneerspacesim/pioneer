-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine  = require 'Engine'
local pigui   = Engine.pigui
local Vector2 = _G.Vector2

local ui = require 'pigui.baseui'

local lui = require 'Lang'.GetResource("ui-core")

local colors = ui.theme.colors
local icons  = ui.theme.icons

local titleFont   = ui.fonts.pionillium.body
local detailsFont = ui.fonts.pionillium.details

local style = ui.rescaleUI {
	padding = Vector2(20, 20),
	innerPadding = Vector2(20, 12),
	innerSpacing = Vector2(4, 4),
	closeButtonSize = Vector2(24, 24),
	rounding = 8,
	barWidth = 6,
	border = 2,
}

--=============================================================================-

local Notification = {}

Notification.dismissAnimLen = 0.7
Notification.expiryTime = 7
Notification.expiryTimeShort = 3
Notification.queue = {}

Notification.Type = {
	Info = "INFO", 		-- General informational notification
	Game = "GAME", 		-- For an in-game event that triggered a notification
	Error = "ERROR",	-- A section of code has encountered an error
}

local notificationColor = {
	[Notification.Type.Info] 	= colors.notificationInfo,
	[Notification.Type.Game] 	= colors.notificationGame,
	[Notification.Type.Error] 	= colors.notificationError
}

-- Function: add
--
-- Add a new notification to be displayed on screen
--
---@param type string The type of notification, one of Notification.Type
---@param title string The title text of the notification. Can wrap if needed.
---@param body string? Additional text explaining the notification. Optional.
---@param icon any? The icon to display on the notification. Optional.
---@param noAutoExpire boolean? If true, the notification must be manually dismissed.
function Notification.add(type, title, body, icon, noAutoExpire)
	local expiryTime = body and Notification.expiryTime or Notification.expiryTimeShort

	table.insert(Notification.queue, {
		expiry = not noAutoExpire and ui.getTime() + expiryTime,
		title = title or "",
		body = body,
		icon = icon,
		type = type
	})
end

-- Function: dismissAll
--
-- Dismiss all current notifications. If the force parameter is passed, resets
-- the notification queue.
function Notification.dismissAll(force)
	if force then
		Notification.queue = {}
	else
		for _, notif in ipairs(Notification.queue) do
			notif.closing = notif.closing or 0.0
		end
	end
end

--==============================================================================

-- Pre-compute the size of a given notification and store it for later
local function calcNotificationSize(notif, wrapWidth)
	local contentSize = Vector2(0, 0)
	local spacing = style.innerSpacing

	-- Compute total size of notification widget
	if notif.icon then
		contentSize.x = titleFont.size + spacing.x
	end

	local titleSize = ui.calcTextSize(notif.title, titleFont, wrapWidth - contentSize.x)

	contentSize.x = contentSize.x + titleSize.x
	contentSize.y = titleSize.y

	-- If we have body notification text, just make this a max-width notification
	if notif.body then
		local textSize = ui.calcTextSize(notif.body or "", detailsFont, wrapWidth)

		contentSize.x = wrapWidth
		contentSize.y = contentSize.y + spacing.y + textSize.y
	end

	local badgeSize = contentSize + style.innerPadding * 2

	notif.size = badgeSize
	notif.titleHeight = titleSize.y
end

-- Draw a notification card
-- Expects the size and titleHeight variables to have been set in the notification from
local function drawNotification(notif, wrapWidth)
	local badgeSize = notif.size
	local spacing = style.innerSpacing

	local startPos = ui.getCursorScreenPos()
	local buttonArea = Vector2(style.closeButtonSize.x, 0)
	local pos = startPos + buttonArea

	ui.beginGroup()

	-- Handle interaction with the notification widget
	ui.dummy(badgeSize + buttonArea)
	local hovered = ui.isItemHovered({ "AllowWhenOverlappedByItem", "AllowWhenBlockedByActiveItem" })

	local round = style.rounding
	local barCol = notificationColor[notif.type] or colors.notificationInfo

	-- Draw background
	ui.addRectFilled(pos, pos + badgeSize, colors.notificationBackground, round, 0xF)
	-- Draw border
	ui.addRect(pos, pos + badgeSize, colors.windowFrame, round, 0xF, style.border)
	-- Draw left bar
	ui.addRectFilled(pos, pos + Vector2(style.barWidth, badgeSize.y), barCol, round, 0x5)

	pos = pos + style.innerPadding

	-- Draw the leading icon for the title line
	if notif.icon then
		ui.addIconSimple(pos, notif.icon, Vector2(titleFont.size), colors.font)
	end

	-- Draw the title line, optionally offset by the icon
	ui.withFont(titleFont, function()
		local titlePos = notif.icon and (pos + Vector2(titleFont.size + spacing.x, 0)) or pos
		ui.addText(titlePos, colors.font, notif.title, wrapWidth)
	end)

	-- Draw the notification body text
	if notif.body then
		ui.withFont(detailsFont, function()
			local bodyPos = pos + Vector2(0, notif.titleHeight + spacing.y)
			ui.addText(bodyPos, colors.fontDim, notif.body, wrapWidth)
		end)
	end

	-- Draw the close button to the left of the notification in empty space
	if hovered and not notif.closing then
		ui.setCursorScreenPos(startPos)
		local clicked = ui.iconButton(icons.retrograde, style.closeButtonSize, "##DismissNotification", ui.theme.buttonColors.transparent)

		if clicked then
			notif.closing = 0.0
		end
	end

	ui.endGroup()

	return hovered
end

--==============================================================================

local windowFlags = ui.WindowFlags { "NoDecoration", "NoBackground", "NoMove" }

ui.registerModule('notification', function()
	if #Notification.queue == 0 then
		return
	end

	local maxWidth = ui.screenWidth / 4.0
	local maxHeight = 0.0

	local badgeWidth = maxWidth - style.padding.x - style.closeButtonSize.x
	local wrapWidth = badgeWidth - style.innerPadding.x * 2
	local vSpacing = style.innerPadding.y

	-- Compute total vertical height of the notifications on-screen
	-- We also remove expired notifications here
	for i = #Notification.queue, 1, -1 do
		local notif = Notification.queue[i]

		-- Start the close animation
		if notif.expiry and notif.expiry < ui.getTime() and not notif.closing then
			notif.closing = 0.0
		end

		-- Remove finished notifications
		if notif.closing and notif.closing >= 1.0 then
			table.remove(Notification.queue, i)
		else
			-- TODO(screen-resize): this has to be re-calculated if the screen width changes
			if not notif.size then
				calcNotificationSize(notif, wrapWidth)
			end

			maxHeight = maxHeight + notif.size.y + (i == 1 and 0.0 or vSpacing)
		end
	end

	-- Grow vertically, but fix horizontal size
	local windowHeight = math.min(maxHeight, ui.screenHeight)
	ui.setNextWindowSize(Vector2(maxWidth, windowHeight), "Always")
	ui.setNextWindowPos(ui.screenSize() - Vector2(maxWidth, windowHeight + style.padding.y), "Always")

	ui.withStyleVars({ WindowPadding = Vector2(0, 0) }, function()
		pigui.Begin("##notifications", windowFlags)
	end)

	-- Ensure the notification stack renders on top of everything
	-- Notifications are rendered inside of the modal window begin()/end() stack to remain interactable
	pigui.BringWindowToDisplayFront()

	-- Draw notifications bottom-up from newest to oldest
	local currentHeight = windowHeight

	for i = #Notification.queue, 1, -1 do
		local notif = Notification.queue[i]

		-- Increase animation progress
		if notif.closing then
			notif.closing = notif.closing + Engine.frameTime / Notification.dismissAnimLen
		end

		local offset = badgeWidth - notif.size.x + maxWidth * (notif.closing or 0.0)

		ui.setCursorPos(Vector2(offset, currentHeight - notif.size.y))
		local hovered = drawNotification(notif, wrapWidth)

		-- Prevent this notification from expiring while hovered
		if hovered then
			notif.expiry = notif.expiry + Engine.frameTime
		end

		currentHeight = currentHeight - notif.size.y - vSpacing
	end

	pigui.End()
end)

return Notification
