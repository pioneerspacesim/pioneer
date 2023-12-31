-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Lang = require 'Lang'
local Comms = require 'Comms'
local Event = require 'Event'

local l = Lang.GetResource("module-aiwarning")

local messages = {
	GRAV_TOO_HIGH    = l.CANNOT_COMPENSATE_FOR_LOCAL_GRAVITY,
	PRESS_TOO_HIGH   = l.ATMO_PRESSURE_OVER_LIMIT,
	REFUSED_PERM     = l.STARPORT_REFUSED_DOCKING_PERMISSION,
	ORBIT_IMPOSSIBLE = l.CANNOT_COMPUTE_ORBIT_PARAMETERS,
}

Event.Register("onAICompleted", function (s, e)
	if e == 'NONE' then return end
	if not s:IsPlayer() then return end

	Comms.ImportantMessage(messages[e], l.AUTOPILOT)
end)
