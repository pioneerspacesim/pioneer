-- Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import('Engine')
local Input = import('Input')
local Game = import('Game')
local ui = import('pigui/pigui.lua')
local Vector = import('Vector')
local Event = import('Event')
local Lang = import("Lang")
local Color = import("Color")
local lc = Lang.GetResource("core")
local lui = Lang.GetResource("ui-core")

local utils = import("utils")

local player = nil
local colors = ui.theme.colors
local icons = ui.theme.icons
local pionillium = ui.fonts.pionillium

local mainButtonSize = Vector(40,40) * (ui.screenHeight / 1200)
local optionButtonSize = Vector(125,40) * (ui.screenHeight / 1200)
local bindingButtonSize = Vector(177,25) * (ui.screenHeight / 1200)
local mainButtonFramePadding = 3

local bindingPageFontSize = 36 * (ui.screenHeight / 1200)
local bindingGroupFontSize = 26 * (ui.screenHeight / 1200)

local optionsWinSize = Vector(ui.screenWidth * 0.4, ui.screenHeight * 0.6)

local showTab = 'video'

local binding_pages
local showKeyCapture
local keyCaptureId
local keyCaptureNum

local function combo(label, selected, items, tooltip)
	local color = colors.buttonBlue
	local changed, ret = 0
	ui.withStyleColors({["Button"]=color,["ButtonHovered"]=color:tint(0.1),["ButtonActive"]=color:tint(0.2)},function()
		changed, ret = ui.combo(label, selected, items)
	end)
	if ui.isItemHovered() and tooltip then
		Engine.pigui.SetTooltip(tooltip) -- bypass the mouse check, Game.player isn't valid yet
	end
	return changed, ret
end

local function checkbox(label, checked, tooltip)
	local color = colors.buttonBlue
	local changed, ret
	ui.withStyleColors({["Button"]=color,["ButtonHovered"]=color:tint(0.1),["CheckMark"]=color:tint(0.2)},function()
		changed, ret = ui.checkbox(label, checked)
	end)
	if ui.isItemHovered() and tooltip then
		Engine.pigui.SetTooltip(tooltip) -- bypass the mouse check, Game.player isn't valid yet
	end
	return changed, ret
end

local function slider(lbl, value, min, max, tooltip)
	local color = colors.buttonBlue
	local oldval = value
	local ret
	ui.withStyleColors({["SliderGrab"]=color,["ButtonHovered"]=color:tint(0.1),["SliderGrabActive"]=color:tint(0.2)},function()
		ret = ui.sliderInt(lbl, value, min, max)
	end)
	if ui.isItemHovered() and tooltip then
		Engine.pigui.SetTooltip(tooltip) -- bypass the mouse check, Game.player isn't valid yet
	end
	return oldval~=ret,ret
end

local function keyOf(t, value)
	for k,v in pairs(t) do
		if v==value then return k end
	end
	return -1
end

local function bindingTextButton(label, tooltip, enabled, callback)
	local bgcolor = enabled and colors.buttonBlue or colors.grey

	local button
	ui.withFont(pionillium.medium.name, pionillium.medium.size, function()
		button = ui.coloredSelectedButton(label, bindingButtonSize, false, bgcolor, tooltip, enabled)
	end)
	if button then
		callback(button)
	end
end

local function optionTextButton(label, tooltip, enabled, callback)
	local bgcolor = enabled and colors.buttonBlue or colors.grey

	local button
	ui.withFont(pionillium.medium.name, pionillium.medium.size, function()
		button = ui.coloredSelectedButton(label, optionButtonSize, false, bgcolor, tooltip, enabled)
	end)
	if button then
		callback(button)
	end
end --mainButton

local function mainButton(icon, tooltip, selected, callback)
	local button = ui.coloredSelectedIconButton(icon, mainButtonSize, selected, mainButtonFramePadding, colors.buttonBlue, colors.white, tooltip)
	if button then
		callback()
	end
	return button
end --mainButton

local function showVideoOptions()

	local videoModesList = Engine.GetVideoModeList()

	local videoModes = {}
	local videoModeItems = {}
	local currentW,currentH = Engine.GetVideoResolution()
	local currentVideoMode = currentW..'x'..currentH
	local selectedVideoMode = 0

	-- build video mode list
	for i,m in pairs(videoModesList) do
		local lbl = m.width .. 'x' .. m.height
		videoModes[lbl] = m
		videoModeItems[i] = lbl
		if lbl == currentVideoMode then selectedVideoMode = i-1 end
	end

	local aaModes = {}
	local aaLabels = {}
	local maxAAs = Engine.GetMaximumAASamples()

	local curAA = 0

	-- build aa mode list
	while curAA < maxAAs do
		if curAA == 0 then
			table.insert(aaLabels, lui.OFF)
			aaModes[lui.OFF] = curAA
			curAA = 2
		else
			table.insert(aaLabels, 'x'..curAA)
			aaModes['x'..curAA] = curAA
			curAA = curAA * 2
		end
	end
	local selectedAA = keyOf(aaLabels,keyOf(aaModes,Engine.GetMultisampling())) - 1

	local detailLevels = {[lui.VERY_LOW]='VERY_LOW', [lui.LOW]='LOW', [lui.MEDIUM]='MEDIUM', [lui.HIGH]='HIGH', [lui.VERY_HIGH]='VERY_HIGH' }
	local detailLabels = { lui.VERY_LOW, lui.LOW, lui.MEDIUM, lui.HIGH, lui.VERY_HIGH }
	local selectedDetail = keyOf(detailLabels,keyOf(detailLevels, Engine.GetPlanetDetailLevel()))-1

	local fullscreen = Engine.GetFullscreen()
	local vsync = Engine.GetVSyncEnabled()
	local anisoFilter = Engine.GetAnisoFiltering()
	local planetTextures = Engine.GetPlanetFractalColourEnabled()

	local textCompress = Engine.GetTextureCompressionEnabled()
	local gpuJobs = Engine.GetGpuJobsEnabled()

	local fractalDetail = keyOf(detailLabels,keyOf(detailLevels, Engine.GetFractalDetailLevel()))-1
	local cityDetail = keyOf(detailLabels,keyOf(detailLevels, Engine.GetCityDetailLevel()))-1
	local displayNavTunnels = Engine.GetDisplayNavTunnels()
	local displaySpeedLines = Engine.GetDisplaySpeedLines()
	local displayHudTrails = Engine.GetDisplayHudTrails()
	local enableCockpit = Engine.GetCockpitEnabled()
	local enableAutoSave = Engine.GetAutosaveEnabled()
	local starDensity = Engine.GetAmountStars() * 100

	local c
	ui.text(lui.VIDEO_CONFIGURATION_RESTART_GAME_TO_APPLY)

	c,selectedVideoMode = combo(lui.VIDEO_RESOLUTION, selectedVideoMode,videoModeItems,lui.VIDEO_RESOLUTION_DESC)
	if c then
		local mode = videoModes[videoModeItems[selectedVideoMode+1]]
		Engine.SetVideoResolution(mode.width,mode.height)
	end

	c,selectedAA = combo(lui.MULTISAMPLING, selectedAA, aaLabels, lui.MULTISAMPLING_DESC)
	if c then
		local aa = aaModes[aaLabels[selectedAA+1]]
		Engine.SetMultisampling(aa)
	end

	ui.columns(2,"video_checkboxes",false)
	c,fullscreen = checkbox(lui.FULL_SCREEN, fullscreen)
	if c then
		Engine.SetFullscreen(fullscreen)
	end
	ui.nextColumn()
	c,vsync = checkbox(lui.VSYNC, vsync, lui.VSYNC_DESC)
	if c then
		Engine.SetVSyncEnabled(vsync)
	end
	ui.nextColumn()
	c,anisoFilter = checkbox(lui.ENABLE_ANISOTROPIC_FILTERING, anisoFilter, lui.ANISOTROPIC_FILTERING_DESC)
	if c then
		Engine.SetAnisoFiltering(anisoFilter)
	end
	ui.nextColumn()
	c,textCompress = checkbox(lui.COMPRESS_TEXTURES, textCompress, lui.TEXTURE_COMPRESSION)
	if c then
		Engine.SetTextureCompressionEnabled(textCompress)
	end
	ui.nextColumn()
	c,gpuJobs = checkbox(lui.GPU_JOBS, gpuJobs, lui.GPU_JOBS_DESC)
	if c then
		Engine.SetGpuJobsEnabled(gpuJobs)
	end
	ui.columns(1,"",false)


	ui.separator()
	c,selectedDetail = combo(lui.PLANET_DETAIL_DISTANCE, selectedDetail, detailLabels, lui.DETAIL_DESC)
	if c then
		local detail = detailLevels[detailLabels[selectedDetail+1]]
		Engine.SetPlanetDetailLevel(detail)
	end

	c,planetTextures = checkbox(lui.PLANET_TEXTURES, planetTextures, lui.TEXTURE_COMPRESSION)
	if c then
		Engine.SetPlanetFractalColourEnabled(planetTextures)
	end

	c,fractalDetail = combo(lui.FRACTAL_DETAIL, fractalDetail, detailLabels, lui.DETAIL_DESC)
	if c then
		local detail = detailLevels[detailLabels[fractalDetail+1]]
		Engine.SetFractalDetailLevel(detail)
	end

	c,cityDetail = combo(lui.CITY_DETAIL_LEVEL, cityDetail, detailLabels, lui.DETAIL_DESC)
	if c then
		local detail = detailLevels[detailLabels[cityDetail+1]]
		Engine.SetCityDetailLevel(detail)
	end

	c,displayNavTunnels = checkbox(lui.DISPLAY_NAV_TUNNELS, displayNavTunnels, lui.DISPLAY_NAV_TUNNELS_DESC)
	if c then
		Engine.SetDisplayNavTunnels(displayNavTunnels)
	end

	c,displaySpeedLines = checkbox(lui.DISPLAY_SPEED_LINES, displaySpeedLines, lui.DISPLAY_SPEED_LINES_DESC)
	if c then
		Engine.SetDisplaySpeedLines(displaySpeedLines)
	end

	c,displayHudTrails = checkbox(lui.DISPLAY_HUD_TRAILS, displayHudTrails, lui.DISPLAY_HUD_TRAILS_DESC)
	if c then
		Engine.SetDisplayHudTrails(displayHudTrails)
	end

	c,enableCockpit = checkbox(lui.ENABLE_COCKPIT, enableCockpit, lui.ENABLE_COCKPIT_DESC)
	if c then
		Engine.SetCockpitEnabled(enableCockpit)
	end

	c,enableAutoSave = checkbox(lui.ENABLE_AUTOSAVE, enableAutoSave, lui.ENABLE_AUTOSAVE_DESC)
	if c then
		Engine.SetAutosaveEnabled(enableAutoSave)
	end

	c,starDensity = slider(lui.STAR_FIELD_DENSITY, starDensity, 0, 100)
	if c then
		Engine.SetAmountStars(starDensity/100)
	end
end

local function captureBinding(id,num)

	local info

	for _,page in pairs(binding_pages) do
		for _,group in pairs(page) do
			if group.id then
				for _,i in pairs(group) do
					if i.id == id then
						info = i
					end
				end
			end
		end
	end

	ui.setNextWindowPosCenter('Always')
	ui.withStyleColors({["WindowBg"] = Color(20, 20, 80, 230)}, function()
		-- TODO: this window should be ShowBorders
		ui.window("captureBinding", {"NoTitleBar", "NoResize"}, function()
			-- TODO: localizations for binding IDs
			ui.text(info.id)
			ui.text(lui.PRESS_A_KEY_OR_CONTROLLER_BUTTON)

			if info.type == 'action' then
				local desc
				if num == 1 then desc = info.bindingDescription1
				else desc = info.bindingDescription2 end
				desc = desc or '<None>'
				ui.text(desc)

				local bindingKey = Engine.pigui.GetKeyBinding()
				local setBinding = false
				if(bindingKey and num==1 and bindingKey~=info.binding1) or (bindingKey and num==2 and bindingKey~=info.binding2) then setBinding = true end

				if setBinding and  num == 1 then Input.SetActionBinding(info.id, bindingKey, info.binding2)
				elseif setBinding and num==2 then Input.SetActionBinding(info.id, info.binding1, bindingKey)
				end
			elseif info.type == 'axis' then
				local desc
				if num == 1 then desc = info.axisDescription
				elseif num == 2 then desc = info.positiveDescription
				else desc = info.negativeDescription end
				desc = desc or '<None>'
				ui.text(desc)

				if num == 1 then
					local bindingAxis = Engine.pigui.GetAxisBinding()

					if bindingAxis and bindingAxis~=info.axis then
						Input.SetAxisBinding(info.id, bindingAxis, info.positive, info.negative)
					end
				elseif num == 2 then
					local bindingKey = Engine.pigui.GetKeyBinding()

					if bindingKey and bindingKey ~= info.positive then
						Input.SetAxisBinding(info.id, info.axis, bindingKey, info.negative)
					end
				else
					local bindingKey = Engine.pigui.GetKeyBinding()
					if bindingKey and bindingKey ~= info.negative then
						Input.SetAxisBinding(info.id, info.axis, info.positive, bindingKey)
					end
				end
			end

			optionTextButton(lui.OK, nil, true, function() showKeyCapture = false end)
		end)
	end)
end

local function showSoundOptions()
	local masterMuted = Engine.GetMasterMuted()
	local masterLevel = Engine.GetMasterVolume()*100
	local musicMuted = Engine.GetMusicMuted()
	local musicLevel = Engine.GetMusicVolume()*100
	local effectsMuted = Engine.GetEffectsMuted()
	local effectsLevel = Engine.GetEffectsVolume()*100

	local c

	c,masterMuted = checkbox(lui.MUTE.."##master", masterMuted)
	if c then Engine.SetMasterMuted(masterMuted) end
	ui.sameLine()
	c,masterLevel = slider(lui.MASTER_VOL, masterLevel, 0, 100)
	if c then Engine.SetMasterVolume(masterLevel/100) end

	c,musicMuted = checkbox(lui.MUTE.."##music", musicMuted)
	if c then Engine.SetMusicMuted(musicMuted) end
	ui.sameLine()
	c,musicLevel = slider(lui.MUSIC, musicLevel, 0, 100)
	if c then Engine.SetMusicVolume(musicLevel/100) end

	c,effectsMuted = checkbox(lui.MUTE.."##effects", effectsMuted)
	if c then Engine.SetEffectsMuted(effectsMuted) end
	ui.sameLine()
	c,effectsLevel = slider(lui.EFFECTS, effectsLevel, 0, 100)
	if c then Engine.SetEffectsVolume(effectsLevel/100) end
end

local function showLanguageOptions()
	local langs = Lang.GetAvailableLanguages("core")

	ui.withFont(pionillium.large.name, pionillium.large.size, function()
		ui.text(lui.LANGUAGE_RESTART_GAME_TO_APPLY)
	end)

	local clicked
	for _,lang in pairs(langs) do
		ui.withFont(pionillium.large.name, pionillium.large.size, function()
			if ui.selectable(Lang.GetResource("core",lang).LANG_NAME, Lang.currentLanguage==lang, {}) then
				clicked = lang
			end
		end)
	end

	if clicked then
		Lang.SetCurrentLanguage(clicked)
	end
end

local function actionBinding(info)
	local bindings = { info.binding1, info.binding2 }
	local descs = { info.bindingDescription1, info.bindingDescription2 }

	-- TODO: localizations for binding IDs
	if (ui.collapsingHeader(info.id, {})) then
		ui.columns(3,"##bindings",false)
		ui.nextColumn()
		ui.text("Binding")
		bindingTextButton((descs[1] or '')..'##'..info.id..'1', (descs[1] or ''), true, function()
			showKeyCapture = true
			keyCaptureId = info.id
			keyCaptureNum = 1
		end)
		ui.nextColumn()
		ui.text("Alt. Binding")
		bindingTextButton((descs[2] or '')..'##'..info.id..'2', (descs[2] or ''), true, function()
			showKeyCapture = true
			keyCaptureId = info.id
			keyCaptureNum = 2
		end)
		ui.columns(1,"",false)
	end
end

local function axisBinding(info)
	local bindings = { info.axis, info.positive, info.negative }
	local descs = { info.axisDescription, info.positiveDescription, info.negativeDescription }
	-- TODO: localizations for binding IDs
	if (ui.collapsingHeader(info.id, {})) then
		ui.columns(3,"##axisjoybindings",false)
		ui.text("Axis:")
		ui.nextColumn()
		bindingTextButton((descs[1] or '')..'##'..info.id..'axis', (descs[1] or ''), true, function()
			showKeyCapture = true
			keyCaptureId = info.id
			keyCaptureNum = 1
		end)
		ui.nextColumn()
		if info.axis then
			local c, inverted, deadzone, sensitivity = nil, info.axis:sub(1,1) == "-",
				tonumber(info.axis:match"/DZ(%d+%.%d*)" or 0) * 100,
				tonumber(info.axis:match"/E(%d+%.%d*)" or 1) * 100
			local axis = info.axis:match("Joy[0-9a-f]+/Axis%d+")
			local function set_axis()
				local _ax = (inverted and "-" or "") .. axis .. "/DZ" .. deadzone / 100.0 .. "/E" .. sensitivity / 100.0
				Input.SetAxisBinding(info.id, _ax, info.positive, info.negative)
			end
			-- TODO: localize this and find a better way to handle it.
			c,inverted = ui.checkbox("Inverted##"..info.id, inverted, "Invert Axis")
			set_axis()
			ui.nextColumn()
			ui.nextColumn()
			-- ui.columns(3, "##axisinfo", false)
			-- TODO: localize all of these
			-- ui.text("Options:")
			-- ui.nextColumn()
			c, deadzone = slider("Deadzone##"..info.id, deadzone, 0, 100, "Axis Deadzone")
			set_axis()
			ui.nextColumn()
			c,sensitivity = slider("Sensitivity##"..info.id, sensitivity, 0, 100, "Axis Sensitivity")
			set_axis()
		end
		ui.nextColumn()
		ui.columns(3,"##axiskeybindings",false)
		-- TODO: translate this string
		ui.text("Key Bindings:")
		ui.nextColumn()
		ui.text("Positive:")
		bindingTextButton((descs[2] or '')..'##'..info.id..'positive', (descs[2] or ''), true, function()
			showKeyCapture = true
			keyCaptureId = info.id
			keyCaptureNum = 2
		end)
		ui.nextColumn()
		ui.text("Negative:")
		bindingTextButton((descs[3] or '')..'##'..info.id..'negative', (descs[3] or ''), true, function()
			showKeyCapture = true
			keyCaptureId = info.id
			keyCaptureNum = 3
		end)
		ui.columns(1,"",false)
	end
end

local function showControlsOptions()
	ui.text(lui.CONTROL_OPTIONS)

	local mouseYInvert = Input.GetMouseYInverted()
	local joystickEnabled = Input.GetJoystickEnabled()
	binding_pages = Input.GetBindings()
	local c

	c,mouseYInvert = checkbox(lui.INVERT_MOUSE_Y, mouseYInvert)
	if c then Input.SetMouseYInverted(mouseYInvert) end

	c,joystickEnabled = checkbox(lui.ENABLE_JOYSTICK, joystickEnabled)
	if c then Input.SetJoystickEnabled(joystickEnabled) end

	for _,page in ipairs(binding_pages) do
		ui.separator()
		-- TODO: localizations for page IDs
		ui.withFont(pionillium.medium.name, bindingPageFontSize, function() ui.text(page.id) end)
		for _,group in ipairs(page) do
			if group.id then
				ui.separator()
				-- TODO: localizations for group IDs
				ui.withFont(pionillium.medium.name, bindingGroupFontSize, function() ui.text(group.id) end)
				for _,info in ipairs(group) do
					if info.type == 'action' then
						actionBinding(info)
					elseif info.type == 'axis' then
						axisBinding(info)
					end
				end
			end
		end
	end
end

local optionsTabs = {
	["video"]=showVideoOptions,
	["sound"]=showSoundOptions,
	["language"]=showLanguageOptions,
	["controls"]=showControlsOptions
}

local function optionsWindow()
	if ui.showOptionsWindow then
		ui.setNextWindowSize(optionsWinSize, 'Always')
		ui.setNextWindowPosCenter('Always')
		ui.withStyleColors({["WindowBg"] = Color(20, 20, 80, 230)}, function()
			-- TODO: this window should be ShowBorders
			ui.window("Options", {"NoTitleBar", "NoResize"}, function()
				mainButton(icons.view_sidereal, lui.VIDEO, showTab=='video', function()
					showTab = 'video'
				end)
				ui.sameLine()
				mainButton(icons.sound, lui.SOUND, showTab=='sound', function()
					showTab = 'sound'
				end)
				ui.sameLine()
				mainButton(icons.language, lui.LANGUAGE, showTab=='language', function()
					showTab = 'language'
				end)
				ui.sameLine()
				mainButton(icons.controls, lui.CONTROLS, showTab=='controls', function()
					showTab = 'controls'
				end)

				ui.separator()

				ui.child("options_tab", Vector(-1, optionsWinSize.y - mainButtonSize.y*3 - 4), function()
					optionsTabs[showTab]()
				end)

				ui.separator()
				optionTextButton(lui.OPEN_USER_FOLDER, nil, Engine.CanBrowseUserFolder, function()
					Engine.OpenBrowseUserFolder()
				end)

				ui.sameLine()
				optionTextButton(lui.CLOSE, nil, true, function()
					ui.showOptionsWindow = false
					if Game.player then
						Game.SetTimeAcceleration("1x")
						Input.EnableBindings();
					end
				end)

				if Game.player then
					ui.sameLine()
					optionTextButton(lui.SAVE, nil, true, function() ui.showSavedGameWindow = "SAVE" end)

					ui.sameLine()
					optionTextButton(lui.END_GAME, nil, true, function()
						ui.showOptionsWindow = false
						Input.EnableBindings();
						Game.EndGame()
					end)
				end
			end)
		end)
		if showKeyCapture then
			captureBinding(keyCaptureId, keyCaptureNum)
		end
	end
end

ui.registerModule("game", optionsWindow)
ui.registerModule("mainMenu", optionsWindow)

return {}
