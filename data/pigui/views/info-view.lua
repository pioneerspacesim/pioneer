-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = import 'pigui/pigui.lua'
local TabView = import 'pigui/views/tab-view.lua'

local infoView

if not infoView then
	infoView = TabView.New("info")

	ui.registerModule("game", function() infoView:renderTabView() end)
end

return infoView
