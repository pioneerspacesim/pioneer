-- Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Game = require 'Game'
local Ship = require 'Ship'
local ShipDef = require 'ShipDef'
local SystemPath = require 'SystemPath'
local Equipment = require 'Equipment'
local Format = require 'Format'
local MusicPlayer = require 'modules.MusicPlayer'
local Lang = require 'Lang'
local FlightLog = require 'FlightLog'
local Commodities = require 'Commodities'
local Character = require 'Character'
local Vector2 = _G.Vector2

local lc = Lang.GetResource("core")
local lui = Lang.GetResource("ui-core")
local qlc = Lang.GetResource("quitconfirmation-core")
local elc = Lang.GetResource("equipment-core")
local clc = Lang.GetResource("commodity")

local ui = require 'pigui'

local misc = Equipment.misc
local laser = Equipment.laser
local hyperspace = Equipment.hyperspace

local colors = ui.theme.colors
local pionillium = ui.fonts.pionillium
local orbiteer = ui.fonts.orbiteer

local mainButtonSize = ui.rescaleUI(Vector2(400,46))
local dialogButtonSize = ui.rescaleUI(Vector2(150,46))
local mainButtonFont = pionillium.large
local quitPadding = ui.rescaleUI(Vector2(12, 12))

local showQuitConfirm = false
local quitConfirmMsg
local max_flavours = 22

local startLocations = {
	{
		['name']       = lui.START_AT_MARS,
		['desc']       = lui.START_AT_MARS_DESC,
		['location']   = SystemPath.New(0,0,0,0,18),
		['logmsg']     = lui.START_LOG_ENTRY_1,
		['shipType']   = 'coronatrix',
		['money']      = 600,
		['hyperdrive'] = true,
		['equipment']  = {
			{ laser.pulsecannon_1mw,      1 },
			{ misc.atmospheric_shielding, 1 },
			{ misc.autopilot,             1 },
			{ misc.radar,                 1 }
		},
		['cargo']      = {
			{ Commodities.hydrogen, 2 }
		}
	},
	{
		['name']       = lui.START_AT_NEW_HOPE,
		['desc']       = lui.START_AT_NEW_HOPE_DESC,
		['location']   = SystemPath.New(1,-1,-1,0,4),
		['logmsg']     = lui.START_LOG_ENTRY_2,
		['shipType']   = 'pumpkinseed',
		['money']      = 400,
		['hyperdrive'] = true,
		['equipment']  = {
			{ laser.pulsecannon_1mw,      1 },
			{ misc.atmospheric_shielding, 1 },
			{ misc.autopilot,             1 },
			{ misc.radar,                 1 }
		},
		['cargo']      = {
			{ Commodities.hydrogen, 2 }
		}
	},
	{	['name']        = lui.START_AT_BARNARDS_STAR,
		['desc']        = lui.START_AT_BARNARDS_STAR_DESC,
		['location']    = SystemPath.New(-1,0,0,0,16),
		['logmsg']      = lui.START_LOG_ENTRY_3,
		['shipType']    = 'xylophis',
		['money']       = 100,
		['hyperdrive']  = false,
		['equipment']   = {
			{misc.atmospheric_shielding,1},
			{misc.autopilot,1},
			{misc.radar,1}
		},
		['cargo']       = {
			{Commodities.hydrogen,2}
		}
	}
}

local function dialogTextButton(label, enabled, callback)
	local variant = not enabled and ui.theme.buttonColors.disabled or nil

	local button
	ui.withFont(mainButtonFont, function()
		button = ui.button(label, dialogButtonSize, variant)
	end)
	if button then
		callback()
	end
	return button
end

local function mainTextButton(label, tooltip, enabled, callback)
	local variant = not enabled and ui.theme.buttonColors.disabled or nil

	local button
	ui.withFont(mainButtonFont, function()
		button = ui.button(label, mainButtonSize, variant, tooltip)
	end)
	if button and enabled then
		callback()
	end
	return button
end --mainTextButton

local function confirmQuit()
	ui.setNextWindowPosCenter('Always')

	ui.withStyleColorsAndVars({WindowBg = colors.blueBackground:opacity(0.70)}, {WindowPadding = quitPadding}, function()
		-- TODO: this window should be ShowBorders
		ui.window("MainMenuQuitConfirm", {"NoTitleBar", "NoResize", "AlwaysAutoResize"}, function()
			local w = dialogButtonSize.x * 0.6
			local fullW = w * 3 + dialogButtonSize.x * 2

			ui.withFont(orbiteer.medlarge, function()
				ui.text(qlc.QUIT)
			end)
			ui.withFont(pionillium.medlarge, function()
				ui.pushTextWrapPos(fullW)
				ui.textWrapped(quitConfirmMsg)
				ui.popTextWrapPos()
			end)
			ui.dummy(Vector2(5,15)) -- small vertical spacing

			ui.dummy(Vector2(0, 0))
			ui.sameLine(0, w)
			dialogTextButton(qlc.YES, true, Engine.Quit)
			ui.sameLine(0, w)
			dialogTextButton(qlc.NO, true, function() showQuitConfirm = false end)
			ui.sameLine(0, w)
			ui.dummy(Vector2(0, 0))
		end)
	end)
end

local function showOptions()
	ui.optionsWindow:open()
end

local function quitGame()
	if Engine.GetConfirmQuit() then
		quitConfirmMsg = string.interp(qlc["MSG_" .. Engine.rand:Integer(1, max_flavours)],{yes = qlc.YES, no = qlc.NO})
		showQuitConfirm = true
	else
		Engine.Quit()
	end
end

local function continueGame()
	if Game.CanLoadGame('_exit') and ( Engine.GetAutosaveEnabled() or not Game.CanLoadGame('_quicksave') ) then
		Game.LoadGame("_exit")
	else
		Game.LoadGame('_quicksave')
	end
end

local function canContinue()
	return Game.CanLoadGame('_exit') or Game.CanLoadGame('_quicksave')
end

local hasMusicList = false -- required false at init, see showMainMenu() for usage
local function startAtLocation(location)
	hasMusicList = false -- set false so that player restarts music
	Game.StartGame(location.location)

	Game.player:SetShipType(location.shipType)
	Game.player:SetLabel(Ship.MakeRandomLabel())

	Game.player:SetMoney(location.money)

	local ship = Game.player
	-- Generate crew for the starting ship
	for i = 2, ShipDef[ship.shipId].minCrew do
		local newCrew = Character.New()
		newCrew:RollNew(true)
		ship:Enroll(newCrew)
	end

	if location.hyperdrive then
		Game.player:AddEquip(hyperspace['hyperdrive_'..ShipDef[Game.player.shipId].hyperdriveClass])
	end

	for _,equip in pairs(location.equipment) do
		Game.player:AddEquip(equip[1],equip[2])
	end

	---@type CargoManager
	local cargoMgr = Game.player:GetComponent('CargoManager')
	for _,cargo in pairs(location.cargo) do
		cargoMgr:AddCommodity(cargo[1], cargo[2])
	end

	-- XXX horrible hack here to avoid paying a spawn-in docking fee
	Game.player:setprop("is_first_spawn", true)
	FlightLog.MakeCustomEntry(location.logmsg)
end

local function callModules(mode)
	for _,v in ipairs(ui.getModules(mode)) do
		v.draw()
	end
end

local overlayWindowFlags = ui.WindowFlags {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus", "AlwaysAutoResize"}
local mainMenuFlags = ui.WindowFlags{"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus"}
local function showMainMenu()
	if not hasMusicList then
		hasMusicList = true
		MusicPlayer.rebuildSongList()
		MusicPlayer.playRandomSongFromCategory("menu", true)
	end

	local showContinue = canContinue()
	local buttons = 4

	local winPos = Vector2(ui.screenWidth - mainButtonSize.x - 100, ui.screenHeight/2 - (buttons * mainButtonSize.y)/2 - (2*mainButtonSize.y)/2 - 8)

	ui.setNextWindowPos(Vector2(110,65),'Always')
	ui.withStyleColors({["WindowBg"]=colors.transparent}, function()
		ui.window("headingWindow", overlayWindowFlags, function()
			ui.withFont(orbiteer.xlarge, function() ui.text("Pioneer") end)
		end)
	end)
	if Engine.IsIntroZooming() then
		ui.setNextWindowPos(Vector2(0,0),'Always')
		ui.setNextWindowSize(Vector2(ui.screenWidth, ui.screenHeight), 'Always')
		ui.withStyleColors({["WindowBg"]=colors.transparent}, function()
			ui.window("shipinfoWindow", overlayWindowFlags, function()
				local mn = Engine.GetIntroCurrentModelName()
				if mn then
					local sd = ShipDef[mn]
					if sd then
						ui.addFancyText(Vector2(ui.screenWidth / 3, ui.screenHeight / 5.0 * 4), ui.anchor.center, ui.anchor.bottom, {{text=sd.name, color=colors.white, font=orbiteer.medlarge}}, colors.transparent)
						ui.addFancyText(Vector2(ui.screenWidth / 3, ui.screenHeight / 5.0 * 4.02), ui.anchor.center, ui.anchor.top, {{text=lui[sd.shipClass:upper()], color=colors.white, font=orbiteer.medium}}, colors.transparent)
					end
				end
			end)
		end)
	end
	local build_text = Engine.version
	ui.withFont(orbiteer.medium,
		function()
			ui.setNextWindowPos(Vector2(ui.screenWidth - ui.calcTextSize(build_text).x * 1.2,ui.screenHeight - 50), 'Always')
			ui.withStyleColors({["WindowBg"] = colors.transparent}, function()
				ui.window("buildLabel", overlayWindowFlags, function()
					ui.text(build_text)
				end)
			end)
		end)

	ui.setNextWindowPos(winPos,'Always')
	ui.setNextWindowSize(Vector2(0,0), 'Always')
	ui.withStyleColors({["WindowBg"] = colors.lightBlackBackground}, function()
		ui.window("MainMenuButtons", mainMenuFlags, function()
			mainTextButton(lui.CONTINUE_GAME, nil, showContinue, continueGame)

			for _,loc in pairs(startLocations) do
				local desc = loc.desc .. "\n"
				local name = loc.shipType
				local sd = ShipDef[loc.shipType]
				if sd then
					name = sd.name
				end
				desc = desc .. lc.SHIP .. ": " .. name .. "\n"
				desc = desc .. lui.CREDITS .. ": "  .. Format.Money(loc.money) .. "\n"
				desc = desc .. lui.HYPERDRIVE .. ": " .. (loc.hyperdrive and lui.YES or lui.NO) .. "\n"
				desc = desc .. lui.EQUIPMENT .. ":\n"
				for _,eq in pairs(loc.equipment) do
					local equipname = rawget(elc, eq[1].l10n_key) or rawget(clc, eq[1].l10n_key)
					desc = desc .. "  - " .. equipname
					if eq[2] > 1 then
						desc = desc .. " x" .. eq[2] .. "\n"
					else desc = desc .. "\n"
					end
				end
				mainTextButton(loc.name, desc, true, function() startAtLocation(loc) end)
			end

			mainTextButton(lui.LOAD_GAME, nil, true, function()
				ui.saveLoadWindow.mode = "LOAD"
				ui.saveLoadWindow:open()
			end)

			mainTextButton(lui.OPTIONS, nil, true, showOptions)
			mainTextButton(lui.QUIT, nil, true, quitGame)

			if showQuitConfirm then confirmQuit() end
		end)
	end)

	callModules('mainMenu')
	callModules('modal')
end -- showMainMenu

ui.registerHandler('mainMenu', showMainMenu)
