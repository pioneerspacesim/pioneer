-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

---@class ui
local ui = require 'pigui'

local timers = {}

function ui.createTimer(name, endTime, callback)
	local timer = {}
	timer.endTime = ui.getTime() + endTime
	timer.callback = callback
	timers[name] = timer
end

function ui.deleteTimer(name)
	local timer = timers[name]
	if not timer then return end
	if timer.callback then
		timer.callback()
	end
	timers[name] = nil
end

local function updateTimers()
	for name, timer in pairs(timers) do
		if ui.getTime() > timer.endTime then
			ui.deleteTimer(name)
		end
	end
end

ui.registerModule('ui-timer', function()
	updateTimers()
end)

return ui
