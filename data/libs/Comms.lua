-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'

-- Interface: Comms
--
-- Player communication functions.

local Comms = {}

Comms.Priority = {
	Normal = 0,
	Important = 1,
	Alert = 2
}

-- Function: Message
--
-- Post a message to the player's control panel.
--
-- > Comms.Message(message, from)
--
-- Parameters:
--   message - the message text to post
--   from - optional; who the message is from (person, ship, etc)
--
-- Example:
-- > Comms.Message("Please repair my ship.", "Gary Jones")
--
-- Availability:
--   alpha 10
--
-- Status:
--   experimental

function Comms.Message(msg, from)
	Game.AddCommsLogLine(msg, from, Comms.Priority.Normal)
end

-- Function: ImportantMessage
--
-- Post an important message to the player's control panel.
--
-- > Comms.ImportantMessage(message, from)
--
-- The only difference between this and <Message> is that if multiple messages
-- arrive at the same time, the important ones will be shown first.
--
-- Parameters:
--   message - the message text to post
--   from - optional; who the message is from (person, ship, etc)
--
-- Example:
-- > Comms.ImportantMessage("Prepare to die!", "AB-1234")
--
-- Availability:
--   alpha 10
--
-- Status:
--   experimental

function Comms.ImportantMessage(msg, from)
	Game.AddCommsLogLine(msg, from, Comms.Priority.Important)
end

return Comms
