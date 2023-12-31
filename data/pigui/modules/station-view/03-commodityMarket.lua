-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Lang = require 'Lang'
local StationView = require 'pigui.views.station-view'
local CommodityWidget = require 'pigui.libs.commodity-market'

local ui = require 'pigui'
local l = Lang.GetResource("ui-core")

local commodityMarket = CommodityWidget.New("commodityMarket", false)

local function render()
	commodityMarket:Render()
end

StationView:registerView({
	id = "commodityMarket",
	name = l.COMMODITY_MARKET,
	icon = ui.theme.icons.market,
	showView = true,
	draw = render,
	refresh = function()
		commodityMarket:Refresh()
		commodityMarket.scrollReset = true
		commodityMarket.selectedItem = nil
	end,
	debugReload = function()
		package.reimport('pigui.libs.commodity-market')
		package.reimport()
	end
})
