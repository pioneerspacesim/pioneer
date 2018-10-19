-- Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import('Engine')
local Game = import('Game')
local Ship = import("Ship")
local ShipDef = import("ShipDef")
local SystemPath = import("SystemPath")
local Equipment = import("Equipment")
local Format = import("Format")
local ui = import('pigui/pigui.lua')
local Vector = import('Vector')
local Event = import('Event')
local Lang = import("Lang")
local lc = Lang.GetResource("core")
local lui = Lang.GetResource("ui-core")
local qlc = Lang.GetResource("quitconfirmation-core")
local elc = Lang.GetResource("equipment-core")
local clc = Lang.GetResource("commodity")

local cargo = Equipment.cargo
local misc = Equipment.misc
local laser = Equipment.laser
local hyperspace = Equipment.hyperspace

local player = nil
local colors = ui.theme.colors
local pionillium = ui.fonts.pionillium
local icons = ui.theme.icons

local mainButtonSize = Vector(400,46) * (ui.screenHeight / 1200)
local dialogButtonSize = Vector(150,46) * (ui.screenHeight / 1200)
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
			ui.dummy(Vector(5,15)) -- small vertical spacing
			ui.dummy(Vector(30, 5))
			ui.sameLine()
			dialogTextButton(qlc.YES, true, Engine.Quit)
			ui.sameLine()
			ui.dummy(Vector(30, 5))
			ui.sameLine()
			dialogTextButton(qlc.NO, true, function() showQuitConfirm = false end)
		end)
	end)
end

local function showOptions()
	ui.showOptionsWindow = true
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
	Game.LoadGame("_exit")
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
	local canContinue = Game.CanLoadGame('_exit')
	local buttons = 4

	local winPos = Vector(ui.screenWidth - mainButtonSize.x - 100, ui.screenHeight/2 - (buttons * mainButtonSize.y)/2 - (2*mainButtonSize.y)/2 - 8)

	ui.setNextWindowPos(Vector(110,65),'Always')
	ui.withStyleColors({["WindowBg"]=colors.transparent}, function()
		ui.window("headingWindow", {"NoTitleBar","NoResize","NoFocusOnAppearing","NoBringToFrontOnFocus","AlwaysAutoResize"}, function()
			ui.withFont("orbiteer",36 * (ui.screenHeight/1200),function() ui.text("Pioneer") end)
		end)
	end)
	local build_text = Engine.version
	ui.withFont("orbiteer", 16 * (ui.screenHeight/1200),
							function()
								ui.setNextWindowPos(Vector(ui.screenWidth - ui.calcTextSize(build_text).x * 1.2,ui.screenHeight - 50), 'Always')
								ui.withStyleColors({["WindowBg"] = colors.transparent}, function()
										ui.window("buildLabel", {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus", "AlwaysAutoResize"},
															function()
																ui.text(build_text)
										end)
								end)
	end)

	ui.setNextWindowPos(winPos,'Always')
	ui.withStyleColors({["WindowBg"] = colors.lightBlackBackground}, function()
		ui.window("MainMenuButtons", {"NoTitleBar", "NoResize", "NoFocusOnAppearing", "NoBringToFrontOnFocus"}, function()
			mainTextButton(lui.CONTINUE_GAME, nil, canContinue, continueGame)

			for _,loc in pairs(startLocations) do
				local desc = loc.desc .. "\n"
				desc = desc .. lc.SHIP .. ": " .. loc.shipType .. "\n"
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

			mainTextButton(lui.LOAD_GAME, nil, true, function() ui.showSavedGameWindow = "LOAD" end)
			mainTextButton(lui.OPTIONS, nil, true, showOptions)
			mainTextButton(lui.QUIT, nil, true, quitGame)

			if showQuitConfirm then confirmQuit() end
		end)
	end)

	callModules('mainMenu')
end -- showMainMenu

ui.registerHandler('mainMenu',function(delta)
	showMainMenu()
end)
