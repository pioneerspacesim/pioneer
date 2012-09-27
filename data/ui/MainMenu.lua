local ui = Engine.ui
local l = Lang.GetDictionary()

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

local addDebugEnemy = function ()
	local enemy = Space.SpawnShipNear("eagle_lrf", Game.player, 9, 9)
	enemy:AddEquip("PULSECANNON_1MW")
	enemy:AddEquip("ATMOSPHERIC_SHIELDING")
	enemy:AddEquip("AUTOPILOT")
	enemy:AddEquip("SCANNER")
	enemy:AddEquip("HYDROGEN", 2)
	enemy:AIKill(Game.player)
	Game.player:SetCombatTarget(enemy)
end

local doLoadDialog = function ()
	ui:SetInnerWidget(
		ui.templates.FileDialog({
			title       = "Select game to load...",
			path        = "savefiles",
			selectLabel = "Load game",
			onSelect    = function (filename) Game.LoadGame(filename) end,
			onCancel    = function () ui:SetInnerWidget(ui.templates.MainMenu()) end
		})
	)
end

local buttonDefs = {
	{ "Start at Earth",    function () Game.StartGame(SystemPath.New(0,0,0,0,9))   setupPlayerEagle() end },
	{ "Start at New Hope", function () Game.StartGame(SystemPath.New(1,-1,-1,0,4)) setupPlayerEagle() end },
	{ "Start at Lave",     function () Game.StartGame(SystemPath.New(-2,1,90,0,2)) setupPlayerCobra() end },
	--{ l.MM_START_NEW_GAME_DEBUG, function () Game.StartGame(SystemPath.New(-1,9,-22,0,5)) setupPlayerEagle() addDebugEnemy() end },
	{ "Load Game",         doLoadDialog },
	{ "Options",           function () ui:SetInnerWidget(ui.templates.Settings()) end },
	{ "Quit",              function () Engine.Quit() end },
}


local buttonSet = {}
for i = 1,#buttonDefs do
	local def = buttonDefs[i]
	local button = ui:Button():SetInnerWidget(ui:HBox():PackEnd(ui:Label(def[1]), { "FILL", "EXPAND"}))
	button.onClick:Connect(def[2])
	buttonSet[i] = button
end

local menu = 
	ui:Grid(1, { 0.2, 0.6, 0.2 })
		:SetRow(0, {
			ui:Grid({ 0.1, 0.8, 0.1 }, 1)
				:SetCell(1, 0,
					ui:Align("LEFT"):SetInnerWidget(
						ui:Label("Pioneer"):SetFontSize("XLARGE")
					)
				)
		})
		:SetRow(1, {
			ui:Grid(2,1)
				:SetColumn(1, {
					ui:Align("MIDDLE"):SetInnerWidget(
						ui:VBox(10):PackEnd(buttonSet)
					)
				} )
		})
		:SetRow(2, {
			ui:Grid({ 0.1, 0.8, 0.1 }, 1)
				:SetCell(1, 0,
					ui:Align("RIGHT"):SetInnerWidget(
						ui:Label("("..Engine.version..")"):SetFontSize("XSMALL")
					)
				)
		})

ui.templates.MainMenu = function (args) return menu end
