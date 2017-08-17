-- Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Lang = import("Lang")
local Game = import("Game")
local Ship = import("Ship")
local ShipDef = import("ShipDef")
local Player = import("Player")
local SystemPath = import("SystemPath")
local ErrorScreen = import("ErrorScreen")
local equipment = import("Equipment")
local cargo = equipment.cargo
local misc = equipment.misc
local laser = equipment.laser
local hyperspace = equipment.hyperspace

local ui = Engine.ui
local l = Lang.GetResource("ui-core");

local setupPlayerSol = function ()
	Game.player:SetShipType("sinonatrix")
	Game.player:SetLabel(Ship.MakeRandomLabel())
	Game.player:AddEquip(hyperspace["hyperdrive_"..ShipDef[Game.player.shipId].hyperdriveClass])
	Game.player:AddEquip(laser.pulsecannon_1mw)
	Game.player:AddEquip(misc.atmospheric_shielding)
	Game.player:AddEquip(misc.autopilot)
	Game.player:AddEquip(misc.radar)
	Game.player:AddEquip(cargo.hydrogen, 2)
	Game.player:SetMoney(100)
end

local setupPlayerEridani = function ()
	Game.player:SetShipType("pumpkinseed")
	Game.player:SetLabel(Ship.MakeRandomLabel())
	Game.player:AddEquip(hyperspace["hyperdrive_"..ShipDef[Game.player.shipId].hyperdriveClass])
	Game.player:AddEquip(laser.pulsecannon_1mw)
	Game.player:AddEquip(misc.atmospheric_shielding)
	Game.player:AddEquip(misc.autopilot)
	Game.player:AddEquip(misc.radar)
	Game.player:AddEquip(cargo.hydrogen, 2)
	Game.player:SetMoney(100)
end

local setupPlayerBarnard = function ()
	Game.player:SetShipType("xylophis")
	Game.player:SetLabel(Ship.MakeRandomLabel())
	--Game.player:AddEquip(equipment.laser.pulsecannon_1mw)
	Game.player:AddEquip(misc.atmospheric_shielding)
	Game.player:AddEquip(misc.autopilot)
	Game.player:AddEquip(misc.radar)
	Game.player:AddEquip(cargo.hydrogen, 2)
	Game.player:SetMoney(100)
end

local loadGame = function (path)
	local ok, err = pcall(Game.LoadGame, path)
	if not ok then
		ErrorScreen.ShowError(l.COULD_NOT_LOAD_GAME .. err)
	end
end

local doLoadDialog = function ()
	ui:NewLayer(
		ui.templates.FileDialog({
			title       = l.LOAD,
			helpText    = l.SELECT_GAME_TO_LOAD,
			path        = "savefiles",
			selectLabel = l.LOAD_GAME,
			onSelect    = loadGame,
			onCancel    = function () ui:DropLayer() end
		})
	)
end

local doSettingsScreen = function()
	ui.layer:SetInnerWidget(
		ui.templates.Settings({
			closeButtons = {
				{ text = l.RETURN_TO_MENU, onClick = function () ui.layer:SetInnerWidget(ui.templates.MainMenu()) end },
				{ text = l.OPEN_USER_FOLDER, onClick = Engine.OpenBrowseUserFolder, toDisable = function () return Engine.CanBrowseUserFolder==false end }
			}
		})
	)
end

local doQuitConfirmation = function()
	if Engine.GetConfirmQuit() then
		ui:NewLayer(
			ui.templates.QuitConfirmation({
				onConfirm = function () Engine.Quit() end,
				onCancel  = function () ui:DropLayer() end
			})
		)
	else
		Engine.Quit()
	end
end

local buttonDefs = {
	{ l.CONTINUE_GAME,          function () loadGame("_exit") end },
	{ l.START_AT_EARTH,         function () Game.StartGame(SystemPath.New(0,0,0,0,6),48600)   setupPlayerSol() end },
	{ l.START_AT_NEW_HOPE,      function () Game.StartGame(SystemPath.New(1,-1,-1,0,4)) setupPlayerEridani() end },
	{ l.START_AT_BARNARDS_STAR, function () Game.StartGame(SystemPath.New(-1,0,0,0,16))  setupPlayerBarnard() end },
	{ l.LOAD_GAME,              doLoadDialog },
	{ l.OPTIONS,                doSettingsScreen },
	{ l.QUIT,                   doQuitConfirmation },
}

local anims = {}

local buttonSet = {}
for i = 1,#buttonDefs do
	local def = buttonDefs[i]
	local button = ui:Button(ui:HBox():PackEnd(ui:Label(def[1])))
	button.onClick:Connect(def[2])
	if i < 10 then button:AddShortcut(i) end
	if i == 10 then button:AddShortcut("0") end
	if 1 == i then
		if not Game.CanLoadGame("_exit") then
			button:Disable()
		end
	end
	buttonSet[i] = button
	table.insert(anims, {
		widget = button,
		type = "IN",
		easing = "ZERO",
		target = "POSITION_X_REV",
		duration = i * 0.05,
		next = {
			widget = button,
			type = "IN",
			easing = "LINEAR",
			target = "POSITION_X_REV",
			duration = 0.4,
		}
	})
end

local headingLabel = ui:Label("Pioneer"):SetFont("HEADING_XLARGE")
table.insert(anims, {
	widget = headingLabel,
	type = "IN",
	easing = "LINEAR",
	target = "OPACITY",
	duration = 0.4,
})

local versionLabel = ui:Label("(build: "..Engine.version..")"):SetFont("HEADING_XSMALL")
table.insert(anims, {
	widget = versionLabel,
	type = "IN",
	easing = "LINEAR",
	target = "OPACITY",
	duration = 0.4,
})

local menu =
	ui:Grid(1, { 0.2, 0.6, 0.2 })
		:SetRow(0, {
			ui:Grid({ 0.1, 0.8, 0.1 }, 1)
				:SetCell(1, 0,
					ui:Align("LEFT",
						headingLabel
					)
				)
		})
		:SetRow(1, {
			ui:Grid(2,1)
				:SetColumn(1, {
					ui:Align("MIDDLE",
						ui:VBox(10):PackEnd(buttonSet):SetFont("HEADING_NORMAL")
					)
				} )
		})
		:SetRow(2, {
			ui:Grid({ 0.1, 0.8, 0.1 }, 1)
				:SetCell(1, 0,
					ui:Align("RIGHT",
						versionLabel
					)
				)
		})

ui.templates.MainMenu = function (args)
	for _,anim in ipairs(anims) do ui:Animate(anim) end
	return menu
end
