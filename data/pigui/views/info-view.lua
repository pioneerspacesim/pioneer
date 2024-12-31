-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local ui = require 'pigui'
local TabView = require 'pigui.views.tab-view'

local infoView = TabView.New("InfoView")
infoView.windowPadding = ui.rescaleUI(Vector2(18, 18))

ui.registerModule("game", function()
	infoView:renderTabView()
	if infoView.isActive and ui.escapeKeyReleased() then
		Game.SetView("WorldView")
	end
end)

return infoView
