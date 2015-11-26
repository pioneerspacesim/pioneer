local Game = import("Game")
local Engine = import("Engine")
local Event = import("Event")
local FileSystem = import("FileSystem")

local max_autosaves = 9

local function PickNextAutosave()
	local ok, files, _ = pcall(FileSystem.ReadDirectory, 'USER', 'savefiles')
	if not ok then
		print('Error picking auto-save')
		return '_autosave1'
	end

	-- find the most recent autosave
	local next_save_number = 1
	local best_tstamp
	for i = 1, #files do
		local f = files[i]
		local num = string.match(f.name, '^_autosave(%d+)$')
		if num ~= nil then
			local tstamp = f.mtime.timestamp
			if best_tstamp == nil or tstamp > best_tstamp then
				best_tstamp = tstamp
				next_save_number = tonumber(num) + 1
				if next_save_number > max_autosaves then
					next_save_number = 1
				end
			end
		end
	end

	return '_autosave' .. next_save_number
end

local function CheckedSave(filename)
	if not Engine.GetAutosaveEnabled() then
		return
	end

	local ok, err = pcall(Game.SaveGame, filename)
	if not ok then
		print('Error making autosave:')
		print(err)
	end
end

local f = function (ship) if ship:IsPlayer() then CheckedSave(PickNextAutosave()); end; end
Event.Register('onShipDocked', f)
Event.Register('onShipLanded', f)
Event.Register('onShipUndocked', f)
Event.Register('onShipTakeOff', f)
Event.Register('onGameEnd', function() CheckedSave('_exit'); end)
