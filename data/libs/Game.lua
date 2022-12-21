local Game = package.core["Game"]
local Event = require 'Event'

-- Simple way to track the start date of the game until DateTime objects are exposed to lua
local gameStartTime = 0.0

Game.comms_log_lines = {}
Game.AddCommsLogLine = function(text, sender, priority)
	table.insert(Game.comms_log_lines, { text=text, time=Game.time, sender=sender, priority = priority or 'normal'})
	if type(priority) == "number" and priority > 0 then
		Game.SetTimeAcceleration("1x")
	end 
end

Game.GetCommsLines = function()
	return Game.comms_log_lines
end

-- Function: GetStartTime()
--
-- Returns the zero-offset time in seconds since Jan 1 3200 at which the
-- current game began.
--
-- > local seconds_since_start = Game.time - Game.GetStartTime()
--
function Game.GetStartTime()
	return gameStartTime
end

Event.Register('onGameStart', function()
	Game.comms_log_lines = {}
	gameStartTime = Game.time
end)

Event.Register('onGameEnd', function()
	gameStartTime = 0
end)

local function _serialize()
	return { startTime = gameStartTime }
end

local function _deserialize(data)
	gameStartTime = data.startTime or 0
end

require 'Serializer':Register('Game', _serialize, _deserialize)

return Game
