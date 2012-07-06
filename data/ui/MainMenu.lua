local ui = Engine.ui
local l = Lang.GetDictionary()

local addDebugEnemy = function ()
	local enemy = Space.SpawnShipNear("Eagle Long Range Fighter", Game.player, 9, 9)
	enemy:AddEquip("PULSECANNON_1MW")
	enemy:AddEquip("HYDROGEN", 2)
	enemy:AddEquip("ATMOSPHERIC_SHIELDING")
	enemy:AddEquip("AUTOPILOT")
	enemy:AddEquip("SCANNER")
	enemy:AIKill(Game.player)
	Game.player:SetCombatTarget(enemy)
end

local buttonDefs = {
	{ l.MM_START_NEW_GAME_EARTH,     function () Game.StartGame(SystemPath.New(0,0,0,0,9))    end },
	{ l.MM_START_NEW_GAME_E_ERIDANI, function () Game.StartGame(SystemPath.New(1,-1,-1,0,4))  end },
	{ l.MM_START_NEW_GAME_LAVE,      function () Game.StartGame(SystemPath.New(-2,1,90,0,2))  end },
	{ l.MM_START_NEW_GAME_DEBUG,     function () Game.StartGame(SystemPath.New(-1,9,-22,0,5)) addDebugEnemy() end },
	{ l.MM_LOAD_SAVED_GAME,          function () print("load game") end },
	{ l.MM_QUIT,                     function () print("quit") end },
}


local buttonSet = {}
for i = 1,#buttonDefs do
    local def = buttonDefs[i]
    local label = ui:Label(def[1])
    local button = ui:Button()
    button.onClick:Connect(def[2])
    buttonSet[i] = ui:HBox():PackEnd({ button, label })
end

local menu = 
	ui:Margin(10):SetInnerWidget(
		ui:Grid(1, { 0.25,0.5,0.25 })
			:SetCell(0,2,
				ui:Grid({ 0.2, 0.8 }, 1)
					:SetRow(0, {
						ui:Image("icons/badge.png"),
						ui:Align("LEFT"):SetInnerWidget(
							ui:Margin(10):SetInnerWidget(
								ui:VBox():PackEnd({
									ui:Label("Pioneer"):SetFontSize("XLARGE"),
									ui:Label(Engine.version)
								})
							)
						)
					})
			)
			:SetCell(0,1,
				ui:Align("MIDDLE"):SetInnerWidget(
					ui:VBox():PackEnd(buttonSet)
				)
			)
	)

ui:AddToCatalog("MainMenu", menu);
