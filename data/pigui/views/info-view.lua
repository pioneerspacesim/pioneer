-- Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local ui = require 'pigui'
local TabView = require 'pigui.views.tab-view'

local infoView = TabView.New("InfoView")
infoView.windowPadding = ui.rescaleUI(Vector2(18, 18))

ui.registerHandler("InfoView", function(delta_t, refresh)
	infoView:renderTabView(refresh)

	if ui.escapeKeyReleased() then
		Game.SetView("WorldView")
	end
end)

return infoView
