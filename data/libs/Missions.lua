-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

--
-- Interface: Missions
--
-- Provides a set of interfaces to the "Missions" infoview screen. These
-- missions are stored in the PersistenCharacters.player.missions table.
--
-- Lua modules should use the Missions interface, which provides data
-- sanitation and error checking.
--

local MissionStatus = {
	-- Valid mission states, used in data sanitation
	ACTIVE = true,
	COMPLETED = true,
	FAILED = true,
}

-- Registered mission click functions go here
local MissionClickHandler = {}

--
-- Group: Methods
--

Mission = {
--
-- Method: Add
--
-- Add a mission to the player's mission list
--
-- >	ref = Mission.Add({
-- >		'type'     = type,
-- >		'client'   = client,
-- >		'due'      = due,
-- >		'reward'   = reward,
-- >		'location' = location,
-- >		'status'   = status,
-- >	})
--
-- Parameters:
--
-- Add takes a table as its only parameter.  The fields of that table are as follows
--
--   type - type of mission.  This can be any translatable string token.
--   client - the Character object that offered the mission
--   due - due date/time, in seconds since 12:00 01-01-3200
--   reward - reward for mission completion, in dollars
--   location - a SystemPath for the destination space station
--   status - a Constants.MissionStatus string for the current mission status
--
-- Return:
--
--   ref - an integer value for referring to the mission in the future
--
-- Example:
--
-- >  local ref = Mission.Add({
-- >      'type'     = 'Delivery',
-- >      'client'   = Character.New()
-- >      'due'      = Game.time + 3*24*60*60,    -- three days
-- >      'reward'   = 123.45,
-- >      'location' = SystemPath:New(0,0,0,0,16),  -- Mars High, Sol
-- >      'status'   = 'ACTIVE',
-- >  })
--
-- Availability:
--
-- alpha 29
--
-- Status:
--
-- testing
--
	Add = function (row)
		-- Data sanitation
		if not row or (type(row) ~= 'table') then
			error("Missing argument: mission table expected")
		end
		if not (type(row.type) == "string") then row.type = 'NONE' end
		if not row.client then row.client = Character.New() end
		if not (
			type(row.client) == "table" and
			getmetatable(row.client) and
			getmetatable(row.client).class == 'Character'
		) then
			error("Mission.Add: client must be a Character object")
		end
		if not (type(row.due) == "number") then row.due = 0 end
		if not (type(row.reward) == "number") then row.reward = 0 end
		if not (type(row.location) == "userdata") then row.location = Game.system.path end
		if not MissionStatus[row.status] then row.status = 'ACTIVE' end
		table.insert(PersistentCharacters.player.missions,row)
		return #PersistentCharacters.player.missions
	end,
--
-- Method: Get
--
-- Retrieve a mission from the player's mission list
--
-- > mission = Mission.Get(ref)
--
-- Parameters:
--
--   ref - the mission reference number returned by AddMission
--
-- Return:
--
--   mission - a table containing the mission parameters.  The fields
--   of the table are the same as described in AddMission.
--
-- Availability:
--
-- alpha 29
--
-- Status:
--
-- testing
--
	Get = function (ref)
		-- Add some reference checking here, or we could be in trouble
		if not PersistentCharacters.player.missions[ref] then
			error("Mission reference not valid")
		end
		local returnCopy = {}
		for k,v in pairs(PersistentCharacters.player.missions[ref]) do
			returnCopy[k] = v
		end
		return returnCopy
	end,
--
-- Method: Update
--
-- Update a mission on the player's mission list
--
-- >  Mission.Update(ref, mission)
--
-- The mission data provided to this method is used to overwrite the
-- existing mission data.  The intention is that you will use GetMission
-- to retrieve the mission table, make the modifications you need, and
-- then call UpdateMission to update it.
--
-- Parameters:
--
--   ref - the mission reference number returned by AddMission
--   mission - a table of mission fields.  The fields are the same as those described in AddMission.
--
-- Example:
--
-- > local mission = Mission.Get(ref)
-- > mission.status = 'FAILED'
-- > Mission.Update(ref, mission)
--
-- Availability:
--
-- alpha 29
--
-- Status:
--
-- testing
--
	Update = function (ref, row)
		-- Reference and data sanitation
		if not row or (type(row) ~= 'table') then
			error("Missing argument: updated mission table expected")
		end
		if not PersistentCharacters.player.missions[ref] then
			error("Mission reference ",ref," not valid")
		end
		if row.type and not (type(row.type) == "string") then row.type = 'NONE' end
		if row.client and not (type(row.client) == "table") then row.client = nil end -- Don't replace with junk
		if row.due and not (type(row.due) == "number") then row.due = 0 end
		if row.reward and not (type(row.reward) == "number") then row.reward = 0 end
		if row.location and not (type(row.location) == "userdata") then row.location = Game.system.path end
		if row.status and not MissionStatus[row.status] then row.status = 'ACTIVE' end
		local missions = PersistentCharacters.player.missions
		for k,v in pairs(row) do
			missions[ref][k] = v
		end
	end,
--
-- Method: Remove
--
-- Remove a mission from the player's mission list
--
-- > Mission.Remove(ref)
--
-- Parameters:
--
--   ref - the mission reference number returned by AddMission
--
-- Availability:
--
-- alpha 29
--
-- Status:
--
-- testing
--
	Remove = function (ref)
		PersistentCharacters.player.missions[ref] = nil
	end,
--
-- Method: RegisterClick
--
-- Register a handler function for the mission list button
--
-- > Mission.RegisterClick(type, handler)
--
-- Parameters:
--
--   type - the mission type, as seen in the Mission.Add() example
--          handler - a function to be run when the "Active" button is
--          clicked. Handler is passed a reference compatible with
--          Mission.Get() and may return an Engine.ui instance, which
--          will be displayed on screen if not nil.
--
-- Example:
--
-- > Mission.RegisterClick('Race',function (ref)
-- >   local race = races[ref] -- Assuming some local table of races for example
-- >   Comms.Message(string.interp('Stage {stage}: You are in position {pos}',{stage=race.stage, pos=race.pos}))
-- > end) -- Not returning a UI in this small example
--
-- Availability:
--
-- alpha 29
--
-- Status:
--
-- experimental
--
	RegisterClick = function (missiontype, handler)
		if not (type(missiontype) == 'string') then
			error('Mission.RegisterClick: type must be a translatable string token')
		end
		if not (type(handler) == 'function') then
			error('Mission.RegisterClick: handler must be a function')
		end
		MissionClickHandler[missiontype] = handler
	end,
--
-- Method: GetClick
--
-- Internal method to retrieve a handler function for the mission list button.
-- Normally called from InfoView/Missions, but could be useful elsewhere.
--
-- > handler = Mission.GetClick(type)
--
-- Parameters:
--
--   type - the mission type, as seen in the Mission.Add() example
--          handler - a function to be run when the "Active" button is
--          clicked. Handler is passed a reference compatible with
--          Mission.Get() and may return an Engine.ui instance, which
--          will be displayed on screen if not nil.
--
-- Returns:
--
--   handler - a function to be connected to the missions form 'Active'
--             button.
--
-- Availability:
--
-- alpha 29
--
-- Status:
--
-- experimental
--
	GetClick = function (missiontype)
		local t = Translate:GetTranslator()
		return MissionClickHandler[missiontype] or function (ref) return Engine.ui:Label(t('NOT_FOUND')) end
	end
}
