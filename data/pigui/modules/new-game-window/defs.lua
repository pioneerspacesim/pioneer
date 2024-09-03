-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = require 'pigui'
local Defs = {}

Defs.mainFont = ui.fonts.pionillium.medlarge
Defs.subFont = ui.fonts.pionillium.medium
Defs.scrollWidth = Defs.mainFont.size
Defs.winSize = Vector2(Defs.mainFont.size * 60, Defs.mainFont.size * 40)

Defs.rand = require 'Rand'.New()
Defs.charParams = { 'luck', 'intelligence', 'charisma', 'notoriety', 'lawfulness', 'engineering', 'piloting', 'navigation', 'sensors' }
Defs.charInterval = { 4, 64 }

-- it depends on the style settings and can only be computed inside a frame
function Defs.updateLayoutValues(contentRegion)

	Defs.contentRegion = contentRegion
	Defs.gap = ui.getItemSpacing()
	Defs.lineHeight = Defs.mainFont.size + Defs.gap.y
	Defs.halfSize = Vector2(Defs.contentRegion.x / 2, Defs.contentRegion.y)
	Defs.goodWidth = Defs.winSize.x / 2 - ui.getWindowPadding().x * 2 - Defs.scrollWidth
	Defs.dragWidth = ui.calcTextSize("<-- 1000t -->").x + Defs.gap.x * 2
	Defs.secWidth = ui.calcTextSize("<- -0000 ->").x
	Defs.removeWidth = Defs.lineHeight
	Defs.addWidth = ui.calcTextSize("__+__").x
	Defs.tonnesWidth = ui.calcTextSize("0000t").x
	Defs.eqTonnesWidth = ui.calcTextSize("000t").x
	Defs.frameShadeColor = ui.getStyleColor("FrameBg"):opacity(0.2)
	Defs.progressBarColor = ui.getStyleColor("FrameBgActive")
	Defs.buttonSize = ui.getFrameHeight()
end

return Defs
