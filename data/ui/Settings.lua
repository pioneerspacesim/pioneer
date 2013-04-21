-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = Engine.ui
local l = Lang.GetDictionary()

local return_to_menu = ui:Button():SetInnerWidget(ui:Label(l.RETURN_TO_MENU))
return_to_menu.onClick:Connect(function () ui:SetInnerWidget(ui.templates.MainMenu()) end)



ui.templates.Settings = function (args) 
	local iniTable = Settings:GetGameConfig()
	
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
					
	local videoTemplate = function()
		local modeList = Settings:GetVideoModes()
		
		local setDropdown = function(list, drop)
			for i = 1,#list do drop:AddOption(list[i]) end
		end
		local pDetailList = {l.LOW, l.MEDIUM, l.HIGH, l.VERY_HIGH, l.VERY_VERY_HIGH}
		local pDetailDropdown = ui:DropDown()
		setDropdown(pDetailList, pDetailDropdown)
		pDetailDropdown:SetOption(pDetailList[iniTable["DetailPlanets"]+1])
	-- 	pDetailDropdown.onOptionSelected:Connect(function() local option = pDetailDropdown.selectedOption
	-- 						print(option) end)
		
		if iniTable["Textures"] == nil then
			iniTable["Textures"] = "0"
		end
		local pTextureList = {l.OFF, l.ON}
		local pTextureDropdown = ui:DropDown()
		setDropdown(pTextureList,pTextureDropdown)
		pTextureDropdown:SetOption(pTextureList[iniTable["Textures"]+1])
		
		local fullScreenCheckBox = ui:CheckBox()
		local shadersCheckBox = ui:CheckBox()
		local compressionCheckBox = ui:CheckBox()
		
		local screenModeDropdown = ui:DropDown()
		setDropdown(modeList,screenModeDropdown)
	-- 	for i = 1,#modeList do screenModeDropdown:AddOption(modeList[i]) end
		--   print ("option = ", screenModeDropdown.selectedOption)
		screenModeDropdown:SetOption(iniTable["ScrWidth"].."x"..iniTable["ScrHeight"]);
	-- 	print ("option = ", screenModeDropdown.selectedOption)
		screenModeDropdown.onOptionSelected:Connect(function() local option = screenModeDropdown.selectedOption
							
							print (option) end)
		fullScreenCheckBox:SetState(iniTable["StartFullscreen"])
		compressionCheckBox:SetState(iniTable["UseTextureCompression"])
		shadersCheckBox:SetState(iniTable["DisableShaders"])
		shadersCheckBox:Toggle()
		local vbox = ui:Grid({0.25,3,0.25,1,0.25},1)
			:SetCell(1,0, ui:VBox():PackEnd({			
				ui:Background():SetInnerWidget(
					ui:VBox():PackEnd({ui:Label(l.VIDEO_RESOLUTION),
					screenModeDropdown,
					ui:Margin(5)})
				),
				ui:Background():SetInnerWidget(
					ui:VBox():PackEnd({ui:Label(l.PLANET_DETAIL_DISTANCE),
					pDetailDropdown,
					ui:Margin(5)})
				),
				ui:Background():SetInnerWidget(
					ui:VBox():PackEnd({ui:Label(l.PLANET_TEXTURES),
					pTextureDropdown,
					ui:Margin(5)})
				),
			}))
			:SetCell(3,0, ui:VBox():PackEnd({			
				ui:Background():SetInnerWidget(
					ui:VBox():PackEnd({ui:Label(l.FULL_SCREEN),
					fullScreenCheckBox,
					ui:Margin(5)})
				),
				ui:Background():SetInnerWidget(
					ui:VBox():PackEnd({ui:Label(l.COMPRESS_TEXTURES),
					compressionCheckBox,
					ui:Margin(5)})
				),
				ui:Background():SetInnerWidget(
					ui:VBox():PackEnd({ui:Label(l.USE_SHADERS),
					shadersCheckBox,
					ui:Margin(5)})
				),
			}))	
					
		return vbox
	
	end
	
	
	local soundTemplate = function()
		local masterVolume = ui:Adjustment(l.VOL_MASTER)
		masterVolume:SetScrollPosition(iniTable["MasterVolume"]);
		local musicVolume = ui:Adjustment(l.VOL_MUSIC)
		musicVolume:SetScrollPosition(iniTable["MusicVolume"]);
		local sfxVolume = ui:Adjustment(l.VOL_EFFECTS)
		sfxVolume:SetScrollPosition(iniTable["SfxVolume"]);
		
		local controlFunc = function(adj,muteKey)
			
			local plus = UI.SmallLabeledButton.New("+")
			local minus = UI.SmallLabeledButton.New("-")
			local mute = ui:CheckBox()
			local controls = ui:HBox():PackEnd({minus,plus,mute,ui:Label("Mute")})
			mute:SetState(iniTable[muteKey])

			plus.button.onClick:Connect(function() adj:SetScrollPosition(adj.ScrollPosition + 0.05) end)
			minus.button.onClick:Connect(function() adj:SetScrollPosition(adj.ScrollPosition - 0.05) end)
			return controls
			
		end
		local adjChange = function(adj,label)
			local pos = adj.ScrollPosition 
			pos = pos*100;
			adj.InnerWidget:SetText(label.." "..pos)
		end
		masterVolume.OnChange:Connect(function() adjChange(masterVolume, l.VOL_MASTER ) end)
		musicVolume.OnChange:Connect(function() adjChange(musicVolume, l.VOL_MUSIC ) end)
		sfxVolume.OnChange:Connect(function() adjChange(sfxVolume, l.VOL_EFFECTS ) end)
		
		return ui:Grid({1,2,1}, 3)
			:SetCell(0,0,controlFunc(masterVolume,"MasterMuted")) 
			:SetCell(1,0,masterVolume) 
			:SetCell(0,1,controlFunc(musicVolume,"MusicMuted")) 
			:SetCell(1,1,musicVolume)
			:SetCell(0,2,controlFunc(sfxVolume,"SfxMuted")) 
			:SetCell(1,2,sfxVolume)
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
	
	
	
	local keysTemplate = function()
		local box = ui:VBox()
		local controlHeaders = Settings:GetHeaders()
		for i = 1,#controlHeaders do 
			local keys = Settings:GetKeys(controlHeaders[i])
			box:PackEnd(ui:Label(controlHeaders[i]))
			for x,y in pairs(keys) do
				local hbox = ui:HBox()
				local label = ui:Label("")
				hbox:PackEnd({ui:Margin(5),ui:Label(x),ui:Label(": "),ui:Background():SetInnerWidget(label)})
				
				label:SetText(y)
				label.onClick:Connect(function() 	local func = Settings:GetKeyFunction(y)
									print (func.." clicked" )
							end)
				ui.onKeyDown:Connect(function(s) 
					for a,b in pairs(s) do
						print ("keyup",a,b) 
					end
				end)
				box:PackEnd(hbox)
			end
		end
		return box 
	end
	
	local setTabs = UI.TabGroup.New()
	setTabs:AddTab({ id = "Game",        title = l.GAME,     icon = "GameBoy", template = gameTemplate         })
	setTabs:AddTab({ id = "Video",        title = l.VIDEO,     icon = "VideoCamera", template = videoTemplate         })
	setTabs:AddTab({ id = "Sound",        title = l.SOUND,     icon = "Speaker", template = soundTemplate         })
	setTabs:AddTab({ id = "Language",        title = l.LANGUAGE,     icon = "Globe1", template = languageTemplate         })
	setTabs:AddTab({ id = "Controls",	title = "Control",	icon = "Gamepad", template= keysTemplate })
	local settings =  ui:Background():SetInnerWidget(ui:Scroller():SetInnerWidget(ui:Margin(30):SetInnerWidget(
		ui:VBox(10):PackEnd({
			setTabs.widget,
			return_to_menu
			
		})
	)))  
	settings.onKeyDown:Connect(function(t) print "press" end)
	ui:SetInnerWidget(settings)
	
end
