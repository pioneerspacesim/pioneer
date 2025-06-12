-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Style = require 'pigui.libs.style'
local ui = require 'pigui.baseui'

local theme = ui.theme
local colors = theme.colors
local styleVars = theme.styles

local rescaleUI = ui.rescaleUI

local styles = {}

styles.sidebar = Style:clone({
	colors = {},
	vars = {
		FrameBorderSize = 1.0,
		ChildBorderSize = 1.0,
		FrameRounding   = styleVars.StyleRounding,
		ChildRounding   = styleVars.StyleRounding,
	}
})

styles.combo = Style:clone({
	colors = {
		FrameBg        = colors.uiPrimaryDark,
		FrameBgHovered = colors.uiPrimary,
		Button         = colors.uiPrimaryDark,
		ButtonHovered  = colors.uiPrimary,
	}, vars = {
		FrameRounding   = styleVars.StyleRounding,
		PopupRounding   = styleVars.StyleRounding,
		FramePadding    = rescaleUI(Vector2(8, 6)),
		FrameBorderSize = 1.0
	}
})

return styles
