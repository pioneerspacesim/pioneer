-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = Engine.ui
local l = Lang.GetDictionary()

local return_to_menu = ui:Button():SetInnerWidget(ui:Label(l.RETURN_TO_MENU))
return_to_menu.onClick:Connect(function () ui:SetInnerWidget(ui.templates.MainMenu()) end)



ui.templates.Settings = function (args) 
  local gameTemplate = function()
    return ui:VBox():PackEnd({
	    ui:HBox():PackEnd({
	      ui:Background():SetInnerWidget(ui:HBox(5):PackEnd({ui:CheckBox(), ui:Label("Game Test"),
	      })),
	      ui:Margin(5),
	      ui:Background():SetInnerWidget(ui:HBox(5):PackEnd({ui:CheckBox(), ui:Label("Game Test2"),
	      }))
	    })
	  })
  end
  local modeTable = Settings:GetVideoModes()
  local iniTable = Settings:GetGameConfig()
  local screenModeList = ui:List()
	for i = 1,#modeTable do screenModeList:AddOption(modeTable[i]) end
  local videoTemplate = function()
    return ui:VBox():PackEnd({
	    ui:HBox():PackEnd({
	      ui:Background():SetInnerWidget(ui:HBox(5):PackEnd({screenModeList, ui:Label("Video Test"),
	      })),
	      ui:Margin(5),
	      ui:Background():SetInnerWidget(ui:HBox(5):PackEnd({ui:CheckBox(), ui:Label("Video Test2"),
	      }))
	    })
	  })
  end
  local soundTemplate = function()
    return ui:VBox():PackEnd({
	    ui:HBox():PackEnd({
	      ui:Background():SetInnerWidget(ui:HBox(5):PackEnd({ui:CheckBox(), ui:Label("Sound Test"),
	      })),
	      ui:Margin(5),
	      ui:Background():SetInnerWidget(ui:HBox(5):PackEnd({ui:CheckBox(), ui:Label("Sound Test2"),
	      }))
	    })
	  })
  end
  local languageTemplate = function()
    return ui:VBox():PackEnd({
	    ui:HBox():PackEnd({
	      ui:Background():SetInnerWidget(ui:HBox(5):PackEnd({ui:CheckBox(), ui:Label("Language Test"),
	      })),
	      ui:Margin(5),
	      ui:Background():SetInnerWidget(ui:HBox(5):PackEnd({ui:CheckBox(), ui:Label("Language Test2"),
	      }))
	    })
	  })
  end
  local setTabs = UI.TabGroup.New()
  setTabs:AddTab({ id = "Game",        title = l.GAME,     icon = "GameBoy", template = gameTemplate         })
  setTabs:AddTab({ id = "Video",        title = l.VIDEO,     icon = "VideoCamera", template = videoTemplate         })
  setTabs:AddTab({ id = "Sound",        title = l.SOUND,     icon = "Speaker", template = soundTemplate         })
  setTabs:AddTab({ id = "Language",        title = l.LANGUAGE,     icon = "Globe1", template = languageTemplate         })
  local settings =
    ui:Background():SetInnerWidget(ui:Margin(30):SetInnerWidget(
	    ui:VBox(10):PackEnd({
		    setTabs.widget,
		    return_to_menu
		    
	    })
    ))  
  
  print("videotable", #modeTable)
--   print ("iniTable", #iniTable)
--   print("BindViewForward", iniTable["BindViewForward"])
  local i = 0
  for x,y in pairs(iniTable) do
    i = i +1
  end
  print("counted number: ", i)
  return settings
end
