local Game = import("Game")
local Event = import("Event")

-- We deliberately retain the next_autosave state between games.
-- This is likely to break in the future, as the engine will probably
-- be changed to destroy the Lua context between games.
local next_autosave = 1

local function CheckedSave(filename)
	local ok, err = pcall(Game.SaveGame, filename)
	if not ok then
		print('Error making autosave:')
		print(err)
	end
end

local function AutoSave()
	CheckedSave('_autosave' .. next_autosave)
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
Event.Register('onGameEnd', function() CheckedSave('_exit'); end)
