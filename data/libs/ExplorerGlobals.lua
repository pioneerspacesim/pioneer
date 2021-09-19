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

local loaded_data

local ExplorerGlobals
ExplorerGlobals = {

	memberCost = 200,
	deviceCost = 1400,

	explorerRank = 0,
	-- explorerInvite: 0, no invite, 1, active invite, 2, invite accepted (member), 3, invite rejected
	explorerInvite = 0,
	systemsExplored = 0,
	jumpsMade = 0,
	lightyearsTraveled = 0,
	-- bountylist: make a list of systems explored while a member of the explorers guild
	-- 			   upon docking and connecting to explorers guild, list is used to calculate rewards
	bountylist = {},
	-- hyperSource: save the system path we are jumping from to accumulate lightyearstraveled
	hyperSource = nil,

	device = Equipment.EquipType.New({
		l10n_key="EXPLORER_DEVICE", l10n_resource="module-explorerclub", slots="explorer_device", price=0,
		capabilities={mass=0, explorer_device=1},
		purchasable=false, tech_level=5, infovis=1
	}),

	Init = function()
		ExplorerGlobals.explorerInvite	= 0
		ExplorerGlobals.explorerRank = 0
		ExplorerGlobals.systemsExplored = 0
		ExplorerGlobals.bountylist = {}
		ExplorerGlobals.jumpsMade = 0
		ExplorerGlobals.lightyearsTraveled = 0
		ExplorerGlobals.hyperSource = nil

		if loaded_data then
			ExplorerGlobals.explorerInvite = loaded_data.inv
			ExplorerGlobals.explorerRank = loaded_data.rank
			ExplorerGlobals.systemsExplored = loaded_data.sysex
			ExplorerGlobals.jumpsMade = loaded_data.jumps
			ExplorerGlobals.lightyearsTraveled = loaded_data.travel
			ExplorerGlobals.bountylist = loaded_data.bolist
			ExplorerGlobals.hyperSource = loaded_data.hsrc
			loaded_data = nil
		end
	end

} -- closing ExplorerGlobals

local onShipDocked = function(ship, starport)
	-- any ship can trigger this event, take no action unless its the player
	if not ship:IsPlayer() then return end

	-- when docking again after exploring 10 or more unexplored systems,
	-- explorers guild notifies player of an invitation, place invitation on the bbs

	-- on landing, skip notification if they have previously rejected invitation
	if ExplorerGlobals.explorerInvite == 1 then
		Comms.ImportantMessage(string.interp(l.EXPLORERS_INVITE, {clubname=l.EXPLORERS_CLUB}), l.EXPLORERS_CLUB)
	end

	-- but leave the invite on the bbs, hoping that they will see the light and join the cause
	if ExplorerGlobals.explorerInvite == 1 or ExplorerGlobals.explorerInvite == 3 then
	end
end

local onEnterSystem = function(ship)
	-- any ship can trigger this event, take no action unless its the player
	if not ship:IsPlayer() then return end

	ExplorerGlobals.jumpsMade = ExplorerGlobals.jumpsMade + 1
	ExplorerGlobals.lightyearsTraveled = ExplorerGlobals.lightyearsTraveled + Game.system:DistanceTo(ExplorerGlobals.hyperSource)
end

local onLeaveSystem = function(ship)
	-- any ship can trigger this event, take no action unless its the player
	if not ship:IsPlayer() then return end

	ExplorerGlobals.hyperSource = Game.system.path
end

local onSystemExplored = function(system)
	ExplorerGlobals.systemsExplored = ExplorerGlobals.systemsExplored + 1

	if ExplorerGlobals.systemsExplored >= 10 and ExplorerGlobals.explorerInvite == 0 then
		Event.Register("onShipDocked", onShipDocked)
		ExplorerGlobals.explorerInvite = 1
	end

	if ExplorerGlobals.explorerInvite == 2 and Game.player:GetEquipCountOccupied('explorer_device') > 0 then
		Comms.ImportantMessage(string.interp(l.DATACOLLECTED, {longdevice=l.EXPLORER_DEVICE}), l.EXPLORERS_CLUB)
		table.insert(ExplorerGlobals.bountylist, system.path)
	end
end

local serialize = function()
	return {
		sysex = ExplorerGlobals.systemsExplored,
		rank = ExplorerGlobals.explorerRank,
		inv = ExplorerGlobals.explorerInvite,
		jumps = ExplorerGlobals.jumpsMade,
		travel = ExplorerGlobals.lightyearsTraveled,
		bolist = ExplorerGlobals.bountylist,
		hsrc = ExplorerGlobals.hyperSource
	}
end

local unserialize = function(data)
	loaded_data = data
end

Serializer:Register("ExplorerGlobals", serialize, unserialize)

Event.Register("onEnterSystem", onEnterSystem)
Event.Register("onLeaveSystem", onLeaveSystem)
Event.Register("onSystemExplored", onSystemExplored)

return ExplorerGlobals