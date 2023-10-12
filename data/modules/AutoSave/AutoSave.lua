local Game = require 'Game'
local Engine = require 'Engine'
local Event = require 'Event'
local FileSystem = require 'FileSystem'

local max_autosaves = 9

local function PickNextAutosave()
	local ok, files, _ = pcall(FileSystem.ReadDirectory, 'user://savefiles')
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

local function AutoSave(type)
	if type == 'exit' then
		CheckedSave('_exit')
	else
		CheckedSave(PickNextAutosave())
	end
end

Event.Register('onAutoSave', AutoSave)

local f = function (ship)
	if ship:IsPlayer() then Event.Queue('onAutoSave') end
end

Event.Register('onShipDocked', f)
Event.Register('onShipLanded', f)

-- we have to make sure to autosave the game before the end game process starts
Event.Register('onAutoSaveBeforeGameEnds', function()
	CheckedSave('_exit')
end)

return {
	Save = AutoSave
}
