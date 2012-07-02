local ui = Engine.ui
local l = Lang.GetDictionary()

local buttons = {}
for i = 1,6 do buttons[i] = ui:Button() end

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
					ui:VBox():PackEnd({
						ui:HBox():PackEnd({ buttons[1], ui:Label(l.MM_START_NEW_GAME_EARTH) }),
						ui:HBox():PackEnd({ buttons[2], ui:Label(l.MM_START_NEW_GAME_E_ERIDANI) }),
						ui:HBox():PackEnd({ buttons[3], ui:Label(l.MM_START_NEW_GAME_LAVE) }),
						ui:HBox():PackEnd({ buttons[4], ui:Label(l.MM_START_NEW_GAME_DEBUG) }),
						ui:HBox():PackEnd({ buttons[5], ui:Label(l.MM_LOAD_SAVED_GAME) }),
						ui:HBox():PackEnd({ buttons[6], ui:Label(l.MM_QUIT) }),
					})
				)
			)
	)

ui:AddToCatalog("MainMenu", menu);
