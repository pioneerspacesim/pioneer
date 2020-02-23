-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = require 'pigui'
local InfoView = require 'pigui.views.info-view'
local Lang = require 'Lang'
local Debug = require 'pigui.libs.debug'

local l = Lang.GetResource("ui-core")

local view
view = {
    id = "missions",
    name = l.MISSIONS,
    icon = ui.theme.icons.star,
    showView = false,
    draw = function()
        Debug:render()
    end,
    refresh = function()
        Debug = require 'pigui.libs.debug'
        view.showView = Debug.showView
        Debug.scrollPos = 0
    end,
}

InfoView:registerView(view)
