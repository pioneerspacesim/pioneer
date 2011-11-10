local t = Translate:GetTranslator()

EventQueue.onAICompleted:Connect(function (s, e)

	local messages = {
		GRAV_TOO_HIGH    = t('Cannot compensate for local gravity'),
		REFUSED_PERM     = t('Starport refused docking permission'),
		ORBIT_IMPOSSIBLE = t('Cannot compute orbit parameters'),
	}

	if e == 'NONE' then return end
	if not s:IsPlayer() then return end
	
	UI.ImportantMessage(messages[e], t('AUTOPILOT'))
end)
