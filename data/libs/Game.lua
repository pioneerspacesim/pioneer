local Game = package.core["Game"]
local Event = require 'Event'
Game.comms_log_lines = {}
Game.AddCommsLogLine = function(text, sender, priority)
	 table.insert(Game.comms_log_lines, { text=text, time=Game.time, sender=sender, priority = priority or 'normal'})
end

Game.GetCommsLines = function()
	return Game.comms_log_lines
end

Event.Register('onGameStart', function()
	Game.comms_log_lines = {}
end)
	
return Game
