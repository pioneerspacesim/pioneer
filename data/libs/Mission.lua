-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local Serializer = require 'Serializer'
local Character = require 'Character'
local Lang = require 'Lang'
local Engine = require 'Engine'

local l = Lang.GetResource("ui-core")

--
-- Class: Mission
--
-- A mission object, which is displayed in the missions screen. These
-- missions are stored in the PersistenCharacters.player.missions table.
-- The class is responsible for inserting itself into the missions table,
-- and has a method which removes itself from that table.
--
-- Lua modules should use the Mission interface, which provides data
-- sanitation and error checking.
--

-- Registered mission type data go here
local MissionRegister = {}

local Mission
Mission = {
--
-- Group: Attributes
--

--
-- Attribute: type
--
-- The type of mission.  This can be any registered mission typeid.
--
-- Availability:
--
--   alpha 30
--
-- Status:
--
--   experimental
--
	type = 'NONE',

--
-- Attribute: client
--
-- The [Character] object that offered the mission
--
-- Availability:
--
--   alpha 30
--
-- Status:
--
--   experimental
--

--
-- Attribute: due
--
-- Due date/time, in seconds since 12:00 01-01-3200
--
-- Availability:
--
--   alpha 30
--
-- Status:
--
--   experimental
--
	due = 0,

--
-- Attribute: reward
--
-- Reward for mission completion, in dollars
--
-- Availability:
--
--   alpha 30
--
-- Status:
--
--   experimental
--
	reward = 0,

--
-- Attribute:Location
--
-- A [SystemPath] for the destination space station
--
-- Availability:
--
--   alpha 30
--
-- Status:
--
--   experimental
--
	location = nil,

--
-- Attribute: status
--
-- A string for the current mission status.
--
-- Availability:
--
--   alpha 30
--
-- Status:
--
--   experimental
--
	status = 'ACTIVE',

--
-- Group: Methods
--

--
-- Method: RegisterType
--
-- Register the mission's type, so that the UI has access to information about it. Includes
-- the name of the mission type, and a function that returns a UI widget for the missions
-- screen.
--
-- > Mission.RegisterType(typeid, display, onClick)
--
-- Parameters:
--
--   typeid - a unique string with which to distinguish the mission type.
--            This must be globally unique; the player cannot load modules
--            that attempt to register a typeid that already exists.
--   display - a (translatable) string to be shown in the missions list
--   onClick - (optional) a function which is executed when the details button is
--             clicked. The mission instance is passed to onClick, which
--             must return a mission description table to be displayed on the
--             missions screen.
--
-- Example:
--
-- > RegisterType('racing_mission_XC14','Race',function (mission)
-- >   local race = races[mission] -- Assuming some local table of race missions for example
-- >   return {
-- >       client = mission.client,
-- >       description = string.interp('Stage {stage}: You are in position {pos}',{stage=race.stage, pos=race.pos})
-- >   }
-- > end)
--
-- Availability:
--
--   alpha 30
--
-- Status:
--
--    experimental
--

	RegisterType = function (typeid, display, onClick)
		if not typeid or (type(typeid)~='string') then
			error('typeid: String expected')
		end
		if not display or (type(display)~='string') then
			error('display: String expected')
		end
		if MissionRegister[typeid] then -- We can't have duplicates
			error('Mission type already registered: '..typeid)
		end
		local missiontype = {display = display}
		if onClick and not (type(onClick) == 'function') then
			error('onClick: function expected')
		else
			missiontype.onClick = onClick -- Might be nil; that's fine
		end
		MissionRegister[typeid] = missiontype
	end,

--
-- Method: New
--
-- Create a new mission and add it to the player's mission list
--
-- >	missionRef = Mission.New({
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
-- New takes a table as its only parameter.  The fields of that table match the attributes
-- of the Mission class.
--
-- Return:
--
--   missionRef - a new instance of the Mission class
--
-- Example:
--
-- >  local ref = Mission.New({
-- >      'type'     = 'Delivery', -- Must be a translatable token!
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
	New = function (template)
		-- Data sanitation
		if not template or (type(template) ~= 'table') then
			error("Missing argument: mission table expected")
		end
		local test = getmetatable(template)
		if(test and (test.class == 'Mission')) then
			error("Won't use another mission as a template.")
		end
		if not (type(template.type) == "string") then template.type = nil end
		if not template.client then template.client = Character.New() end
		if not (
			type(template.client) == "table" and
			getmetatable(template.client) and
			getmetatable(template.client).class == 'Character'
		) then
			error("Mission.New: client must be a Character object")
		end
		-- Initialise the new mission
		local newMission = {}
		for k,v in pairs(template) do
			newMission[k] = v
		end
		setmetatable(newMission,Mission.meta)
		if not newMission:GetTypeDescription() then -- An invalid typeid was given
			error(('Mission.New: type "{typeid}" has not been registered with Mission.RegisterType()')
					:interp({typeid=newMission.type}))
		end
		if not (type(newMission.due) == "number") then newMission.due = nil end
		if not (type(newMission.reward) == "number") then newMission.reward = nil end
		if not (type(newMission.location) == "userdata") then newMission.location = Game.system.path end
		table.insert(Character.persistent.player.missions,newMission)
		return newMission;
	end,
--
-- Method: Remove
--
-- Remove a mission from the player's mission list
--
-- Mission:Remove()
--
-- Example:
--
-- > ourMission:Remove() -- Remove mission from list
-- > ourMission = nil    -- Remove our reference too
--
-- Availability:
--
-- alpha 30
--
-- Status:
--
-- testing
--
	Remove = function (self)
		for k,v in pairs(Character.persistent.player.missions) do
			if v == self then
				table.remove(Character.persistent.player.missions,k)
			end
		end
	end,

--
-- Method: GetClick
--
-- Internal method to retrieve a handler function for the mission list button.
-- Normally called from InfoView/Missions, but could be useful elsewhere.
--
-- > handler = ourMission:GetClick()
--
--
-- Returns:
--
--   handler - a function to be connected to the missions form 'Active'
--             button. handler will be passed the mission as its sole argument
--             and is expected to return an [Engine.UI] object, or nil.
--
-- Availability:
--
-- alpha 29
--
-- Status:
--
-- experimental
--
	GetClick = function (self)
		return MissionRegister[self.type].onClick
		-- return (MissionRegister[self.type] and MissionRegister[self.type].onClick)
		-- 		or function (ref) return Engine.ui:Label(l.NOT_FOUND) end -- XXX don't translate things in libs
	end,

--
-- Method: GetTypeDescription
--
-- Internal method to retrieve a descriptive text for the mission list.
-- Normally called from InfoView/Missions, but could be useful elsewhere.
--
-- > displayType = ourMission:GetTypeDescription()
--
--
-- Returns:
--
--   displayType - a ready-translated string describing the mission's type, or nil.
--                 A nil return value indicates an unregistered mission type.
--
-- Availability:
--
-- alpha 30
--
-- Status:
--
-- experimental
--
	GetTypeDescription = function (self)
		return MissionRegister[self.type] and MissionRegister[self.type].display
		-- Might return nil; this indicates an unregistered mission type.
	end,

	Serialize = function (self)
		return self
	end,

	Unserialize = function (data)
		setmetatable(data,Mission.meta)
		return data
	end,
}

Mission.meta = {
	__index = Mission,
	class = "Mission",
}

Serializer:RegisterClass("Mission", Mission)

return Mission
