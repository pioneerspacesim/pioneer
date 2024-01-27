-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Event = require 'Event'
local Input = require 'Input'
local Game = require 'Game'
local Lang = require 'Lang'
local Vector2 = _G.Vector2
local bindManager = require 'bind-manager'

local lc = Lang.GetResource("core")
local lui = Lang.GetResource("ui-core")
local linput = Lang.GetResource("input-core")

local ui = require 'pigui'
local ModalWindow = require 'pigui.libs.modal-win'

local colors = ui.theme.colors
local icons = ui.theme.icons
local pionillium = ui.fonts.pionillium

local mainButtonSize = ui.theme.styles.MainButtonSize
local mainButtonFramePadding = ui.theme.styles.MainButtonPadding
local optionButtonSize = ui.rescaleUI(Vector2(100, 32))
local bindingButtonSize = ui.rescaleUI(Vector2(142, 32))

local optionsWinSize = Vector2(ui.screenWidth * 0.4, ui.screenHeight * 0.6)

local showTab = 'video'

local binding_pages
local keyCaptureBind
local keyCaptureNum

local needBackgroundStarRefresh = false
local starDensity = Engine.GetAmountStars() * 100
local starFieldStarSizeFactor = Engine.GetStarFieldStarSizeFactor() * 100

local function combo(label, selected, items, tooltip)
	local color = colors.buttonBlue
	local changed, ret = 0, nil
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
	local variant = not enabled and ui.theme.buttonColors.disabled

	local button
	ui.withFont(pionillium.small, function()
		button = ui.button(label, bindingButtonSize, variant, tooltip)
	end)
	if button then
		callback(button)
	end
end

local function optionTextButton(label, tooltip, enabled, callback)
	local variant = not enabled and ui.theme.buttonColors.disabled

	local button
	ui.withFont(pionillium.medium, function()
		button = ui.button(label, optionButtonSize, variant, tooltip)
	end)
	if button then
		if enabled then
			callback(button)
		end
	end
end --mainButton

local function mainButton(icon, tooltip, selected, callback)
	local button = ui.mainMenuButton(icon, tooltip, selected)
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

	local textCompress = Engine.GetTextureCompressionEnabled()
	local gpuJobs = Engine.GetGpuJobsEnabled()
	local disableScreenshotInfo = Engine.GetDisableScreenshotInfo()

	-- Scattering is still an experimental feature
	local experimental = "[" .. lui.EXPERIMENTAL .. "] "
	local scatteringLabels = {
		lui.SCATTERING_OLD,
		experimental .. lui.RAYLEIGH_FAST,
		experimental .. lui.RAYLEIGH_ACCURATE
	}

	local realisticScattering = Engine.GetRealisticScattering()

	local cityDetail = keyOf(detailLabels,keyOf(detailLevels, Engine.GetCityDetailLevel()))-1
	local displayNavTunnels = Engine.GetDisplayNavTunnels()
	local displaySpeedLines = Engine.GetDisplaySpeedLines()
	local displayHudTrails = Engine.GetDisplayHudTrails()
	local enableCockpit = Engine.GetCockpitEnabled()
	local enableAutoSave = Engine.GetAutosaveEnabled()

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

	c,scattering = combo(lui.REALISTIC_SCATTERING, realisticScattering, scatteringLabels, lui.REALISTIC_SCATTERING_DESC)
	if c then
		Engine.SetRealisticScattering(scattering)
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
	ui.nextColumn()
	c,disableScreenshotInfo = checkbox(lui.DISABLE_SCREENSHOT_INFO, disableScreenshotInfo, lui.DISABLE_SCREENSHOT_INFO_DESC)
	if c then
		Engine.SetDisableScreenshotInfo(disableScreenshotInfo)
	end
	ui.columns(1,"",false)


	ui.separator()
	c,selectedDetail = combo(lui.PLANET_DETAIL_DISTANCE, selectedDetail, detailLabels, lui.DETAIL_DESC)
	if c then
		local detail = detailLevels[detailLabels[selectedDetail+1]]
		Engine.SetPlanetDetailLevel(detail)
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
		needBackgroundStarRefresh = true
	end

	c,starFieldStarSizeFactor = slider(lui.STAR_FIELD_STAR_SIZE_FACTOR, starFieldStarSizeFactor, 0, 100)
	if c then
		-- TODO: lua somtimes gets very small slider changes, even though I didn't touch the slider
		needBackgroundStarRefresh = true
	end

	ui.separator()
	optionTextButton("Debug Info", nil, true, function()
		Engine.SetShowDebugInfo()
		ui.optionsWindow:close()
	end)
end

local captureBindingWindow
local bindState = nil -- state, to capture the key combination
captureBindingWindow = ModalWindow.New("CaptureBinding", function()
	local info = keyCaptureBind
	ui.text(bindManager.localizeBindingId(info.id))
	ui.text(lui.PRESS_A_KEY_OR_CONTROLLER_BUTTON)

	if info.type == 'Action' then
		local desc = keyCaptureNum == 1 and info.binding or info.binding2
		ui.text(desc.enabled and bindManager.getChordDesc(desc) or lc.NONE)

		local set, bindingKey = Engine.pigui.GetKeyBinding(bindState)
		if set then
			if keyCaptureNum == 1 then
				info.binding = bindingKey
			else
				info.binding2 = bindingKey
			end
		end
		bindState = bindingKey
	elseif info.type == 'Axis' then
		local desc
		if keyCaptureNum == 1 then
			desc = bindManager.getBindingDesc(info.axis) or lc.NONE
		else
			desc = keyCaptureNum == 2 and info.positive or info.negative
			desc = desc.enabled and bindManager.getChordDesc(desc) or lc.NONE
		end
		ui.text(desc)

		if keyCaptureNum == 1 then
			local set, bindingAxis = Engine.pigui.GetAxisBinding()
			if set then
				info.axis = bindingAxis
			end
		else
			local set, bindingKey = Engine.pigui.GetKeyBinding(bindState)
			if set then
				if keyCaptureNum == 2 then
					info.positive = bindingKey
				else
					info.negative = bindingKey
				end
			end
			bindState = bindingKey
		end
	end

	optionTextButton(lui.OK, nil, true, function()
		Input.SaveBinding(info)
		bindManager.updateBinding(info.id)
		captureBindingWindow:close()
	end)
end, function (_, drawPopupFn)
	ui.setNextWindowPosCenter('Always')
	ui.withStyleColors({ PopupBg = ui.theme.colors.modalBackground }, drawPopupFn)
end)

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

	ui.withFont(pionillium.large, function()
		ui.text(lui.LANGUAGE_RESTART_GAME_TO_APPLY)
	end)

	local clicked
	for _,lang in pairs(langs) do
		ui.withFont(pionillium.large, function()
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
	local descs = {
		bindManager.getChordDesc(info.binding),
		bindManager.getChordDesc(info.binding2)
	}

	if (ui.collapsingHeader(bindManager.localizeBindingId(info.id), {})) then
		ui.columns(3,"##bindings",false)
		ui.nextColumn()
		ui.text(linput.TEXT_BINDING)
		bindingTextButton((descs[1] or '')..'##'..info.id..'1', (descs[1] or ''), true, function()
			keyCaptureBind = info
			keyCaptureNum = 1
			captureBindingWindow:open()
		end)
		ui.nextColumn()
		ui.text(linput.TEXT_ALT_BINDING)
		bindingTextButton((descs[2] or '')..'##'..info.id..'2', (descs[2] or ''), true, function()
			keyCaptureBind = info
			keyCaptureNum = 2
			captureBindingWindow:open()
		end)
		ui.columns(1,"",false)
	end
end

local function axisBinding(info)
	local axis, positive, negative = info.axis, info.positive, info.negative
	local descs = { bindManager.getBindingDesc(axis), bindManager.getChordDesc(positive), bindManager.getChordDesc(negative) }

	if (ui.collapsingHeader(bindManager.localizeBindingId(info.id), {})) then
		ui.columns(3,"##axisjoybindings",false)
		ui.text("Axis:")
		ui.nextColumn()
		bindingTextButton((descs[1] or '')..'##'..info.id..'axis', (descs[1] or ''), true, function()
			keyCaptureBind = info
			keyCaptureNum = 1
			captureBindingWindow:open()
		end)
		ui.nextColumn()
		if axis then
			local c, inverted = nil, axis.direction < 0
			c,inverted = ui.checkbox("Inverted##"..info.id, inverted, linput.TEXT_INVERT_AXIS)
			if c then
				axis.direction = inverted and -1 or 1
				info.axis = axis; Input.SaveBinding(info)
			end
		end
		-- new row
		ui.nextColumn()
		ui.text(linput.TEXT_KEY_BINDINGS)
		ui.nextColumn()
		ui.text(linput.TEXT_KEY_POSITIVE)
		bindingTextButton((descs[2] or '')..'##'..info.id..'positive', (descs[2] or ''), true, function()
			keyCaptureBind = info
			keyCaptureNum = 2
			captureBindingWindow:open()
		end)
		ui.nextColumn()
		ui.text(linput.TEXT_KEY_NEGATIVE)
		bindingTextButton((descs[3] or '')..'##'..info.id..'negative', (descs[3] or ''), true, function()
			keyCaptureBind = info
			keyCaptureNum = 3
			captureBindingWindow:open()
		end)
		ui.columns(1,"",false)
	end
end

local function drawJoystickAxisInfo(joystick, i)
	local c, val, enabled

	ui.columns(2, "axis info")

	ui.text(linput.CURRENT_VALUE)
	ui.sameLine()
	ui.text(string.format("%0.2f", joystick:GetAxisValue(i)))

	ui.nextColumn()

	c, enabled = ui.checkbox(linput.HALF_AXIS_MODE, joystick:GetAxisZeroToOne(i))
	if c then
		joystick:SetAxisZeroToOne(i, enabled)
	end

	ui.nextColumn()

	val, c = ui.sliderFloat(linput.DEADZONE, joystick:GetAxisDeadzone(i), 0.0, 1.0, "%0.2f")
	if c then joystick:SetAxisDeadzone(i, val) end

	ui.nextColumn()

	val, c = ui.sliderFloat(linput.CURVE, joystick:GetAxisCurve(i), 0.0, 2.0, "%0.2f")
	if c then joystick:SetAxisCurve(i, val) end

	ui.columns(1, "")
end

local selectedJoystick = nil
local function showJoystickInfo(id)
	local joystick = Input.GetJoystick(id)

	ui.withFont(pionillium.heading, function()
		local buttonSize = Vector2(ui.getTextLineHeightWithSpacing())

		if ui.iconButton(icons.time_backward_1x, buttonSize, lui.GO_BACK .. "##" .. id) then
			Input.SaveJoystickConfig(selectedJoystick)
			selectedJoystick = nil
		end

		ui.sameLine()
		ui.alignTextToLineHeight()
		ui.text(joystick.name)
	end)

	ui.spacing()

	ui.text(linput.NUM_BUTTONS)
	ui.sameLine()
	ui.text(joystick.numButtons)

	ui.text(linput.NUM_HATS)
	ui.sameLine()
	ui.text(joystick.numHats)

	ui.text(linput.NUM_AXES)
	ui.sameLine()
	ui.text(joystick.numAxes)

	ui.spacing()

	for i = 0, joystick.numAxes - 1 do

		local width = ui.getContentRegion().x * 0.5
		local open = ui.collapsingHeader(bindManager.getAxisName(i))

		local isHalfAxis = joystick:GetAxisZeroToOne(i)
		local value = joystick:GetAxisValue(i)

		-- Draw axis preview indicator
		ui.sameLine(width)

		local pos = ui.getCursorScreenPos()
		pos.y = pos.y + ui.getItemSpacing().y * 0.5

		local size = Vector2(width - ui.getItemSpacing().x, ui.getTextLineHeight())
		ui.addRectFilled(pos, pos + size, colors.darkGrey, 0, 0)

		if isHalfAxis then
			size.x = size.x * value
		else
			pos.x = pos.x + size.x * 0.5
			size.x = size.x * 0.5 * value
		end

		ui.addRectFilled(pos, pos + size, colors.primary, 0, 0)
		ui.newLine()

		-- Draw axis details
		if open then
			ui.withID(i, function()
				drawJoystickAxisInfo(joystick, i)
			end)
		end

		ui.spacing()

	end

end

local function showJoystickList(id)
	local connected = Input.IsJoystickConnected(id)
	local buttonSize = Vector2(ui.getTextLineHeightWithSpacing())

	if connected then
		if ui.iconButton(icons.pencil, buttonSize, lui.EDIT .. "##" .. id) then
			selectedJoystick = id
		end
	else
		ui.dummy(buttonSize)
	end

	ui.sameLine()
	ui.alignTextToLineHeight()
	ui.text(bindManager.joyAcronym(id) .. ":")

	local status = connected and linput.CONNECTED or linput.NOT_CONNECTED
	ui.sameLine()
	ui.textColored(ui.theme.colors.grey, Input.GetJoystickName(id) .. ", " .. status)
end

local function showControlsOptions()
	ui.text(lui.CONTROL_OPTIONS)

	local mouseYInvert = Input.GetMouseYInverted()
	local joystickEnabled = Input.GetJoystickEnabled()
	binding_pages = Input.GetBindingPages()
	local c

	c,mouseYInvert = checkbox(lui.INVERT_MOUSE_Y, mouseYInvert)
	if c then Input.SetMouseYInverted(mouseYInvert) end

	c,joystickEnabled = checkbox(lui.ENABLE_JOYSTICK, joystickEnabled)
	if c then Input.SetJoystickEnabled(joystickEnabled) end

	-- list all the joysticks
	local joystick_count = Input.GetJoystickCount()
	if joystick_count > 0 then
		ui.separator()
		ui.text(linput.JOYSTICKS .. ":")
		ui.spacing()

		ui.withFont(pionillium.body, function()
			if selectedJoystick then
				showJoystickInfo(selectedJoystick)
			else
				for id = 0, joystick_count - 1 do
					showJoystickList(id)
				end
			end
		end)
	end

	for _,page in ipairs(binding_pages) do
		ui.text ''
		ui.withFont(pionillium.medium, function()
			ui.text(bindManager.localizeBindingId("Page" .. page.id))
		end)
		ui.separator()
		Engine.pigui.PushID(page.id)
		for _,group in ipairs(page) do
			if group.id then
				Engine.pigui.PushID(group.id)
				if _ > 1 then ui.text '' end
				ui.withFont(pionillium.medium, function()
					ui.text(bindManager.localizeBindingId("Group" .. group.id))
				end)
				ui.separator()
				for _,binding in ipairs(group) do
					if binding.type == 'Action' then
						actionBinding(binding)
					elseif binding.type == 'Axis' then
						axisBinding(binding)
					end
				end
				Engine.pigui.PopID()
			end
		end
		Engine.pigui.PopID()
	end
end

local optionsTabs = {
	["video"]=showVideoOptions,
	["sound"]=showSoundOptions,
	["language"]=showLanguageOptions,
	["controls"]=showControlsOptions
}

ui.optionsWindow = ModalWindow.New("Options", function()
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

	-- I count the separator as two item spacings
	local other_height = mainButtonSize.y + mainButtonFramePadding * 2 + optionButtonSize.y  + ui.getItemSpacing().y * 4 + ui.getWindowPadding().y * 2
	ui.child("options_tab", Vector2(-1, optionsWinSize.y - other_height), function()
		optionsTabs[showTab]()
	end)

	ui.separator()
	optionTextButton(lui.OPEN_USER_FOLDER, nil, Engine.CanBrowseUserFolder, function()
		Engine.OpenBrowseUserFolder()
	end)

	ui.sameLine()
	optionTextButton(lui.CLOSE, nil, true, function()
		ui.optionsWindow:close()
		if needBackgroundStarRefresh then
			Engine.SetAmountStars(starDensity/100)
			Engine.SetStarFieldStarSizeFactor(starFieldStarSizeFactor/100)
			needBackgroundStarRefresh = false
		end
		if selectedJoystick then
			Input.SaveJoystickConfig(selectedJoystick)
		end
	end)

	if Game.player then
		ui.sameLine()
		optionTextButton(lui.SAVE, nil, Game.player.flightState ~= 'HYPERSPACE', function()
			ui.saveLoadWindow.mode = "SAVE"
			ui.saveLoadWindow:open()
		end)

		ui.sameLine()
		optionTextButton(lui.END_GAME, nil, true, function()
			ui.optionsWindow:close()
			Game.EndGame()
		end)
	end
end, function (_, drawPopupFn)
	ui.setNextWindowSize(optionsWinSize, 'Always')
	ui.setNextWindowPosCenter('Always')
	ui.withStyleColors({ PopupBg = ui.theme.colors.modalBackground }, drawPopupFn)
end)

function ui.optionsWindow:changeState()
    if not self.isOpen then
		self:open()
	else
		self:close()
	end
end

function ui.optionsWindow:open()
	ModalWindow.open(self)
	if Game.player then
		Input.EnableBindings(false)
		Event.Queue("onPauseMenuOpen")
	end
end

function ui.optionsWindow:close()
	if not captureBindingWindow.isOpen then
		ModalWindow.close(self)
		if Game.player then
			Game.SetTimeAcceleration("1x")
			Event.Queue("onPauseMenuClosed")
		end
	end
end

return {}
