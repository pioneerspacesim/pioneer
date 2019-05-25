-- Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = import 'pigui/pigui.lua'
local TabView = import 'pigui/views/tab-view.lua'

local stationView

if not stationView then
	stationView = TabView.New("space_station")

	ui.registerModule("game", function() stationView:renderTabView() end)
end

return stationView