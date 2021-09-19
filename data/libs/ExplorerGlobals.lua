-- Copyright � 2008-2021 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local Event = require 'Event'
local Comms = require 'Comms'
local Equipment = require 'Equipment'
local Serializer = require 'Serializer'
local Lang = require 'Lang'
--local utils = require 'utils'

local l = Lang.GetResource("module-explorerclub")

--[[
		Explorer career
--]]

local explorerInvite			-- 0, no invite, 1, active invite, 2, invite accepted (member), 3, invite rejected
local explorerRank = 0			-- 0 trainee, ...
local systemsExplored = 0
local jumpsMade = 0
local lightyearsTraveled = 0
local bountylist				-- make a list of systems explored while a member of the explorers guild
								-- upon docking and connecting to explorers guild, list is used to calculate rewards
local hyperSource
local loaded_data

local ExplorerGlobals
ExplorerGlobals = {

	memberCost = 200,
	deviceCost = 1400,

	device = Equipment.EquipType.New({
		l10n_key="EXPLORER_DEVICE", l10n_resource="module-explorerclub", slots="explorer_device", price=0,
		capabilities={mass=0, explorer_device=1},
		purchasable=false, tech_level=5, infovis=1
	}),

	Init = function()
		explorerInvite	= 0
		explorerRank = 0
		systemsExplored = 0
		bountylist = {}
		jumpsMade = 0
		lightyearsTraveled = 0
		hyperSource = Game.system

		if loaded_data then
			explorerInvite = loaded_data.inv
			explorerRank = loaded_data.rank
			systemsExplored = loaded_data.sysex
			jumpsMade = loaded_data.jumps
			lightyearsTraveled = loaded_data.travel
			bountylist = loaded_data.bolist
			hyperSource = loaded_data.hsrc
			loaded_data = nil
		end
	end,

	getRank = function ()
		return explorerRank
	end,

	getSystemsExplored = function()
		return systemsExplored
	end,

	getJumpsMade = function()
		return jumpsMade
	end,

	getLightyearsTraveled = function()
		return lightyearsTraveled
	end,

	getExplorerInvite = function()
		return explorerInvite
	end,

	setExplorerInvite = function(v)
		explorerInvite = v
	end,

	GetExplorerDevice = function()
		return device
	end

} -- closing ExplorerGlobals

local onShipDocked = function (ship, starport)
	-- when docking again after exploring 10 or more unexplored systems,
	-- explorers guild notifies player of an invitation, place invitation on the bbs

	-- on landing, skip notification if they have previously rejected invitation
	if explorerInvite == 1 then
		Comms.ImportantMessage(string.interp(l.EXPLORERS_INVITE, {clubname=l.EXPLORERS_CLUB}), l.EXPLORERS_CLUB)
	end

	-- but leave the invite on the bbs, hoping that they will see the light and join the cause
	if explorerInvite == 1 or explorerInvite == 3 then
	end
end

local onEnterSystem = function (ship)
	if (ship:IsPlayer()) then
		jumpsMade = jumpsMade + 1
		lightyearsTraveled = lightyearsTraveled + Game.system:DistanceTo(hyperSource)
	end
end
Event.Register("onEnterSystem", onEnterSystem)

local onLeaveSystem = function (ship)
	hyperSource = Game.system.path
end
Event.Register("onLeaveSystem", onLeaveSystem)

local onSystemExplored = function ()
	systemsExplored = systemsExplored + 1

	if systemsExplored >= 10 and explorerInvite == 0 then
		Event.Register("onShipDocked", onShipDocked)
		explorerInvite = 1
	end

	if explorerInvite == 2 and Game.player:GetEquipCountOccupied('explorer_device') > 0 then
		Comms.ImportantMessage(string.interp(l.DATACOLLECTED, {longdevice=l.EXPLORER_DEVICE}), l.EXPLORERS_CLUB)
		table.insert(bountylist, Game.system.path)
	end
end
Event.Register("onSystemExplored", onSystemExplored)

local serialize = function ()
	return { sysex = systemsExplored,
		rank = explorerRank,
		inv = explorerInvite,
		jumps = jumpsMade,
		travel = lightyearsTraveled,
		bolist = bountylist,
		hsrc = hyperSource
	}
end

local unserialize = function (data)
	loaded_data = data
end

Serializer:Register("ExplorerGlobals", serialize, unserialize)

return ExplorerGlobals