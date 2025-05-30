-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Event = require 'Event'
local Input = require 'Input'
local Game = require 'Game'
local Lang = require 'Lang'
local PiImage = require 'pigui.libs.image'
local Vector2 = _G.Vector2
local bindManager = require 'bind-manager'

local lc = Lang.GetResource("core")
local lui = Lang.GetResource("ui-core")

local ui = require 'pigui'
local ModalWindow = require 'pigui.libs.modal-win'

local colors = ui.theme.colors
local icons = ui.theme.icons
local pionillium = ui.fonts.pionillium

local mainButtonSize = ui.theme.styles.MainButtonSize
local optionButtonSize = ui.rescaleUI(Vector2(100, 32))

local optionsWinSize = Vector2(ui.screenWidth * 0.4, ui.screenHeight * 0.6)

local showTab = 'info'

local keyCaptureBind
local keyCaptureNum


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

local function showInfo()
	ui.withFont(pionillium.heading, function()
		ui.textWrapped("Pioneer is a single player space adventure game set in the Milky Way galaxy at the turn of the 31st century")
	end)

	ui.withFont(pionillium.body, function()
		ui.textWrapped("The game is open-ended, and you are free to explore the millions of star systems in the game. You can land on planets, slingshot past gas giants, and burn yourself to a crisp flying between binary star systems. You can try your hand at piracy, make your fortune trading between systems, or do missions for the various factions fighting for power, freedom or self-determination.")
		ui.spacing()
		ui.inputText("Version", Engine.version , { "ReadOnly" })
		ui.text("Development is continous. Stable version is released annually, February 3: ")
		ui.spacing()

		ui.text("Homepage: http://pioneerspacesim.net/")
		ui.text("Bug tracker: https://github.com/pioneerspacesim/pioneer/issues/")
		ui.text("Player Forum: http://spacesimcentral.com/community/pioneer/")
		ui.text("Develper Forum: https://forum.pioneerspacesim.net/")
		ui.text("Community discord: https://discord.gg/Xzwh5ZPFcK/")
		ui.text("Developer IRC: #pioneer @ libera.chat: https://web.libera.chat/#pioneer")

		ui.separator()
		ui.text("Copyright © 2008-2025 Pioneer Developers")
	end)
end

local function showHelp()
	ui.withFont(pionillium.heading, function()
		ui.text("Documentation:")
	end)

	ui.withFont(pionillium.body, function()
					ui.text("Manual can be found at: https://wiki.pioneerspacesim.net/wiki/Manual")
					ui.text("Basic flight: https://wiki.pioneerspacesim.net/wiki/Basic_flight")
					ui.text("Keyboard and mouse control is found at: https://wiki.pioneerspacesim.net/wiki/Keyboard_and_mouse_controls")
					ui.text("FAQ: https://wiki.pioneerspacesim.net/wiki/FAQ")
					ui.text("Contributions: https://wiki.pioneerspacesim.net/wiki/How_you_can_contribute")
					ui.text("Translations: https://wiki.pioneerspacesim.net/wiki/Translations")
	end)
end

local function showDonateInfo()
	ui.withFont(pionillium.heading, function()
		ui.text("Support Development")
	end)

	local qr = PiImage.New("icons/donate-qr.png")

	ui.withFont(pionillium.body, function()
		ui.textWrapped("So if you enjoy playing Pioneer and would like to support its continued development, please consider donating, no donation is too small.")
		ui.spacing()
		ui.text("PayPal: https://www.paypal.com/donate/?hosted_button_id=UPNFQCFJ9WSY6")
		ui.spacing()
		qr:Draw(Vector2(128, 128))

		ui.spacing()
		ui.text("Crypto:")
		ui.inputText("BTC", "bc1q38e55z0agw0xrjngvp4qanap8ntuc6v7dpdsc4" , { "ReadOnly" })
		ui.inputText("ETH", "0x0FD599C29e2a4E43f3B300c26DAD00995AB171F8" , { "ReadOnly" })

	end)

	-- local contentRegion = ui.getContentRegion())

	ui.spacing()
	ui.text("Thank you all for your continued support of Pioneer!")
end

local function showGPL()
	ui.withFont(pionillium.heading, function()
		ui.text("Licensed under the terms of the GPL v3")
	end)

	ui.inputText("##gpl", "<LICENCE TEXT HERE>" , { "ReadOnly" })
end

local function showSoftwareUsed()
	ui.withFont(pionillium.heading, function()
		ui.text("Third party software:")
	end)

	ui.withFont(pionillium.body, function()
					ui.text("argh")
					ui.text("atomic_queue")
					ui.text("base64")
					ui.text("doctest")
					ui.text("fmt")
					ui.text("glew")
					ui.text("imgui")
					ui.text("jenkins")
					ui.text("json")
					ui.text("lua")
					ui.text("lz4")
					ui.text("miniz")
					ui.text("nanosockets")
					ui.text("nanosvg")
					ui.text("pcg-cpp")
					ui.text("PicoDDS")
					ui.text("portable-file-dialogs")
					ui.text("profiler")
					ui.text("vcacheopt")
	end)
end


local optionsTabs = {
	info=showInfo,  -- i icon, version, bug tracker
	help=showHelp,     -- question mark icon, link wiki, forum, discord
	donate=showDonateInfo,
	license=showGPL,       -- text icon / fountain pen
	software=showSoftwareUsed,
}

ui.aboutWindow = ModalWindow.New("Options", function()
	ui.horizontalGroup(function()

		mainButton(icons.info, "INFO", showTab=='info', function() -- xxx lui.INFO
			showTab = 'info'
		end)

		mainButton(icons.bookmark, "HELP", showTab=='help', function() -- xxx lui.HELP
			showTab = 'help'
		end)

		mainButton(icons.money, "DONATE", showTab=='donate', function() -- xxx lui.DONATE
			showTab = 'donate'
		end)

		mainButton(icons.scales, "LICENCE", showTab=='license', function() -- xxx lui.LICENCE
			showTab = 'license'
		end)

		mainButton(icons.planet_grid, "SOFTWARE", showTab=='software', function() -- xxx lui.LICENCE
			showTab = 'software'
		end)

	end)

	ui.separator()

	-- I count the separator as two item spacings
	local other_height = mainButtonSize.y + optionButtonSize.y  + ui.getItemSpacing().y * 4 + ui.getWindowPadding().y * 2
	ui.child("options_tab", Vector2(-1, optionsWinSize.y - other_height), function()
		optionsTabs[showTab]()
	end)

	ui.separator()
	optionTextButton(lui.CLOSE, nil, true, function()
		ui.aboutWindow:close()
	end)

	if Game.player then
		ui.sameLine()
		optionTextButton(lui.SAVE, nil, Game.player.flightState ~= 'HYPERSPACE', function()
			ui.saveLoadWindow.mode = "SAVE"
			ui.saveLoadWindow:open()
		end)

		ui.sameLine()
		optionTextButton(lui.END_GAME, nil, true, function()
			ui.aboutWindow:close()
			Game.EndGame()
		end)
	end
end, function (_, drawPopupFn)
	ui.setNextWindowSize(optionsWinSize, 'Always')
	ui.setNextWindowPosCenter('Always')
	ui.withStyleColors({ PopupBg = ui.theme.colors.modalBackground }, drawPopupFn)
end)

function ui.aboutWindow:changeState()
	if not self.isOpen then
		self:open()
	else
		self:close()
	end
end

function ui.aboutWindow:open()
	ModalWindow.open(self)
	if Game.player then
		Input.EnableBindings(false)
		Event.Queue("onPauseMenuOpen")
	end
end

function ui.aboutWindow:close()
	if not captureBindingWindow.isOpen then
		ModalWindow.close(self)
		if Game.player then
			Game.SetTimeAcceleration("1x")
			Event.Queue("onPauseMenuClosed")
		end
	end
end

return {}
