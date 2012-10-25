-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

--
-- Interface: Missions
--
-- Provides a set of interfaces to PersistentCharacters.player.missions
--

local MissionStatus = {
	-- Valid mission states, used in data sanitation
	ACTIVE = true,
	COMPLETED = true,
	FAILED = true,
}

Mission = {
	Add = function (row)
		-- Data sanitation
		if not row or (type(row) ~= 'table') then
			error("Missing argument: mission table expected")
		end
		if not (type(row.type) == "string") then row.type = '-' end
		if not (type(row.client) == "string") then row.client = '-' end
		if not (type(row.due) == "number") then row.due = 0 end
		if not (type(row.reward) == "number") then row.reward = 0 end
		if not (type(row.location) == "userdata") then row.location = Game.system.path end
		if not MissionStatus[row.status] then row.status = 'ACTIVE' end
		table.insert(PersistentCharacters.player.missions,row)
		return #PersistentCharacters.player.missions
	end,
	Get = function (ref)
		-- Add some reference checking here, or we could be in trouble
		if not PersistentCharacters.player.missions[ref] then
			error("Mission reference ",ref," not valid")
		end
		return PersistentCharacters.player.missions[ref]
	end,
	Update = function (ref, row)
		-- Reference and data sanitation
		if not row or (type(row) ~= 'table') then
			error("Missing argument: updated mission table expected")
		end
		if not PersistentCharacters.player.missions[ref] then
			error("Mission reference ",ref," not valid")
		end
		if row.type and not (type(row.type) == "string") then row.type = '-' end
		if row.client and not (type(row.client) == "string") then row.client = '-' end
		if row.due and not (type(row.due) == "number") then row.due = 0 end
		if row.reward and not (type(row.reward) == "number") then row.reward = 0 end
		if row.location and not (type(row.location) == "userdata") then row.location = Game.system.path end
		if row.status and not MissionStatus[row.status] then row.status = 'ACTIVE' end
		local missions = PersistentCharacters.player.missions
		for k,v in pairs(row) do
			missions[ref][k] = v
		end
	end,
	Remove = function (ref)
		table.remove(PersistentCharacters.player.missions, ref)
	end,
}
