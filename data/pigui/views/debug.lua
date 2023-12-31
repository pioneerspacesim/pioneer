-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = require 'pigui'

local debugView = {
	tabs = {},
	modules = {}
}

function debugView.registerModule(name, module)
	if type(module) == "table" then
		module = function(...) return module:draw(...) end
	end

	local index = debugView.modules[name] or #debugView.modules + 1
	debugView.modules[index] = module
	debugView.modules[name] = index
end

function debugView.registerTab(name, tab)
	if type(tab) == "table" then
		tab = function(...) return tab:draw(...) end
	end

	local index = debugView.tabs[name] or #debugView.tabs + 1
	debugView.tabs[index] = tab
	debugView.tabs[name] = index
end

function debugView.render(delta)
	for i, f in ipairs(debugView.modules) do
		f(delta)
	end
end

function debugView.drawTabs(delta)
	for i, f in ipairs(debugView.tabs) do
		f(delta)
	end
end

ui.registerHandler('debug', debugView.render)
ui.registerHandler('debug-tabs', debugView.drawTabs)

return debugView
