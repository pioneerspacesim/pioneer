-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = require 'pigui'
local ModalWindow = require 'pigui.libs.modal-win'
local Lang = require 'Lang'
local lui = Lang.GetResource("ui-core")
local font = ui.fonts.pionillium.body
local msgButtonWidth = font.size * 7

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

msgbox.OK = function(msg)
	createBoxModal(msg, function(self)
		if ui.button(lui.OK, Vector2(msgButtonWidth, 0)) then
			self:close()
		end
	end, msgButtonWidth)
end

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

return msgbox
