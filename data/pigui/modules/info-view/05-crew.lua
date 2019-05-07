-- Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = import 'pigui/pigui.lua'
local InfoView = import 'pigui/views/info-view'
local Lang = import 'Lang'

local l = Lang.GetResource("ui-core")


InfoView.registerView("crew", {
    name = l.CREW_ROSTER,
    icon = ui.theme.icons.rooster,
    showView = false,
    draw = function()
    end
})
