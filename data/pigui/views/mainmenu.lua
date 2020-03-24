-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Game = require 'Game'
local Ship = require 'Ship'
local ShipDef = require 'ShipDef'
local SystemPath = require 'SystemPath'
local Equipment = require 'Equipment'
local Format = require 'Format'
local Event = require 'Event'
local Lang = require 'Lang'

local lc = Lang.GetResource("core")
local lui = Lang.GetResource("ui-core")
local qlc = Lang.GetResource("quitconfirmation-core")
local elc = Lang.GetResource("equipment-core")
local clc = Lang.GetResource("commodity")

local ui = require 'pigui'

local cargo = Equipment.cargo
local misc = Equipment.misc
local laser = Equipment.laser
local hyperspace = Equipment.hyperspace

local player = nil
local colors = ui.theme.colors
local pionillium = ui.fonts.pionillium
local orbiteer = ui.fonts.orbiteer
local icons = ui.theme.icons

local mainButtonSize = Vector2(400,46) * (ui.screenHeight / 1200)
local dialogButtonSize = Vector2(150,46) * (ui.screenHeight / 1200)
local mainButtonFontSize = 24 * (ui.screenHeight / 1200)

local showQuitConfirm = false
local quitConfirmMsg
local max_flavours = 22

local startLocations = {
	{['name']=lui.START_AT_MARS,
	 ['desc']=lui.START_AT_MARS_DESC,
	 ['location']=SystemPath.New(0,0,0,0,18),
	 ['shipType']='sinonatrix',['money']=100,['hyperdrive']=true,
	 ['equipment']={
		{laser.pulsecannon_1mw,1},
		{misc.atmospheric_shielding,1},
		{misc.autopilot,1},
		{misc.radar,1},
		{cargo.hydrogen,2}}},
	{['name']=lui.START_AT_NEW_HOPE,
	 ['desc']=lui.START_AT_NEW_HOPE_DESC,
	 ['location']=SystemPath.New(1,-1,-1,0,4),
	 ['shipType']='pumpkinseed',['money']=100,['hyperdrive']=true,
	 ['equipment']={
		{laser.pulsecannon_1mw,1},
		{misc.atmospheric_shielding,1},
		{misc.autopilot,1},
		{misc.radar,1},
		{cargo.hydrogen,2}}},
	{['name']=lui.START_AT_BARNARDS_STAR,
	 ['desc']=lui.START_AT_BARNARDS_STAR_DESC,
	 ['location']=SystemPath.New(-1,0,0,0,16),
	 ['shipType']='xylophis',['money']=100,['hyperdrive']=false,
	 ['equipment']={
		{misc.atmospheric_shielding,1},
		{misc.autopilot,1},
		{misc.radar,1},
		{cargo.hydrogen,2}}}
}

local function dialogTextButton(label, enabled, callback)
	local bgcolor = enabled and colors.buttonBlue or colors.grey

	local button
	ui.withFont(pionillium.large.name, mainButtonFontSize, function()
		button = ui.coloredSelectedButton(label, dialogButtonSize, false, bgcolor, nil, enabled)
	end)
	if button then
		callback()
	end
	return button
end

local function mainTextButton(label, tooltip, enabled, callback)
	local bgcolor = enabled and colors.buttonBlue or colors.grey

	local button
	ui.withFont(pionillium.large.name, mainButtonFontSize, function()
		button = ui.coloredSelectedButton(label, mainButtonSize, false, bgcolor, tooltip, enabled)
	end)
	if button and enabled then
		callback()
	end
	return button
end --mainTextButton

local function confirmQuit()
	ui.setNextWindowPosCenter('Always')

	ui.withStyleColors({["WindowBg"] = colors.commsWindowBackground}, function()
		-- TODO: this window should be ShowBorders
		ui.window("MainMenuQuitConfirm", {"NoTitleBar", "NoResize", "AlwaysAutoResize"}, function()
			ui.withFont(pionillium.large.name, mainButtonFontSize, function()
				ui.text(qlc.QUIT)
			end)
			ui.withFont(pionillium.large.name, pionillium.large.size, function()
				ui.pushTextWrapPos(450)
				ui.textWrapped(quitConfirmMsg)
				ui.popTextWrapPos()
			end)
			ui.dummy(Vector2(5,15)) -- small vertical spacing
			ui.dummy(Vector2(30, 5))
			ui.sameLine()
			dialogTextButton(qlc.YES, true, Engine.Quit)
			ui.sameLine()
			ui.dummy(Vector2(30, 5))
			ui.sameLine()
			dialogTextButton(qlc.NO, true, function() showQuitConfirm = false end)
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

local function startAtLocation(location)
	Game.StartGame(location.location)
	Game.player:SetShipType(location.shipType)
	Game.player:SetLabel(Ship.MakeRandomLabel())
	if location.hyperdrive then
		Game.player:AddEquip(hyperspace['hyperdrive_'..ShipDef[Game.player.shipId].hyperdriveClass])
	end
	Game.player:SetMoney(location.money)
	for _,equip in pairs(location.equipment) do
		Game.player:AddEquip(equip[1],equip[2])
	end
end

local function callModules(mode)
	for k,v in pairs(ui.getModules(mode)) do
		v.fun()
	end
end

local function showMainMenu()
	local showContinue = canContinue()
	local buttons = 4

	local winPos = Vector2(ui.screenWidth - mainButtonSize.x - 100, ui.screenHeight/2 - (buttons * mainButtonSize.y)/2 - (2*mainButtonSize.y)/2 - 8)

	ui.setNextWindowPos(Vector2(110,65),'Always')
	ui.withStyleColors({["WindowBg"]=colors.transparent}, function()
		ui.window("headingWindow", {"NoTitleBar","NoResize","NoFocusOnAppearing","NoBringToFrontOnFocus","AlwaysAutoResize"}, function()
			ui.withFont("orbiteer",36 * (ui.screenHeight/1200),function() ui.text("Pioneer") end)
		end)
	end)
	if Engine.IsIntroZooming() then
		ui.setNextWindowPos(Vector2(0,0),'Always')
		ui.setNextWindowSize(Vector2(ui.screenWidth, ui.screenHeight), 'Always')
		ui.withStyleColors({["WindowBg"]=colors.transparent}, function()
			ui.window("shipinfoWindow", {"NoTitleBar","NoResize","NoFocusOnAppearing","NoBringToFrontOnFocus","AlwaysAutoResize"}, function()
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
	ui.withFont("orbiteer", 16 * (ui.screenHeight/1200),
							function()
								ui.setNextWindowPos(Vector2(ui.screenWidth - ui.calcTextSize(build_text).x * 1.2,ui.screenHeight - 50), 'Always')
								ui.withStyleColors({["WindowBg"] = colors.transparent}, function()
										ui.window("buildLabel", {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus", "AlwaysAutoResize"},
															function()
																ui.text(build_text)
										end)
								end)
	end)

	ui.setNextWindowPos(winPos,'Always')
	ui.setNextWindowSize(Vector2(0,0), 'Always')
	ui.withStyleColors({["WindowBg"] = colors.lightBlackBackground}, function()
		ui.window("MainMenuButtons", {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus"}, function()
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
				local equipname
					if pcall(function() local t = elc[eq[1].l10n_key] end) then
						equipname = elc[eq[1].l10n_key]
					elseif pcall(function() local t= clc[eq[1].l10n_key] end) then
						equipname = clc[eq[1].l10n_key]
					end
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

ui.registerHandler('mainMenu',function(delta)
	showMainMenu()
end)
