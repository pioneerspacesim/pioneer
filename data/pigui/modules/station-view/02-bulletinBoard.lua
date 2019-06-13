-- Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = import 'pigui/pigui.lua'
local StationView = import 'pigui/views/station-view'
local Lang = import 'Lang'

local l = Lang.GetResource("ui-core")

StationView:registerView({
	id = "bulletinBoard",
	name = l.BULLETIN_BOARD,
	icon = ui.theme.icons.bbs,
	showView = false,
	draw = function() end,
	refresh = function() end,
})
