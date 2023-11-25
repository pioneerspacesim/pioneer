-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = require 'pigui'
local ModalWindow = require 'pigui.libs.modal-win'
local Lang = require 'Lang'
local lui = Lang.GetResource("ui-core")

local OK_BUTTON_WIDTH = 100

local msgbox = {}

msgbox.OK = function(msg)
	ModalWindow.New('PopupMessageBox', function(self)
		ui.text(msg)
		local width = ui.getContentRegion().x
		if width > OK_BUTTON_WIDTH then
			ui.dummy(Vector2((width - OK_BUTTON_WIDTH) / 2, 0))
			ui.sameLine()
		end
		if ui.button(lui.OK, Vector2(OK_BUTTON_WIDTH, 0)) then
			self:close()
		end
	end,
	function (_, drawPopupFn)
		ui.setNextWindowPosCenter('Always')
		ui.withStyleColors({ PopupBg = ui.theme.colors.modalBackground }, drawPopupFn)
	end):open()
end

msgbox.OK_CANCEL = function(msg, callback)
	ModalWindow.New('PopupMessageBox', function(self)
		ui.text(msg)
		local width = ui.getContentRegion().x
		local okButton = ui.button(lui.OK, Vector2(OK_BUTTON_WIDTH, 0))
		if okButton then
			if callback then
				callback(okButton)
			end
			self:close()
		end
		ui.sameLine(width - OK_BUTTON_WIDTH, 0)
		if ui.button(lui.CANCEL, Vector2(OK_BUTTON_WIDTH, 0)) then
			self:close()
		end
	end,
	function (_, drawPopupFn)
		ui.setNextWindowPosCenter('Always')
		ui.withStyleColors({ PopupBg = ui.theme.colors.modalBackground }, drawPopupFn)
	end):open()
end

return msgbox
