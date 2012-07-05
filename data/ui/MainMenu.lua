local ui = Engine.ui
local l = Lang.GetDictionary()

local buttonDefs = {
	{ l.MM_START_NEW_GAME_EARTH,     function () print("earth start") end },
	{ l.MM_START_NEW_GAME_E_ERIDANI, function () print("eridani start") end },
	{ l.MM_START_NEW_GAME_LAVE,      function () print("lave start") end },
	{ l.MM_START_NEW_GAME_DEBUG,     function () print("debug start") end },
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
