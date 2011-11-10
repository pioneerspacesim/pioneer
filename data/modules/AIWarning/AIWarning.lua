local messages = {
	GRAV_TOO_HIGH    = 'Cannot compensate for local gravity',
	REFUSED_PERM     = 'Starport refused docking permission',
	ORBIT_IMPOSSIBLE = 'Cannot compute orbit paramaters',
}

EventQueue.onAICompleted:Connect(function (s, e)
	if e == 'NONE' then return end
	if not s:IsPlayer() then return end
	
	UI.ImportantMessage(messages[e], 'autopilot')
end)
