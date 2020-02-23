-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Lang = import 'Lang'
local StationView = import 'pigui/views/station-view'
local CommodityWidget = import 'pigui/libs/commodity-market.lua'

local ui = import 'pigui/pigui.lua'
local pionillium = ui.fonts.pionillium
local orbiteer = ui.fonts.orbiteer
local l = Lang.GetResource("ui-core")

local commodityMarket = CommodityWidget.New("commodityMarket", false)
local resetSize = true

local function render()
	local size = ui.getContentRegion()
	size.y =  size.y - StationView.style.height

	commodityMarket:Render(size)
	StationView:shipSummary()
end

StationView:registerView({
	id = "commodityMarket",
	name = l.COMMODITY_MARKET,
	icon = ui.theme.icons.market,
	showView = true,
	draw = render,
	refresh = function()
		resetSize = true
		commodityMarket:Refresh()
		commodityMarket.scrollReset = true
	end,
})
