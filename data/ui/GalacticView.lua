-- Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Lang = import("Lang")

local ui = Engine.ui
local l = Lang.GetResource("ui-core");

local galacticView =
	ui:Align("MIDDLE", ui:Label("GALAXY!"):SetFont("HEADING_XLARGE"))

ui.templates.GalacticView = function ()
	return galacticView
end
