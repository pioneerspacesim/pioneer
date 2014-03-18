local Game = import("Game")
local Event = import("Event")

local function Saver(savename)
	return function (ship)
		if ship:IsPlayer() then
			Game.SaveGame(savename)
		end
	end
end

local SaveDocked = Saver('_docked')
local SaveUndocked = Saver('_undocked')

Event.Register('onShipDocked', SaveDocked)
Event.Register('onShipLanded', SaveDocked)
Event.Register('onShipUndocked', SaveUndocked)
Event.Register('onShipTakeOff', SaveUndocked)
Event.Register('onGameEnd', function() Game.SaveGame('_exit'); end)
