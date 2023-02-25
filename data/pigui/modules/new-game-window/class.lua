-- Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Lang = require 'Lang'
local ui = require 'pigui'
local lui = Lang.GetResource("ui-core")
local msgbox = require 'pigui.libs.message-box'
local ModalWindow = require 'pigui.libs.modal-win'
local ModelSkin = require 'SceneGraph.ModelSkin'
local ShipDef = require "ShipDef"
local SystemPath = require 'SystemPath'

local misc = Equipment.misc
local laser = Equipment.laser

local Defs = require 'pigui.modules.new-game-window.defs'
local Layout = require 'pigui.modules.new-game-window.layout'
local Crew = require 'pigui.modules.new-game-window.crew'
local StartVariants = require 'pigui.modules.new-game-window.start-variants'
local Game = require 'Game'

local profileCombo = { items = {}, selected = 0 }

StartVariants.register({
	name       = lui.START_AT_MARS,
	desc       = lui.START_AT_MARS_DESC,
	location   = SystemPath.New(0,0,0,0,18),
	logmsg     = lui.START_LOG_ENTRY_1,
	shipType   = 'coronatrix',
	money      = 600,
	hyperdrive = true,
	equipment  = {
		{ laser.pulsecannon_1mw,      1 },
		{ misc.atmospheric_shielding, 1 },
		{ misc.autopilot,             1 },
		{ misc.radar,                 1 }
	},
	cargo      = {
		{ Commodities.hydrogen, 2 }
	},
	pattern    = 1,
	colors     = { Color('000000'), Color('000000'), Color('000000') }
})

StartVariants.register({
	name       = lui.START_AT_NEW_HOPE,
	desc       = lui.START_AT_NEW_HOPE_DESC,
	location   = SystemPath.New(1,-1,-1,0,4),
	logmsg     = lui.START_LOG_ENTRY_2,
	shipType   = 'pumpkinseed',
	money      = 400,
	hyperdrive = true,
	equipment  = {
		{ laser.pulsecannon_1mw,      1 },
		{ misc.atmospheric_shielding, 1 },
		{ misc.autopilot,             1 },
		{ misc.radar,                 1 }
	},
	cargo      = {
		{ Commodities.hydrogen, 2 }
	},
	pattern    = 1,
	colors     = { Color('000000'), Color('000000'), Color('FFFF00') }
})

StartVariants.register({
	name       = lui.START_AT_BARNARDS_STAR,
	desc           = lui.START_AT_BARNARDS_STAR_DESC,
	location       = SystemPath.New(-1,0,0,0,16),
	logmsg         = lui.START_LOG_ENTRY_3,
	shipType       = 'xylophis',
	money          = 100,
	hyperdrive     = false,
	equipment      = {
		{misc.atmospheric_shielding,1},
		{misc.autopilot,1},
		{misc.radar,1}
	},
	cargo          = {
		{ Commodities.hydrogen, 2 }
	},
	pattern    = 6,
	colors     = { Color('E17F00'), Color('FFFFFF'), Color('FF7F00') }
})

-- pass avalable space rect
local function updateLayout(contentRegion)
	Layout.updateLayout(contentRegion)

	for _, tab in ipairs(Layout.Tabs) do
		tab:updateLayout()
	end
end

local function setStartVariant(variant)
	for _, param in ipairs(Layout.UpdateOrder) do
		param:fromStartVariant(variant)
	end
	Defs.currentStartVariant = variant
end

local NewGameWindow = {}

local function putByPath(tbl, path, value)
	local ref, last
	for token in string.gmatch(path, "[^.]+") do
		if not tbl[token] then tbl[token] = {} end
		ref = tbl
		last = token
		tbl = tbl[token]
	end
	ref[last] = value
end

local function drawBottomButtons()
	ui.separator()
	if ui.button(lui.START_GAME) then
		local errors = {}
		local msg = ""
		for _, param in ipairs(Layout.UpdateOrder) do
			if not param:isValid() then
				table.insert(errors, param.name)
			end
		end
		if #errors == 0 then
			local gameParams = {}
			for _, param in ipairs(Layout.UpdateOrder) do
				putByPath(gameParams, param.path, param.value)
			end
			-- start game here from gameParams
		else
			msg = lui.SOME_PARAMETERS_ARE_NOT_OK
			for _, v in ipairs(errors) do
				msg = msg .. '\n - ' .. tostring(v)
			end
			msgbox.OK(msg)
		end
	end
	ui.sameLine()
	if ui.button(lui.CLOSE) then
		NewGameWindow:close()
	end
end

-- wait a few frames, and then calculate the static layout (updateLayout)
local initFrames = 3

NewGameWindow = ModalWindow.New("New Game", function()

	ui.withFont(Defs.mainFont, function()

		ui.alignTextToFramePadding()
		ui.text(lui.SELECT_START_PROFILE)

		ui.sameLine()
		local changed, ret = ui.combo("##starttype", profileCombo.selected, profileCombo.items)
		if changed then
			profileCombo.selected = ret
			if profileCombo.actions[ret + 1] == 'DO_UNLOCK' then
				Layout.setLock(false)
				Crew.Player.Log.value = "Custom start of the game - for the purpose of debugging or cheat."
			else
				setStartVariant(StartVariants.item(ret + 1))
			end
			-- since setStartVariant can be called outside the ImGui frame, we
			-- can't update the interface inside it. But we do not want to
			-- check for data changes every frame, so we explicitly call the
			-- update of the interface, while the amount of free space (content
			-- region) is already known and does not change
			updateLayout(Defs.contentRegion)
		end

		ui.separator()

		if ui.beginTabBar("Tabs") then
			if initFrames > 0 then
				if initFrames == 1 then
					-- consider the height of bottom buttons
					drawBottomButtons()
					-- at this point getContentRegion() exactly returns the size of the available space
					updateLayout(ui.getContentRegion())
				end
				initFrames = initFrames - 1
			else
				for _, tab in ipairs(Layout.Tabs) do
					if ui.beginTabItem(tab.TabName) then
						tab:draw()
						ui.endTabItem()
					end
				end
			end
			ui.endTabBar()
		end

		drawBottomButtons()
	end)
end, function (_, drawPopupFn)
	ui.setNextWindowSize(Defs.winSize, 'Always')
	ui.setNextWindowPosCenter('Always')
	ui.withStyleColors({ PopupBg = ui.theme.colors.modalBackground }, drawPopupFn)
end)

function NewGameWindow:buildProfileCombo()
	profileCombo.items = {}
	profileCombo.actions = {}
	for _, var in ipairs(StartVariants.list()) do
		table.insert(profileCombo.items, var.name)
	end
	if self.debugMode then
		table.insert(profileCombo.items, "Custom start")
		profileCombo.actions[#profileCombo.items] = 'DO_UNLOCK'
	end
end

function NewGameWindow:setDebugMode(value)
	self.debugMode = value
	self:buildProfileCombo()
end

NewGameWindow:setDebugMode(false)

setStartVariant(StartVariants.item(1))

return NewGameWindow
