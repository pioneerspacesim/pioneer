-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = require 'pigui'
local TabView = require 'pigui.views.tab-view'

local infoView

if not infoView then
	infoView = TabView.New("info")

	ui.registerModule("game", function() infoView:renderTabView() end)
end

return infoView
