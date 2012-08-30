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

local ui = Engine.ui
local l = Lang.GetDictionary()

local buttonDefs = {
	{ l.MM_START_NEW_GAME_EARTH,     function () Game.StartGame(SystemPath.New(0,0,0,0,9))    setupPlayerEagle()                 end },
	{ l.MM_START_NEW_GAME_E_ERIDANI, function () Game.StartGame(SystemPath.New(1,-1,-1,0,4))  setupPlayerEagle()                 end },
	{ l.MM_START_NEW_GAME_LAVE,      function () Game.StartGame(SystemPath.New(-2,1,90,0,2))  setupPlayerCobra()                 end },
	{ l.MM_START_NEW_GAME_DEBUG,     function () Game.StartGame(SystemPath.New(-1,9,-22,0,5)) setupPlayerEagle() addDebugEnemy() end },
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

ui.templates.MainMenu = function () return menu end
