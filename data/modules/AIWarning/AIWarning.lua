local t = Translate:GetTranslator()

local messages = {
	GRAV_TOO_HIGH    = 'Cannot compensate for local gravity',
	REFUSED_PERM     = 'Starport refused docking permission',
	ORBIT_IMPOSSIBLE = 'Cannot compute orbit parameters',
}

EventQueue.onAICompleted:Connect(function (s, e)
	if e == 'NONE' then return end
	if not s:IsPlayer() then return end

	Comms.ImportantMessage(t(messages[e]), t('AUTOPILOT'))
end)
