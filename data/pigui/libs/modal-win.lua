-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = require 'pigui'

local modalStack = {}

local ModalWindow = {}
local defaultModalFlags = ui.WindowFlags {"NoTitleBar", "NoResize", "AlwaysAutoResize", "NoMove"}

function ModalWindow.New(name, innerHandler, outerHandler, flags)
	local modalWin = {
		name = name,
		flags = flags or defaultModalFlags,
		stackIdx = -1,
		isOpen = false,
		innerHandler = innerHandler,
		outerHandler = outerHandler or function(_, drawPopupFn)
			drawPopupFn()
		end,
	}

	setmetatable(modalWin, {
		__index = ModalWindow,
		class = "UI.ModalWindow",
	})

	return modalWin
end

function ModalWindow:open()
	if self.stackIdx < 0 then
		table.insert(modalStack, self)
		self.stackIdx = #modalStack
	end
end

function ModalWindow:close()
	for i=#modalStack, self.stackIdx, -1 do
		modalStack[i].stackIdx = -1
		modalStack[i].isOpen = false
		ui.closeCurrentPopup()
		table.remove(modalStack, i)
	end
end

local function drawModals(idx)
	if idx <= #modalStack then
		local win = modalStack[idx]

		if (not win.isOpen) then
			win.isOpen = true
			ui.openPopup(win.name)
		end

		win:outerHandler(function ()
			if ui.beginPopupModal(win.name, win.flags) then
				win:innerHandler()
				-- modal could close in handler
				if win.isOpen then
					drawModals(idx+1)
				end
				ui.endPopup()
			end
		end)
	end
end

ui.registerModule('modal', function()
	drawModals(1)
end)

return ModalWindow
