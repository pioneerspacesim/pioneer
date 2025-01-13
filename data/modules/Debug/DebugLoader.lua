-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Event = require 'Event'
local utils = require 'utils'

local ui = require 'pigui'

local colors = ui.theme.colors
local icons = ui.theme.icons

local Notification = require 'pigui.libs.notification'
local debugView    = require 'pigui.views.debug'

local messageData = {
	log = {},
	bySource = utils.automagic()
}

local activeLoader = nil
local messageCount = {}

--=============================================================================

local DebugLoader = {}

DebugLoader.Type = {
	Error = "ERROR",
	Warn = "WARN",
	Info = "INFO"
}

DebugLoader.checks = {}

---@param name string
---@param func fun()
function DebugLoader.RegisterCheck(name, func)
	table.insert(DebugLoader.checks, {
		id = name,
		run = func
	})
end

---@param type string One of DebugLoader.Type
---@param message string
function DebugLoader.LogMessage(type, message)
	table.insert(messageData.log, { type, message, activeLoader })
	messageCount[type] = (messageCount[type] or 0) + 1

	print("validation {}: {}" % { type, message })
end

---@param type string One of DebugLoader.Type
---@param message string
function DebugLoader.LogFileMessage(type, source, message)
	table.insert(messageData.bySource[source], { type, message, activeLoader })
	table.insert(messageData.log, { type, message, source })
	messageCount[type] = (messageCount[type] or 0) + 1

	print("validation {} [{}]: {}" % { type, source, message })
end

--=============================================================================

local function scanForErrors()
	for _, check in ipairs(DebugLoader.checks) do
		activeLoader = check.id
		check.run()
	end
end

--=============================================================================

local DebugLoaderUI = utils.class("Debug.LoaderUI", require 'pigui.libs.module')

local msg_order = {
	DebugLoader.Type.Error,
	DebugLoader.Type.Warn,
	DebugLoader.Type.Info,
}

local msg_icons = {
	[DebugLoader.Type.Error] = icons.alert_generic,
	[DebugLoader.Type.Warn] = icons.view_internal,
	[DebugLoader.Type.Info] = icons.info
}

local msg_colors = {
	[DebugLoader.Type.Error] = ui.theme.styleColors.danger_300,
	[DebugLoader.Type.Warn] = ui.theme.styleColors.warning_300,
	[DebugLoader.Type.Info] = colors.font
}

function DebugLoaderUI:Constructor()
	self.Super().Constructor(self)

	self.activeSource = nil
end

function DebugLoaderUI:setActiveSource(source)
	self.activeSource = source
end

function DebugLoaderUI:render()
	ui.horizontalGroup(function()
		for _, type in ipairs(msg_order) do
			ui.icon(msg_icons[type], Vector2(ui.getTextLineHeight()), msg_colors[type])
			ui.text(ui.Format.Number(messageCount[type] or 0, 0))
		end
	end)

	local function getSourceName(source)
		return "{} ({})" % { source or "All", ui.Format.Number((source and #messageData.bySource[source] or #messageData.log), 0) }
	end

	local sourceList = utils.build_array(utils.keys(messageData.bySource))
	table.sort(sourceList)

	ui.comboBox("Source Filter", getSourceName(self.activeSource), function()
		if ui.selectable(getSourceName(nil)) then
			self:message("setActiveSource", nil)
		end

		for _, source in ipairs(sourceList) do
			if ui.selectable(getSourceName(source)) then
				self:message("setActiveSource", source)
			end
		end
	end)

	ui.separator()

	ui.child("MessageView", function()

		ui.pushTextWrapPos(ui.getContentRegion().x)

		local source = self.activeSource and messageData.bySource[self.activeSource] or messageData.log

		for _, log in ipairs(source) do
			local type, msg, src = log[1], log[2], log[3]
			ui.textColored(msg_colors[type], "[{}] {} {}" % { src, ui.get_icon_glyph(msg_icons[type]), msg })
		end

		ui.popTextWrapPos()

	end)
end

--=============================================================================

Event.Register("onEnterMainMenu", function()
	-- Reset messages on menu load so they don't accumulate after each new game
	messageData = {
		log = {},
		bySource = utils.automagic()
	}

	messageCount = {}

	scanForErrors()

	local message_body = "{} errors, {} warnings generated. See the debug Loading Messages tab (Ctrl+I) for more information."

	local numErrors = messageCount[DebugLoader.Type.Error] or 0
	local numWarnings = messageCount[DebugLoader.Type.Warn] or 0

	if numErrors > 0 or numWarnings > 0 then
		Notification.add(Notification.Type.Error, "Validation Issues Found",
			message_body % { numErrors, numWarnings },
			icons.repairs)
	end
end)

debugView.registerTab("Loader", {
	label = "Loading Messages",
	icon = icons.repairs,
	debugUI = DebugLoaderUI.New(),
	show = function() return utils.count(messageCount) > 0 end,
	draw = function(self)
		self.debugUI:update()
		self.debugUI:render()
	end
})

return DebugLoader
