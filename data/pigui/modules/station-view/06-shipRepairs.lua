-- Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = import 'pigui/pigui.lua'
local StationView = import 'pigui/views/station-view'
local Lang = import 'Lang'

local l = Lang.GetResource("ui-core")

StationView:registerView({
	id = "shipRepairs",
	name = l.SHIP_REPAIRS,
	icon = ui.theme.icons.repairs,
	showView = false,
	draw = function() end,
	refresh = function() end,
})
