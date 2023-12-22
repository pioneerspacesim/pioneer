-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Game = require 'Game'
local Ship = require 'Ship'
local ShipDef = require 'ShipDef'
local Equipment = require 'Equipment'
local MusicPlayer = require 'modules.MusicPlayer'
local Lang = require 'Lang'
local FlightLog = require 'modules.FlightLog.FlightLog'
local Character = require 'Character'
local Vector2 = _G.Vector2
local NewGameWindow = require("pigui.modules.new-game-window.class")

local lui = Lang.GetResource("ui-core")
local qlc = Lang.GetResource("quitconfirmation-core")

local ui = require 'pigui'

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
	end
	
	MusicPlayer.playRandomSongFromCategory("menu", true)
	
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

			mainTextButton(lui.NEW_GAME, nil, true, function()
				NewGameWindow:setDebugMode(ui.ctrlHeld())
				NewGameWindow.mode = 'NEW_GAME'
				NewGameWindow:open();
			end)

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
	callModules('ui-timer')
end -- showMainMenu

ui.registerHandler('mainMenu', showMainMenu)
