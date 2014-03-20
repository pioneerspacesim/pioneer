local Game = import("Game")
local Event = import("Event")

local next_autosave = 1

local function AutoSave()
	Game.SaveGame('_autosave' .. next_autosave)
	next_autosave = next_autosave + 1
	if next_autosave > 9 then
		next_autosave = 1
	end
end

local f = function (ship) if ship:IsPlayer() then AutoSave(); end; end
Event.Register('onShipDocked', f)
Event.Register('onShipLanded', f)
Event.Register('onShipUndocked', f)
Event.Register('onShipTakeOff', f)
Event.Register('onGameEnd', function() Game.SaveGame('_exit'); end)
