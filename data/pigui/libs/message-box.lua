-- Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = require 'pigui'
local ModalWindow = require 'pigui.libs.modal-win'

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
		if ui.button("OK", Vector2(OK_BUTTON_WIDTH, 0)) then
			self:close()
		end
	end):open()
end

return msgbox
