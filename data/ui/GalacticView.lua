-- Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Game = import("Game")
local Lang = import("Lang")
local GalaxyMap = import("UI.Game.GalaxyMap")

local ui = Engine.ui
local l = Lang.GetResource("ui-core");

local zoomSlider = ui:VSlider()
local map = GalaxyMap.New(ui):SetFont('LARGE')
local scaleIndicator =
		ui:Image("icons/map_scale_indicator.png", { "PRESERVE_ASPECT" })
local scaleLabel = ui:NumberLabel('DISTANCE_LY')

map.onMapScaleChanged:Connect(function (new_scale)
	scaleLabel:SetValue(scaleIndicator.width * new_scale * 8)
end)

local resetCurrentSector = function (map)
	if not Game.system then return end
	local sysPath = Game.system.path
	local x = sysPath.sectorX
	local y = sysPath.sectorY

	map:ClearLabels()
	map:SetCentreSector(x, y)
	map
		:AddAreaLabel(-3250,  1875, l.THREE_KPC_ARM)
		:AddAreaLabel(-3900,  2812, l.NORMA_ARM)
		:AddAreaLabel( 1500,  1200, l.PERSEUS_ARM)
		:AddAreaLabel(-2600, -5100, l.OUTER_ARM)
		:AddAreaLabel(-4100,  4375, l.SAGITTARIUS_ARM)
		:AddAreaLabel(-4300,  3450, l.SCUTUM_CENTAURUS_ARM)
		:AddAreaLabel(  100, -1100, l.LOCAL_ARM)
		:AddPointLabel(   x,     y, Game.system.name)
end

zoomSlider:SetRange(-1, 3.322)
zoomSlider.onValueChanged:Connect(function (new_zoom_pos)
	map:SetZoom(math.pow(2, new_zoom_pos))
end)

local controls =
	ui:Margin(30, 'ALL',
		ui:Align('LEFT',
			ui:HBox():PackEnd({
				ui:VBox(10):PackEnd({
					l.ZOOM,
					ui:Expand('VERTICAL', ui:Align('MIDDLE', zoomSlider)),
				}),
				ui:Align('BOTTOM', ui:HBox(10):PackEnd({scaleIndicator, scaleLabel}))
			})
		)
	)

local galacticView =
	ui:ColorBackground(0,0,0,1,
		ui:OverlayStack():AddLayer(map):AddLayer(controls))

ui.templates.GalacticView = function ()
	resetCurrentSector(map)
	return galacticView
end
