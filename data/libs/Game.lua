local Game = import_core("Game")
Game.comms_log_lines = {}
Game.AddCommsLogLine = function(text, sender)
	 table.insert(Game.comms_log_lines, 1, { text=text, time=Game.time, sender=sender})
end
return Game
