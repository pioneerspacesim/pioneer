-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = require 'pigui'
local InfoView = require 'pigui.views.info-view'
local Lang = require 'Lang'

local l = Lang.GetResource("ui-core")


InfoView:registerView({
    id = "crew",
    name = l.CREW_ROSTER,
    icon = ui.theme.icons.rooster,
    showView = false,
    draw = function() end,
    refresh = function() end,
})
