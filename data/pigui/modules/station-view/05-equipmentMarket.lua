-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Lang = require 'Lang'
local Game = require 'Game'
local StationView = require 'pigui.views.station-view'

local ui = require 'pigui'
local l = Lang.GetResource("ui-core")

local equipmentWidget = require 'pigui.libs.ship-equipment'.New("ShipInfo")

StationView:registerView({
	id = "equipmentMarketView",
	name = l.EQUIPMENT_MARKET,
	icon = ui.theme.icons.equipment,
	showView = true,
	draw = function()
		equipmentWidget:draw()
	end,
	refresh = function()
		equipmentWidget.ship    = Game.player
		equipmentWidget.station = Game.player:GetDockedWith()
		equipmentWidget:refresh()
	end,
	debugReload = function()
		equipmentWidget:debugReload()
		package.reimport()
	end
})
