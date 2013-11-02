-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = import("Game")
local Engine = import("Engine")
local Lang = import("Lang")
local utils = import("utils")
local TabGroup = import("ui/TabGroup")
local SmallLabeledButton = import("ui/SmallLabeledButton")
local KeyBindingCapture = import("UI.Game.KeyBindingCapture")
local AxisBindingCapture = import("UI.Game.AxisBindingCapture")

local ui = Engine.ui
local l = Lang.GetResource("ui-core");

local optionCheckBox = function (getter, setter, caption)
	local cb = ui:CheckBox()
	local initial = getter()
	cb:SetState(initial)
	cb.onClick:Connect(function () setter(cb.isChecked); end)
	return ui:HBox(5):PackEnd({cb, ui:Label(caption)})
end

local optionListOrDropDown = function (widget, getter, setter, settingCaption, captions, values)
	local list = ui[widget](ui)
	local initial_value = getter()
	local initial_index
	for i = 1, #values do
		list:AddOption(captions[i])
		if initial_value == values[i] then
			initial_index = i
		end
	end
	initial_index = initial_index or 1
	list:SetSelectedIndex(initial_index)
	list.onOptionSelected:Connect(function ()
		setter(values[list.selectedIndex])
	end)
	return ui:VBox(5):PackEnd({ui:Label(settingCaption), list})
end

local optionDropDown = function (getter, setter, settingCaption, captions, values)
	return optionListOrDropDown('DropDown', getter, setter, settingCaption, captions, values)
end

local optionList = function (getter, setter, settingCaption, captions, values)
	return optionListOrDropDown('List', getter, setter, settingCaption, captions, values)
end

ui.templates.Settings = function (args)
	local videoTemplate = function()
		local videoModes = Engine.GetVideoModeList()
		local videoModeLabels = {}
		local videoModeValues = {}
		for i = 1, #videoModes do
			local mode = videoModes[i]
			local token = mode.width .. 'x' .. mode.height
			videoModeLabels[i] = token
			videoModeValues[token] = mode
		end
		local GetVideoMode = function ()
			local w, h = Engine.GetVideoResolution()
			return w .. 'x' .. h -- return a string token (matches the tokens used as keys in videoModeValues)
		end
		local SetVideoMode = function (token)
			local mode = videoModeValues[token]
			Engine.SetVideoResolution(mode.width, mode.height)
		end
		local modeDropDown = optionDropDown(GetVideoMode, SetVideoMode, l.VIDEO_RESOLUTION, videoModeLabels, videoModeLabels)

		local aaLabels = { l.OFF, "x2", "x4", "x8", "x16" }
		local aaModes = { 0, 2, 4, 8, 16 }
		local aaDropDown = optionDropDown(Engine.GetMultisampling, Engine.SetMultisampling, l.MULTISAMPLING, aaLabels, aaModes)

		local detailLevels = { 'VERY_LOW', 'LOW', 'MEDIUM', 'HIGH', 'VERY_HIGH' }
		local detailLabels = { l.VERY_LOW, l.LOW, l.MEDIUM, l.HIGH, l.VERY_HIGH }

		local planetDetailDropDown = optionDropDown(
			Engine.GetPlanetDetailLevel, Engine.SetPlanetDetailLevel,
			l.PLANET_DETAIL_DISTANCE, detailLabels, detailLevels)

		local planetTextureCheckBox = optionCheckBox(
			Engine.GetPlanetFractalColourEnabled, Engine.SetPlanetFractalColourEnabled,
			l.PLANET_TEXTURES)

		local fractalDetailDropDown = optionDropDown(
			Engine.GetFractalDetailLevel, Engine.SetFractalDetailLevel,
			l.FRACTAL_DETAIL, detailLabels, detailLevels)

		local cityDetailDropDown = optionDropDown(
			Engine.GetCityDetailLevel, Engine.SetCityDetailLevel,
			l.CITY_DETAIL_LEVEL, detailLabels, detailLevels)

		local navTunnelsCheckBox = optionCheckBox(
			Engine.GetDisplayNavTunnels, Engine.SetDisplayNavTunnels,
			l.DISPLAY_NAV_TUNNELS)

		local fullScreenCheckBox = optionCheckBox(
			Engine.GetFullscreen, Engine.SetFullscreen,
			l.FULL_SCREEN)
		local shadersCheckBox = optionCheckBox(
			Engine.GetShadersEnabled, Engine.SetShadersEnabled,
			l.USE_SHADERS)
		local compressionCheckBox = optionCheckBox(
			Engine.GetTextureCompressionEnabled, Engine.SetTextureCompressionEnabled,
			l.COMPRESS_TEXTURES)

		return ui:Grid({1,1}, 1)
			:SetCell(0,0, ui:Margin(5, 'ALL', ui:VBox(5):PackEnd({
				ui:Label(l.VIDEO_CONFIGURATION_RESTART_GAME_TO_APPLY),
				modeDropDown,
				aaDropDown,
				fullScreenCheckBox,
				shadersCheckBox,
				compressionCheckBox,
			})))
			:SetCell(1,0, ui:Margin(5, 'ALL', ui:VBox(5):PackEnd({
				planetDetailDropDown,
				planetTextureCheckBox,
				fractalDetailDropDown,
				cityDetailDropDown,
				navTunnelsCheckBox,
			})))
	end

	local soundTemplate = function()
		local volumeSlider = function (caption, getter, setter)
			local initial_value = getter()
			local slider = ui:HSlider()
			local label = ui:Label(caption .. " " .. math.floor(initial_value * 100))
			slider:SetValue(initial_value)
			slider.onValueChanged:Connect(function (new_value)
					label:SetText(caption .. " " .. math.floor(new_value * 100))
					setter(new_value)
				end)
			return ui:VBox():PackEnd({label, slider})
		end

		local muteBox = function(getter, setter)
			return optionCheckBox(getter, setter, l.MUTE)
		end

		return ui:VBox():PackEnd(ui:Grid({1,2,1}, 3)
			:SetCell(0,0,muteBox(Engine.GetMasterMuted, Engine.SetMasterMuted))
			:SetCell(1,0,volumeSlider(l.MASTER, Engine.GetMasterVolume, Engine.SetMasterVolume))
			:SetCell(0,1,muteBox(Engine.GetMusicMuted, Engine.SetMusicMuted))
			:SetCell(1,1,volumeSlider(l.MUSIC, Engine.GetMusicVolume, Engine.SetMusicVolume))
			:SetCell(0,2,muteBox(Engine.GetEffectsMuted, Engine.SetEffectsMuted))
			:SetCell(1,2,volumeSlider(l.EFFECTS, Engine.GetEffectsVolume, Engine.SetEffectsVolume)))
	end

	local languageTemplate = function()
		local langs = Lang.GetAvailableLanguages("core")
		local captions = utils.build_array(utils.map(function (k,v) return k,Lang.GetResource("core", v).LANG_NAME end, ipairs(langs)))
		return optionList(function () return Lang.currentLanguage end, Lang.SetCurrentLanguage, l.LANGUAGE_RESTART_GAME_TO_APPLY, captions, langs)
	end

	local captureDialog = function (captureWidget, label, onOk)
		local captureLabel = ui:Label(l.PRESS_A_KEY_OR_CONTROLLER_BUTTON)
		local capture = captureWidget.New(ui)
		local curBinding, curDescription

		capture:SetInnerWidget(captureLabel)
		capture.onCapture:Connect(function (binding)
			captureLabel:SetText(capture.bindingDescription or '')
			curBinding = capture.binding
			curDescription = capture.bindingDescription
		end)

		local okButton = ui:Button(ui:Label(l.OK):SetFont("HEADING_NORMAL"))
		okButton.onClick:Connect(function()
			print('Capture: ' .. (curBinding or 'nil') .. ' (' .. (curDescription or 'nil') .. ')')
			onOk(curBinding, curDescription)
			ui:DropLayer()
		end)

		local clearButton = ui:Button(ui:Label(l.CLEAR):SetFont("HEADING_NORMAL"))
		clearButton.onClick:Connect(function()
			curBinding = nil
			curDescription = nil
			captureLabel:SetText('')
		end)

		local cancelButton = ui:Button(ui:Label(l.CANCEL):SetFont("HEADING_NORMAL"))
		cancelButton.onClick:Connect(function()
			ui:DropLayer()
		end)

		local dialog =
			ui:ColorBackground(0,0,0,0.5,
				ui:Align("MIDDLE",
					ui:Background(
						ui:VBox(10)
							:PackEnd(ui:Label(l.CHANGE_BINDING):SetFont("HEADING_NORMAL"))
							:PackEnd(ui:Label(label))
							:PackEnd(ui:Align("MIDDLE", capture))
							:PackEnd(ui:HBox(5):PackEnd({okButton,clearButton,cancelButton}))
					)
				)
			)
		return dialog
	end

	local captureKeyDialog = function (label, onOk)
		return captureDialog(KeyBindingCapture, label, onOk)
	end

	local captureAxisDialog = function (label, onOk)
		return captureDialog(AxisBindingCapture, label, onOk)
	end

	local initKeyBindingControls = function (grid, row, info)
		local bindings = { info.binding1, info.binding2 }
		local descriptions = { info.bindingDescription1, info.bindingDescription2 }
		local buttons = { SmallLabeledButton.New(''), SmallLabeledButton.New('') }

		grid:SetCell(0, row, ui:Label(info.label))
		grid:SetCell(1, row, buttons[1])

		local updateUI = function ()
			buttons[1].label:SetText(descriptions[1] or '')
			buttons[2].label:SetText(descriptions[2] or '')
			-- the first button is always shown
			-- the second button is shown if there's already one binding
			if bindings[1] then
				grid:SetCell(2, row, buttons[2])
			else
				grid:ClearCell(2, row)
			end
		end

		local captureBinding = function (which)
			local dialog = captureKeyDialog(info.label, function (new_binding, new_binding_description)
				if new_binding then
					bindings[which] = new_binding
					descriptions[which] = new_binding_description
					buttons[which].label:SetText(new_binding_description)
				else
					bindings[which] = nil
					descriptions[which] = nil
					buttons[which].label:SetText('')
				end
				if bindings[2] and not bindings[1] then
					bindings[1] = bindings[2]
					descriptions[1] = descriptions[2]
					bindings[2], descriptions[2] = nil, nil
				end
				if bindings[1] == bindings[2] then
					bindings[2], descriptions[2] = nil, nil
				end
				Engine.SetKeyBinding(info.id, table.unpack(bindings))
				updateUI()
			end)
			ui:NewLayer(dialog)
		end

		buttons[1].button.onClick:Connect(function () captureBinding(1); end)
		buttons[2].button.onClick:Connect(function () captureBinding(2); end)
		updateUI()
	end

	local initAxisBindingControls = function (grid, row, info)
		local button = SmallLabeledButton.New(info.bindingDescription1 or '')

		grid:SetCell(0, row, ui:Label(info.label))
		grid:SetCell(1, row, button)
		grid:ClearCell(2, row)

		button.button.onClick:Connect(function ()
			local dialog = captureAxisDialog(info.label, function (new_binding, new_binding_description)
				Engine.SetKeyBinding(info.id, new_binding)
				button.label:SetText(new_binding_description)
			end)
			ui:NewLayer(dialog)
		end)
	end

	local controlsTemplate = function()
		local options = ui:Margin(10, 'LEFT', ui:VBox():PackEnd({
			optionCheckBox(Engine.GetMouseYInverted, Engine.SetMouseYInverted, l.INVERT_MOUSE_Y),
			optionCheckBox(Engine.GetJoystickEnabled, Engine.SetJoystickEnabled, l.ENABLE_JOYSTICK),
		}))

		local box = ui:VBox()
		box:PackEnd(ui:Label(l.CONTROL_OPTIONS):SetFont('HEADING_LARGE'))
		box:PackEnd(options)

		local pages = Engine.GetKeyBindings()
		for page_idx = 1, #pages do
			local page = pages[page_idx]
			box:PackEnd(ui:Label(page.label):SetFont('HEADING_LARGE'))
			for group_idx = 1, #page do
				local group = page[group_idx]
				box:PackEnd(ui:Margin(10, 'LEFT', ui:Label(group.label):SetFont('HEADING_NORMAL')))
				local grid = ui:Grid(3, #group)
				-- grid columns: Action, Binding 1, Binding 2
				for i = 1, #group do
					local info = group[i]
					if info.type == 'KEY' then
						initKeyBindingControls(grid, i - 1, info)
					elseif info.type == 'AXIS' then
						initAxisBindingControls(grid, i - 1, info)
					end
				end
				box:PackEnd(ui:Margin(30, 'LEFT', grid))
			end
		end
		return box
	end

	local function wrapWithScroller(template)
		return function (...)
			local inner = template(...)
			return ui:Expand():SetInnerWidget(ui:Scroller():SetInnerWidget(inner))
		end
	end

	local setTabs = nil
	setTabs = TabGroup.New()
	setTabs:AddTab({ id = "Video",    title = l.VIDEO,    icon = "VideoCamera", template = wrapWithScroller(videoTemplate)    })
	setTabs:AddTab({ id = "Sound",    title = l.SOUND,    icon = "Speaker",     template = wrapWithScroller(soundTemplate)    })
	setTabs:AddTab({ id = "Language", title = l.LANGUAGE, icon = "Globe1",      template = wrapWithScroller(languageTemplate) })
	setTabs:AddTab({ id = "Controls", title = l.CONTROLS, icon = "Gamepad",     template = wrapWithScroller(controlsTemplate) })

	local close_buttons = {}
	do
		local items = args.closeButtons
		for i = 1, #items do
			local btn = ui:Button():SetInnerWidget(ui:Label(items[i].text))
			btn.onClick:Connect(items[i].onClick)
			close_buttons[i] = btn
		end
	end

	if #close_buttons > 1 then
		close_buttons = ui:HBox(5):PackEnd(close_buttons)
	else
		close_buttons = close_buttons[1]
	end

	return ui:VBox():PackEnd({setTabs, ui:Margin(10, "ALL", close_buttons)})
end

ui.templates.SettingsInGame = function ()
	return ui.templates.Settings({
		closeButtons = {
			{ text = l.SAVE, onClick = function ()
				local settings_view = ui.layer.innerWidget
				ui:NewLayer(
					ui.templates.FileDialog({
						title        = l.SAVE,
						helpText     = l.SELECT_A_FILE_TO_SAVE_TO_OR_ENTER_A_NEW_FILENAME,
						path         = "savefiles",
						allowNewFile = true,
						selectLabel  = l.SAVE,
						onSelect     = function (filename)
							Game.SaveGame(filename)
							ui:DropLayer()
						end,
						onCancel    = function ()
							ui:DropLayer()
						end
					})
				)
			end },
			{ text = l.EXIT_THIS_GAME, onClick = Game.EndGame }
		}
	})
end
