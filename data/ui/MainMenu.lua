-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = Engine.ui
local t = Translate:GetTranslator()

local setupPlayerEagle = function ()
	Game.player:SetShipType("eagle_lrf")
	Game.player:AddEquip("PULSECANNON_1MW")
	Game.player:AddEquip("ATMOSPHERIC_SHIELDING")
	Game.player:AddEquip("AUTOPILOT")
	Game.player:AddEquip("SCANNER")
	Game.player:AddEquip("MISSILE_GUIDED", 2)
	Game.player:AddEquip("HYDROGEN")
	Game.player:SetMoney(100)
end

local setupPlayerCobra = function ()
	Game.player:SetShipType("cobra3")
	Game.player:AddEquip("PULSECANNON_1MW")
	Game.player:AddEquip("SCANNER")
	Game.player:AddEquip("MISSILE_GUIDED", 2)
	Game.player:AddEquip("HYDROGEN", 2)
	Game.player:SetMoney(100)
end

local doLoadDialog = function ()
	ui:SetInnerWidget(
		ui.templates.FileDialog({
			title       = t("Select game to load..."),
			path        = "savefiles",
			selectLabel = t("Load game"),
			onSelect    = function (filename) Game.LoadGame(filename) end,
			onCancel    = function () ui:SetInnerWidget(ui.templates.MainMenu()) end
		})
	)
end

local buttonDefs = {
	{ t("Start at Earth"),    function () Game.StartGame(SystemPath.New(0,0,0,0,9))   setupPlayerEagle() end },
	{ t("Start at New Hope"), function () Game.StartGame(SystemPath.New(1,-1,-1,0,4)) setupPlayerEagle() end },
	{ t("Start at Lave"),     function () Game.StartGame(SystemPath.New(-2,1,90,0,2)) setupPlayerCobra() end },
	{ t("Load game"),         doLoadDialog },
	{ t("Options"),           function () Engine.SettingsView() end },
	{ t("Quit"),              function () Engine.Quit() end },
}


local buttonSet = {}
for i = 1,#buttonDefs do
	local def = buttonDefs[i]
	local button = ui:Button(ui:HBox():PackEnd(ui:Label(def[1])))
	button.onClick:Connect(def[2])
	buttonSet[i] = button
end

local menu = 
	ui:Grid(1, { 0.2, 0.6, 0.2 })
		:SetRow(0, {
			ui:Grid({ 0.1, 0.8, 0.1 }, 1)
				:SetCell(1, 0,
					ui:Align("LEFT",
						ui:Label("Pioneer"):SetFont("HEADING_XLARGE")
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
						ui:Label("("..Engine.version..")"):SetFont("HEADING_XSMALL")
					)
				)
		})

ui.templates.MainMenu = function (args) return menu end
