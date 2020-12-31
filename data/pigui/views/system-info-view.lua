-- Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local ui = require 'pigui'

-- TODO: draw the system-info view here (or merge with system map view)

ui.registerModule("game", function()
	if Game.CurrentView() == "system_info" then
		if ui.escapeKeyReleased() then
			Game.SetView("sector")
		end
	end
end)
