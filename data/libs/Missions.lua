-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

--
-- Interface: Missions
--
-- Provides a set of interfaces to PersistentCharacters.player.missions
--

Mission = {
	Add = function (row)
		-- Add some data checking here, or we could be in trouble
		table.insert(PersistentCharacters.player.missions,row)
		return #missions
	end,
	Get = function (ref)
		-- Add some reference checking here, or we could be in trouble
		return PersistentCharacters.player.missions[ref]
	end,
	Update = function (ref, row)
		-- Add some reference and data checking here, or we could be in trouble
		local missions = PersistentCharacters.player.missions
		for k,v in pairs(row) do
			missions[ref][k] = v
		end
	end,
}
