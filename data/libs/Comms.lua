local Comms = import_core("Comms")
local Game = import("Game")
Comms.ImportantMessage = function(text, sender)
	 Game.AddCommsLogLine(text, sender)
end
Comms.Message = function(text, sender)
	 Game.AddCommsLogLine(text, sender)
end

return Comms
