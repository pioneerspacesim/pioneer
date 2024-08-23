-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = require 'pigui'

local Module = require 'pigui.libs.module'
local utils = require 'utils'

local modalStack = {}

---@class UI.ModalWindow : UI.Module
local ModalWindow = utils.class("UI.ModalWindow", Module)

local defaultModalFlags = ui.WindowFlags {"NoTitleBar", "NoResize", "AlwaysAutoResize", "NoMove"}

function ModalWindow.New(name, render, outerHandler, flags)
	local modalWin = {
		name = name,
		flags = flags or defaultModalFlags,
		stackIdx = -1,
		isOpen = false,
		render = render,
		outerHandler = outerHandler,
	}

	setmetatable(modalWin, {
		__index = ModalWindow,
		class = "UI.ModalWindow",
	})

	Module.Constructor(modalWin)

	return modalWin
end

function ModalWindow:open(...)
	if self.stackIdx < 0 then
		table.insert(modalStack, self)
		self.stackIdx = #modalStack
	end

	self:message("onOpen", ...)
end

function ModalWindow:close(...)
	for i=#modalStack, self.stackIdx, -1 do
		modalStack[i].stackIdx = -1
		modalStack[i].isOpen = false
		table.remove(modalStack, i)
	end

	self:message("onClose", ...)
end

function ModalWindow:onOpen() end

function ModalWindow:onClose() end

function ModalWindow:outerHandler(innerFn)
	innerFn()
end

local function drawModals(idx)
	if idx <= #modalStack then
		local win = modalStack[idx]

		if (not win.isOpen) then
			win.isOpen = true
			ui.openPopup(win.name)
		end

		win:update()

		win:outerHandler(function ()
			if ui.beginPopupModal(win.name, win.flags) then
				win:render()
				-- modal could close in handler
				if win.isOpen then
					drawModals(idx+1)
				else
					ui.closeCurrentPopup()
				end
				ui.endPopup()
			end
		end)
	end

	if idx == #modalStack + 1 then
		for _,v in ipairs(ui.getModules('notification')) do
			v.draw()
		end
	end
end

ui.registerModule('modal', function()
	drawModals(1)
end)

return ModalWindow
