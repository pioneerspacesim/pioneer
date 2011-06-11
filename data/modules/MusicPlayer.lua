local test = function()
	Music.Play("scj");
end

EventQueue.onGameStart:Connect(test)
