-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Lang = require 'Lang'
local ui = require 'pigui'
local lui = Lang.GetResource("ui-core")
local msgbox = require 'pigui.libs.message-box'
local Character = require 'Character'
local Commodities = require 'Commodities'
local Equipment = require 'Equipment'
local FlightLog = require 'modules.FlightLog.FlightLog'
local ModalWindow = require 'pigui.libs.modal-win'
local ModelSkin = require 'SceneGraph.ModelSkin'
local ShipDef = require "ShipDef"
local SystemPath = require 'SystemPath'

local Defs = require 'pigui.modules.new-game-window.defs'
local Layout = require 'pigui.modules.new-game-window.layout'
local Recovery = require 'pigui.modules.new-game-window.recovery'
local StartVariants = require 'pigui.modules.new-game-window.start-variants'
local FlightLogParam = require 'pigui.modules.new-game-window.flight-log'
local Game = require 'Game'

local profileCombo = { items = {}, selected = 0 }

StartVariants.register({
	name       = lui.START_AT_MARS,
	desc       = lui.START_AT_MARS_DESC,
	location   = SystemPath.New(0,0,0,0,18),
	logmsg     = lui.START_LOG_ENTRY_1,
	shipType   = 'coronatrix',
	money      = 600,
	equipment = {
		computer_1     = "misc.autopilot",
		laser_front_s2 = "laser.pulsecannon_1mw",
		shield_s1_1    = "shield.basic_s1",
		shield_s1_2    = "shield.basic_s1",
		sensor         = "sensor.radar",
		hull_mod       = "hull.atmospheric_shielding_s1",
		hyperdrive     = "hyperspace.hyperdrive_2",
		thruster       = "thruster.default_s1",
		missile_bay_1  = "missile_bay.opli_internal_s2",
		missile_bay_2  = "missile_bay.opli_internal_s2",
	},
	cargo      = {
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
	equipment = {
		computer_1     = "misc.autopilot",
		laser_front_s1 = "laser.pulsecannon_1mw",
		sensor         = "sensor.radar",
		hull_mod       = "hull.atmospheric_shielding_s1",
		hyperdrive     = "hyperspace.hyperdrive_1",
		thruster       = "thruster.default_s1",
	},
	cargo      = {
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
	equipment = {
		computer_1     = "misc.autopilot",
		sensor         = "sensor.radar",
		hull_mod       = "hull.atmospheric_shielding_s0",
		thruster       = "thruster.default_s1",
	},
	cargo          = {
	},
	pattern    = 6,
	colors     = { Color('E17F00'), Color('FFFFFF'), Color('FF7F00') }
})

-- synchronize parameter views with updated values
local function updateParams()
	for _, tab in ipairs(Layout.Tabs) do
		if tab.updateParams then tab:updateParams() end
	end
end

local function setStartVariant(variant)
	for _, param in ipairs(Layout.UpdateOrder) do
		param:fromStartVariant(variant)
	end
end

local function startGame(gameParams)

	-- space, ship in dock / orbit
	local startTime = gameParams.time
	if startTime < 0 then
		startTime = util.standardGameStartTime()
	end
	Game.StartGame(gameParams.location.path, startTime, gameParams.ship.type)
	local player = Game.player

	player:SetLabel(gameParams.ship.label)
	player:SetMoney(gameParams.player.money)
	player:SetShipName(gameParams.ship.name)

	local pattern = gameParams.ship.model.pattern
	if pattern > 0 then
		player:SetPattern(gameParams.ship.model.pattern - 1)
	end
	local colors = gameParams.ship.model.colors
	local shipDef = ShipDef[gameParams.ship.type]
	local skin = ModelSkin.New()
	skin:SetColors({ primary = colors[1], secondary = colors[2], trim = colors[3] })
	skin:SetDecal(shipDef.manufacturer)
	skin:SetLabel(gameParams.ship.label)
	player:SetSkin(skin)

	-- setup player character
	local PlayerCharacter = gameParams.player.char
	PlayerCharacter.reputation = gameParams.player.reputation
	PlayerCharacter.killcount = gameParams.player.kills
	-- Gave the player a missions table (for Misssions.lua)
	PlayerCharacter.missions = {}
	-- Insert the player character into the persistent character
	-- table.  Player won't be ennumerated with NPCs, because player
	-- is not numerically keyed.
	Character.persistent = { player = PlayerCharacter }
	-- Enroll the player in their own crew
	player:Enroll(PlayerCharacter)

	-- Generate crew for the starting ship
	for _, member in ipairs(gameParams.crew) do
		member.contract.payday = startTime + 604800 -- in a week
		member.contract.outstanding = 0
		player:Enroll(member)
	end

	local equipSet = player:GetComponent("EquipSet")
	player:UpdateEquipStats()

	-- slotless
	for _, item in ipairs(gameParams.ship.equipment) do
		local proto = Equipment.Get(item)
		if not equipSet:Install(proto:Instance()) then
			logWarning("Couldn't install equipment item {} in misc. cargo space" % { proto:GetName() })
		end
	end

	local function installEquipment(nodes, prefix)
		for slot, node in pairs(nodes) do

			local item

			if type(node) == 'table' then
				item = node.id
				assert(item)
			else
				item = node
			end

			if prefix then
				slot = prefix .. slot
			end

			local proto = Equipment.Get(item)
			if type(slot) == "string" then
				local slotHandle = equipSet:GetSlotHandle(slot)
				if slotHandle then
					local inst = proto:Instance()

					if inst.SpecializeForShip then inst:SpecializeForShip(equipSet.config) end

					if slotHandle.count then
						inst:SetCount(slotHandle.count)
					end

					if not equipSet:Install(inst, slotHandle) then
						logWarning("Couldn't install equipment item {} into slot {}" % { inst:GetName(), slot })
					end
				end

				if type(node) == 'table' then
					installEquipment(node.slots, slot .. "##")
				end
			end
		end
	end
	installEquipment(gameParams.ship.equipment)

	---@type CargoManager
	local cargoMgr = player:GetComponent('CargoManager')
	for id, amount in pairs(gameParams.ship.cargo) do
		cargoMgr:AddCommodity(Commodities[id], amount)
	end

	local drive = player:GetInstalledHyperdrive()
	if drive then
		drive:SetFuel(player, drive:GetMaxFuel())
	end

	for _, entry in ipairs(gameParams.flightlog.Custom) do
		if FlightLog.InsertCustomEntry(entry) then
			-- ok then
		elseif entry.entry then
			-- allow a custom log entry containing only text
			-- because when starting a new game we only have text
			FlightLog.MakeCustomEntry(entry.entry)
		else
			logWarning("Wrong entry for the custom flight log")
		end
	end

	for _, entry in ipairs(gameParams.flightlog.System) do
		if not FlightLog.InsertSystemEntry(entry) then
			logWarning("Wrong entry for the system flight log")
		end
	end

	for _, entry in ipairs(gameParams.flightlog.Station) do
		if not FlightLog.InsertStationEntry(entry) then
			logWarning("Wrong entry for the station flight log")
		end
	end
	FlightLog.OrganizeEntries()

	if gameParams.autoExec then
		gameParams.autoExec()
	end
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
			startGame(gameParams)
			NewGameWindow:close()
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

local function hasNameInArray(param, array)
	for _, v in pairs(array) do
		if v.name == param.name then return true end
	end
end

local function unlockAll()
	Layout.setLock(false)
	FlightLogParam.value.Custom = {{ entry = "Custom start of the game - for the purpose of debugging or cheat." }}
end

-- wait a few frames, and then calculate the static layout (updateLayout)
local initFrames = 2

NewGameWindow = ModalWindow.New("New Game", function()

	ui.withFont(Defs.mainFont, function()

		ui.alignTextToFramePadding()
		if NewGameWindow.mode == 'RECOVER' then
			ui.text(lui.RECOVER_SAVEGAME)
		else
			ui.text(lui.SELECT_START_PROFILE)
			ui.sameLine()
			local changed, ret = ui.combo("##starttype", profileCombo.selected, profileCombo.items)
			if changed then
				profileCombo.selected = ret
				local action = profileCombo.actions[ret + 1]
				if action == 'DO_UNLOCK' then
					unlockAll()
				else
					setStartVariant(StartVariants.item(ret + 1))
				end
				updateParams()
			end
		end

		ui.separator()

		if ui.beginTabBar("Tabs") then
			if initFrames > 0 then
				if initFrames == 1 then
					-- consider the height of bottom buttons
					drawBottomButtons()
					-- at this point getContentRegion() exactly returns the size of the available space
					Layout.updateLayout(ui.getContentRegion())
					updateParams()
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

function NewGameWindow.restoreSaveGame(selectedSave)
	local saveGame = Recovery.loadDoc(selectedSave)
	if type(saveGame) == 'table' then

		local paramErrors = {}

		local report = lui.ATTEMPTING_TO_RECOVER_GAME_PROGRESS_FROM_SAVE_X:interp{ save = selectedSave }
		-- first variant as default value
		setStartVariant(StartVariants.item(1))
		-- except for clearing the log
		FlightLogParam:fromStartVariant({})
		report = report .. "\n\n" .. string.format(lui.FILE_SAVEGAME_VERSION, tostring(saveGame.version))
		report = report .. "\n" .. string.format(lui.CURRENT_SAVEGAME_VERSION, tostring(Game.CurrentSaveVersion()))
		for _, param in ipairs(Layout.UpdateOrder) do
			local paramErrorString = param:fromSaveGame(saveGame)
			-- could not be restored - we will give the
			-- player the opportunity to edit it himself
			if paramErrorString then
				table.insert(paramErrors, { name = param.name, string = paramErrorString })
				param.lock = false
			else
				param.lock = true
			end
		end
		updateParams()
		for _, param in ipairs(Layout.UpdateOrder) do
			if not param:isValid() and not hasNameInArray(param, paramErrors)  then
				table.insert(paramErrors, { name = param.name, string = lui.RECOVERED_BUT_NOT_VALID })
				param.lock = false
			end
		end
		if #paramErrors > 0 then
			table.sort(paramErrors, function(x1, x2) return x1.name < x2.name end)
			report = report .. "\n\n" .. lui.THERE_WERE_ERRORS_IN_RECOVERY .. "\n"
			for _, err in ipairs(paramErrors) do
				report = report .. "\n" .. err.name .. ": " .. err.string
			end
			report = report .. "\n\n" .. lui.UNSUCCESSFULLY_RECOVERED_PARAMETERS_ARE_NOW_UNLOCKED_PLEASE_SET_THEM_MANUALLY
		else
			report = report .. "\n\n" .. lui.ALL_PARAMETERS_WERE_RECOVERED_SUCCESSFULLY
		end
		return true, report
	else
		assert(type(saveGame) == 'string')
		local errorString = saveGame
		return false, lui.AN_ERROR_HAS_OCCURRED .. '\n' .. errorString
	end
end

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

local firstOpen = true
function NewGameWindow:open()
	if (firstOpen or not self.debugMode) and self.mode ~= 'RECOVER' then
		firstOpen = false
		setStartVariant(StartVariants.item(1))
		updateParams()
		profileCombo.selected = 0
	end
	if self.debugMode then
		profileCombo.selected = #profileCombo.items - 1
		unlockAll()
	end
	ModalWindow.open(self)
end

return NewGameWindow
