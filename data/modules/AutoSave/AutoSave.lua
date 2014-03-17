local Game = import("Game")
local Event = import("Event")

local function SaveDocked(ship)
	if ship:IsPlayer() then
		Game.SaveGame('_docked')
	end
end

local function SaveUndocked(ship)
	if ship:IsPlayer() then
		Game.SaveGame('_undocked')
	end
end

Event.Register('onShipDocked', SaveDocked)
Event.Register('onShipLanded', SaveDocked)
Event.Register('onShipUndocked', SaveUndocked)
Event.Register('onShipTakeOff', SaveUndocked)
