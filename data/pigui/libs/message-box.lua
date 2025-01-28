-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = require 'pigui'
local ModalWindow = require 'pigui.libs.modal-win'
local Lang = require 'Lang'
local lui = Lang.GetResource("ui-core")
local font = ui.fonts.pionillium.body
local msgButtonWidth = font.size * 7

--
-- Interface: msgbox
--

local msgbox = {}

local function createBoxModal(msg, footer, footerWidth)
	ModalWindow.New('PopupMessageBox', function(self)

		local textSize = ui.calcTextSize(tostring(msg) .. "\n.")
		textSize.x = math.min(textSize.x, ui.screenWidth * 0.6)
		textSize.y = math.min(textSize.y, ui.screenHeight * 0.6)
		ui.child('PopupMessageBoxText', textSize, { 'HorizontalScrollbar' }, function ()
			ui.text(msg)
		end)

		-- centered footer
		local width = ui.getContentRegion().x
		if width > footerWidth then
			ui.dummy(Vector2((width - footerWidth) / 2, 0))
			ui.sameLine(0, 0)
		end
		footer(self)
	end,
	function (_, drawPopupFn)
		ui.setNextWindowPosCenter('Always')
		ui.withFont(font, function()
			ui.withStyleColors({ PopupBg = ui.theme.colors.modalBackground }, drawPopupFn)
		end)
	end):open()
end

--
-- Function: msgbox.OK
--
-- msgbox.OK(msg)
--
msgbox.OK = function(msg)
	createBoxModal(msg, function(self)
		if ui.button(lui.OK, Vector2(msgButtonWidth, 0)) then
			self:close()
		end
	end, msgButtonWidth)
end

--
-- Function: msgbox.OK_CANCEL
--
-- msgbox.OK_CANCEL(msg)
--
msgbox.OK_CANCEL = function(msg, callback)
	createBoxModal(msg, function(self)
		if ui.button(lui.OK, Vector2(msgButtonWidth, 0)) then
			if callback then
				callback()
			end
			self:close()
		end
		ui.sameLine()
		if ui.button(lui.CANCEL, Vector2(msgButtonWidth, 0)) then
			self:close()
		end
	end, msgButtonWidth * 2 + ui.getItemSpacing().x)
end

--
-- Function: msgbox.YES_NO
--
-- msgbox.YES_NO(msg)
--
msgbox.YES_NO = function(msg, callbacks)
	createBoxModal(msg, function(self)
		if ui.button(lui.YES, Vector2(msgButtonWidth, 0)) then
			if callbacks and callbacks.yes then
				callbacks.yes()
			end
			self:close()
		end
		ui.sameLine()
		if ui.button(lui.NO, Vector2(msgButtonWidth, 0)) then
			if callbacks and callbacks.no then
				callbacks.no()
			end
			self:close()
		end
	end, msgButtonWidth * 2 + ui.getItemSpacing().x)
end

--
-- Function: msgbox.custom
--
-- msgbox.custom(msg, buttons)
--
-- Show a message box with custom buttons and corresponding callbacks.
--
-- Parameters:
--
--   msg - string, message for message box
--   buttons - array of type:
--       label - string, button label
--       callback - function
--
msgbox.custom = function(msg, buttons)
	createBoxModal(msg, function(self)
		for i, button in ipairs(buttons) do

			if i ~= 1 then ui.sameLine() end

			if ui.button(button.label, Vector2(msgButtonWidth, 0)) then
				if button.callback then
					button.callback()
				end
				self:close()
			end
		end
	end, msgButtonWidth * #buttons + ui.getItemSpacing().x)
end

return msgbox
