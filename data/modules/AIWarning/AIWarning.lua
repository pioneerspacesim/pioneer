-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Comms = import("Comms")
local Translate = import("Translate")
local Event = import("Event")

local t = Translate:GetTranslator()

local messages = {
	GRAV_TOO_HIGH    = 'Cannot compensate for local gravity',
	REFUSED_PERM     = 'Starport refused docking permission',
	ORBIT_IMPOSSIBLE = 'Cannot compute orbit parameters',
}

Event.Register("onAICompleted", function (s, e)
	if e == 'NONE' then return end
	if not s:IsPlayer() then return end

	Comms.ImportantMessage(t(messages[e]), t('AUTOPILOT'))
end)
