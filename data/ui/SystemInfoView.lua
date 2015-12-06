local Engine = import("Engine")
local Lang = import("Lang")
local ui = Engine.ui
local l = Lang.GetResource("ui-core")

local sysInfoView =
	ui:Expand('BOTH',
		 ui:Align('MIDDLE',
			 ui:Label('System Info View!'):SetFont('HEADING_LARGE')
		)
	)

ui.templates.SystemInfoView = function ()
	return sysInfoView
end
