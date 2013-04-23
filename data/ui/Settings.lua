-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = Engine.ui
local l = Lang.GetDictionary()
local iniTable = Settings:GetGameConfig()
local return_to_menu = ui:Button():SetInnerWidget(ui:Label(l.RETURN_TO_MENU))
return_to_menu.onClick:Connect(function () Settings:SaveGameConfig(iniTable)
					ui:SetInnerWidget(ui.templates.MainMenu()) end)



ui.templates.Settings = function (args) 
	
	local checkboxHandler = function(cb, iniEntry, negate)
		local updater = function()
			local checked
			print ("checked: ",cb.IsChecked)
			if cb.IsChecked then checked = "1" else checked = "0" end
			if negate == true then
				print ("NEG", negate)
				if checked == "1" then checked = "0" else checked = "1" end
			end
			print ("CHECKED: ", checked)
			iniTable[iniEntry] = checked
		end
		cb:SetState(iniTable[iniEntry])
		if negate == true then cb:Toggle() end
		cb.onClick:Connect(updater )
	end
	
	local handleDropDown = function(dd, dl, en) 
		local option = dd.selectedOption
		for i,v in ipairs(dl) do
			if v == option then
				iniTable[en] = ""..i-1
			end
		end
	end
	local setDropdown = function(list, drop)
		for i,v in ipairs(list) do drop:AddOption(list[i]) end
	end
	
	local gameTemplate = function()
			
		local AAList = {"Off", "x2",  "x4", "x8",  "x16"}
		local parseListOut = function(i)
			if i == 1 then return "0" end
			if i == 2 then return  "2" end
			if i == 3 then return  "4" end
			if i == 4 then return  "8" end
			if i == 5 then return  "16" end
		end
		local parseListIn = function(i)
			if i == "0" then return AAList[1] end
			if i == "2" then return  AAList[2] end
			if i == "4" then return  AAList[3] end
			if i == "8" then return  AAList[4] end
			if i == "16" then return  AAList[5] end
		end
		local AADropdown = ui:DropDown()
		setDropdown(AAList, AADropdown)
		local opt = parseListIn(iniTable["AntiAliasingMode"])
		print (opt)
		AADropdown:SetOption(opt)
		AADropdown.onOptionSelected:Connect(function() 
			local option = AADropdown.selectedOption
			for i,v in ipairs(AAList) do
				if v == option then
					iniTable["AntiAliasingMode"] = parseListOut(i)
					
				end
			end
		end)
		
		local vSyncCheckbox = ui:CheckBox()
		local navTunnelCheckbox = ui:CheckBox()
		local invertMouse = ui:CheckBox()
		
		checkboxHandler(invertMouse, "InvertMouseY", false)
		checkboxHandler(vSyncCheckbox,"VSync",false)
		checkboxHandler(navTunnelCheckbox,"DisplayNavTunnel",false)
		
		return ui:Grid({0.25,1,0.25,1,0.25},1)
			:SetCell(1,0, ui:VBox():PackEnd({			
				ui:Background():SetInnerWidget(
					ui:VBox():PackEnd({ui:Label("VSYNC"),
					vSyncCheckbox,
					ui:Margin(5)})
				),
				ui:Background():SetInnerWidget(
					ui:VBox():PackEnd({ui:Label("AA"),
					AADropdown,
					ui:Margin(5)})
				),
			}))
			:SetCell(3,0, ui:VBox():PackEnd({			
				ui:Background():SetInnerWidget(
					ui:VBox():PackEnd({ui:Label(l.DISPLAY_NAV_TUNNEL),
					navTunnelCheckbox,
					ui:Margin(5)})
				),
				ui:Background():SetInnerWidget(
					ui:VBox():PackEnd({ui:Label(l.INVERT_MOUSE_Y),
					invertMouse,
					ui:Margin(5)})
				),
			}))
	end
					
	local videoTemplate = function()
		local modeList = Settings:GetVideoModes()
		
		
		local pDetailList = {l.LOW, l.MEDIUM, l.HIGH, l.VERY_HIGH, l.VERY_VERY_HIGH}
		local pDetailDropdown = ui:DropDown()
		setDropdown(pDetailList, pDetailDropdown)
		pDetailDropdown:SetOption(pDetailList[iniTable["DetailPlanets"]+1])
		pDetailDropdown.onOptionSelected:Connect(function() handleDropDown(pDetailDropdown, pDetailList, "DetailPlanets") end)
	-- 	pDetailDropdown.onOptionSelected:Connect(function() local option = pDetailDropdown.selectedOption
	-- 						print(option) end)
		
		if iniTable["Textures"] == nil then
			iniTable["Textures"] = "0"
		end
		local pTextureList = {l.OFF, l.ON}
		local pTextureDropdown = ui:DropDown()
		setDropdown(pTextureList,pTextureDropdown)
		pTextureDropdown:SetOption(pTextureList[iniTable["Textures"]+1])
		pTextureDropdown.onOptionSelected:Connect(function() handleDropDown(pTextureDropdown,pTextureList,"Textures") end)
		
		if iniTable["FractalMultiple"] == nil then
			iniTable["FractalMultiple"] = "0"
		end
		
		local pFractalDetailList = {l.VERY_LOW, l.LOW, l.MEDIUM, l.HIGH, l.VERY_HIGH}
		local pFractalDropDown = ui:DropDown()
		setDropdown(pFractalDetailList, pFractalDropDown)
		pFractalDropDown:SetOption(pFractalDetailList[iniTable["FractalMultiple"]+1])
		pFractalDropDown.onOptionSelected:Connect(function() handleDropDown(pFractalDropDown, pFractalDetailList, "FractalMultiple") end)
		
		
		local pDetailCities = ui:DropDown()
		setDropdown(pDetailList, pDetailCities)
		pDetailCities:SetOption(pDetailList[iniTable["DetailCities"]+1])
		pDetailCities.onOptionSelected:Connect(function() handleDropDown(pDetailCities, pDetailList, "DetailCities") end)
		
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
							for k, v in string.gmatch(option, "(%w+)x(%w+)") do
								iniTable["ScrWidth"] = k
								iniTable["ScrHeight"] = v
							end
							print (option) end)
-- 		fullScreenCheckBox:SetState(iniTable["StartFullscreen"])
		checkboxHandler(fullScreenCheckBox,"StartFullscreen",false)
		checkboxHandler(compressionCheckBox, "UseTextureCompression", false)
		checkboxHandler(shadersCheckBox, "DisableShaders", true)
-- 		compressionCheckBox:SetState(iniTable["UseTextureCompression"])
-- 		shadersCheckBox:SetState(iniTable["DisableShaders"])
-- 		shadersCheckBox:Toggle()
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
				ui:Background():SetInnerWidget(
					ui:VBox():PackEnd({ui:Label(l.FRACTAL_DETAIL),
					pFractalDropDown,
					ui:Margin(5)})
				),
				ui:Background():SetInnerWidget(
					ui:VBox():PackEnd({ui:Label(l.CITY_DETAIL_LEVEL),
					pDetailCities,
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
			checkboxHandler(mute, muteKey, false)
-- 			mute:SetState(iniTable[muteKey])

			plus.button.onClick:Connect(function() adj:SetScrollPosition(adj.ScrollPosition + 0.05) end)
			minus.button.onClick:Connect(function() adj:SetScrollPosition(adj.ScrollPosition - 0.05) end)
			return controls
			
		end
		local adjChange = function(adj,label,ini)
			local pos = adj.ScrollPosition 
			iniTable[ini] = pos
			pos = pos*100
			adj.InnerWidget:SetText(label.." "..pos)
			
		end
		masterVolume.OnChange:Connect(function() adjChange(masterVolume, l.VOL_MASTER,"MasterVolume" ) end)
		musicVolume.OnChange:Connect(function() adjChange(musicVolume, l.VOL_MUSIC,"MusicVolume" ) end)
		sfxVolume.OnChange:Connect(function() adjChange(sfxVolume, l.VOL_EFFECTS,"SfxVolume" ) end)
		
		return ui:Grid({1,2,1}, 3)
			:SetCell(0,0,controlFunc(masterVolume,"MasterMuted")) 
			:SetCell(1,0,masterVolume) 
			:SetCell(0,1,controlFunc(musicVolume,"MusicMuted")) 
			:SetCell(1,1,musicVolume)
			:SetCell(0,2,controlFunc(sfxVolume,"SfxMuted")) 
			:SetCell(1,2,sfxVolume)
	end
	local languageTemplate = function()
		local langs = Lang:GetCoreLanguages()
		local langList = {}
		for i,v in ipairs(langs) do
			table.insert(langList,v)
		end
		local langDropdown = ui:DropDown()
		setDropdown(langList,langDropdown)
		langDropdown:SetOption(iniTable["Lang"])
		langDropdown.onOptionSelected:Connect(function() iniTable["Lang"] = langDropdown.selectedOption end)
		return ui:VBox():PackEnd({ui:Label(l.LANGUAGE_SELECTION),langDropdown})
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
