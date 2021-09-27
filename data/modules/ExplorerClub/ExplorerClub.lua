-- Copyright � 2008-2021 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local Event = require 'Event'
local Comms = require 'Comms'
local Format = require 'Format'
local Serializer = require 'Serializer'
local SystemPath = require 'SystemPath'
local FlightLog = require 'FlightLog'
local Character = require 'Character'
local NameGen = require 'NameGen'
local Rand = require 'Rand'
local Lang = require 'Lang'

local l = Lang.GetResource("module-explorerclub")

local ExplorerData = require 'ExplorerData'

--[[
		Explorer career
--]]

local ad = {}

local explorerBBChat = function(form, ref, option)
	form:Clear()
	-- can this even happen?
	if ad.npc == nil then return end
	form:SetFace(ad.npc)
	form:SetTitle(ad.title)
	local invitestatus = ExplorerData.explorerInvite or 0
	local player = Character.persistent.player
	local bothcost = ExplorerData.deviceCost + ExplorerData.memberCost
	local distFromSol = Game.system:DistanceTo(SystemPath.New(0, 0, 0, 0, 0))
	local substrings = {player = player.name, repname = ad.npcname, station = ad.station.label,
		clubname = l.EXPLORERS_CLUB, shortclub = l.SHORT_CLUB, device= l.SHORT_DEVICE,
		devicecost = Format.Money(ExplorerData.deviceCost), membercost = Format.Money(ExplorerData.memberCost),
		bothcost = Format.Money(bothcost)}
	-- initial dialog. varies if player has already joined (invitestatus)
	if option == 0 then
		if invitestatus == 0 then
			form:SetMessage(string.interp(l.BBS_INVITED_INTRO, substrings))
			form:AddOption(string.interp(l.BBSOPT_TELLMEMORE, substrings), 100)
			form:AddOption(l.BBSOPT_WHAT_FOR_ME, 102)
			form:AddOption(l.BBSOPT_YESDIRECT, 101)
			form:AddOption(l.BBSOPT_NOTHANKS, 103)
		elseif invitestatus == 2 then
			form:SetMessage(string.interp(l.BBS_WELCOMESERVICES, substrings))
			if Game.player:GetEquipCountOccupied('explorer_device') ~= 0 then
				form:AddOption(l.BBSOPT_SUBMITNEWDATA, 301)
			end
			form:AddOption(string.interp(l.BBSOPT_EXPLAIN_RANKS, substrings), 401)
			-- if player is still within the core, tell them where to find unexplored stars
			if distFromSol < 250 then
				form:AddOption(l.BBSOPT_WHERE_UNEXPLORED, 402)
			end
			if Game.player:GetEquipCountOccupied('explorer_device') == 0 then
				form:AddOption(string.interp(l.BBSOPT_NEED_DEVICE, substrings), 125)
			end
		-- previously rejected maybe changed their mind?
		elseif invitestatus == 3 then
			form:SetMessage(l.BBS_REJECT_INTRO)
			form:AddOption(string.interp(l.BBSOPT_TELLMEMORE, substrings), 201)
			form:AddOption(l.BBSOPT_WHAT_FOR_ME, 202)
			form:AddOption(l.BBSOPT_CHANGEDMYMIND, 106)
			form:AddOption(l.BBSOPT_STILLNOINTEREST, 203)
		else
			form:SetMessage(l.BBS_SYSTEM_ERROR)
		end
	-- options 100+ is only possible if invitestatus is 1
	elseif option == 100 then
		form:SetMessage(string.interp(l.BBS_CLUBSTORY, substrings))
		form:AddOption(l.BBSOPT_WHAT_FOR_ME, 102)
		form:AddOption(l.BBSOPT_NOTHANKS, 103)
	-- unquestionable enthusiasm, join without question
	elseif option == 101 then
		form:SetMessage(string.interp(l.BBS_NOTHINGTOSIGN, substrings))
		form:AddOption(string.interp(l.BBSOPT_WHAT_DEVICE, substrings), 105)
		form:AddOption(l.BBSOPT_OKLETSDOIT, 106)
		form:AddOption(l.BBSOPT_NOTHANKS, 103)
	-- whats in it for me?
	elseif option == 102 then
		form:SetMessage(string.interp(l.BBS_EXPLAIN_BENEFITS, substrings))
		form:AddOption(string.interp(l.BBSOPT_WHAT_DEVICE, substrings), 105)
		form:AddOption(l.BBSOPT_OKLETSDOIT, 106)
		form:AddOption(l.BBSOPT_NOTHANKS, 103)
	-- no thanks, dont join
	elseif option == 103 then
		form:SetMessage(l.BBS_REJECTIONTEXT)
		ExplorerData.setExplorerInvite(3) -- 3 = initation rejected
	-- explain the membership token
	elseif option == 105 then
		form:SetMessage(string.interp(l.BBS_EXPLAIN_DEVICE, substrings))
		form:AddOption(l.BBSOPT_OKLETSDOIT, 106)
		form:AddOption(l.BBSOPT_NOTHANKS, 103)
	-- joining
	elseif option == 106 then
		form:SetMessage(string.interp(l.BBS_MEMBERSHIP_FEE, substrings))
		form:AddOption(string.interp(l.BBSOPT_JUST_MEMBERSHIP, substrings), 120)
		form:AddOption(string.interp(l.BBSOPT_MEMBERANDDEVICE, substrings), 121)
	-- only membership
	elseif option == 120 then
		local cash = Game.player:GetMoney()
		if (cash < ExplorerData.memberCost) then
			form:SetMessage(string.interp(l.BBS_NOFUNDS, substrings))
		else
			Game.player:AddMoney(-ExplorerData.memberCost)
			form:SetMessage(string.interp(l.BBS_WELCOME_ONLY, substrings))
			form:AddOption(l.BBSOPT_THANKYOU, -1)
			ExplorerData.explorerInvite = 2 -- 2 = player is now a member
			ExplorerData.explorerRank = 0
			-- push a custom log entry to mark the occasion
			FlightLog.MakeCustomEntry(string.interp(l.LOG_EXPLORER_JOIN, substrings))
		end
	-- both membership fee and explorer device
	elseif option == 121 then
		local cash = Game.player:GetMoney()
		if (cash < bothcost) then
			form:SetMessage(string.interp(l.BBS_NOFUNDS, substrings))
		else
			form:SetMessage(string.interp(l.BBS_WELCOME_FULL, substrings))
			form:AddOption(l.BBSOPT_THANKYOU, -1)
			Game.player:AddMoney(-bothcost)
			ExplorerData.explorerInvite = 2 -- 2 = player is now a member
			-- push a custom log entry to mark the occasion
			FlightLog.MakeCustomEntry(string.interp(l.LOG_EXPLORER_JOIN, substrings))
			-- OH MY! This looks hacky. But Pioneer equipment handling is retarded
			Game.player.equipSet.slots["explorer_device"] = { ExplorerData.device, __occupied = 1, __limit = 1}
			if Game.player:GetEquipCountOccupied('explorer_device') > 0 then
				Comms.Message(string.interp(l.COMM_EXPLORER_JOIN, substrings), l.EXPLORERS_CLUB)
			end
		end
	elseif option == 125 then
		local cash = Game.player:GetMoney()
		if (cash < ExplorerData.deviceCost) then
			form:SetMessage(string.interp(l.BBS_NOFUNDS, substrings))
		else
			form:SetMessage(string.interp(l.BBS_NEW_DEVICE_INSTALLED, substrings))
			form:AddOption(l.BBSOPT_THANKYOU, -1)
			Game.player:AddMoney(-ExplorerData.deviceCost)
			-- OH MY! This looks hacky. But Pioneer equipment handling is retarded
			Game.player.equipSet.slots["explorer_device"] = { ExplorerData.device, __occupied = 1, __limit = 1}
		end
	-- tell me about the club
	elseif option == 201 then
		form:SetMessage(string.interp(l.CLUBSTORY, substrings))
		form:AddOption(l.BBSOPT_WHATFORME, 202)
		form:AddOption(l.BBSOPT_CHANGEDMYMIND, 106)
		form:AddOption(l.BBSOPT_STILLNOINTEREST, 203)
	-- whats in it for me?
	elseif option == 202 then
		form:SetMessage(string.interp(l.BBS_EXPLAIN_BENEFITS, substrings))
		form:AddOption(string.interp(l.BBSOPT_WHAT_DEVICE, substrings), 205)
		form:AddOption(l.BBSOPT_CHANGEDMYMIND, 106)
		form:AddOption(l.BBSOPT_STILLNOINTEREST, 203)
	-- still dont want to join
	elseif option == 203 then
		form:SetMessage(l.BBS_REJECTAGAIN)
	-- explain the token
	elseif option == 205 then
		form:SetMessage(string.interp(l.EXPLAIN_DEVICE, substrings))
		form:AddOption(l.BBSOPT_CHANGEDMYMIND, 106)
		form:AddOption(l.BBSOPT_STILLNOINTEREST, 203)
	elseif option == 301 then
		-- WIP to be continued...
		form:SetMessage(string.interp(l.BBS_NONEWDATAFOUND, substrings))
	elseif option == 401 then
		--string.interp(, substrings)
		form:SetMessage(string.interp(l.BBS_EXPLAIN_RANKS, substrings))
		form:AddOption(l.BBSOPT_OKGOTIT, 0)
	elseif option == 402 then
		form:SetMessage(string.interp(l.BBS_WHERE_UNEXPLORED, substrings))
		form:AddOption(l.BBSOPT_OKGOTIT, 0)
	end
end

local onShipDocked = function(ship, station)
	-- any ship can trigger this event, take no action unless its the player
	if not ship:IsPlayer() then return end

	-- on landing, skip notification if they have previously rejected invitation
	if ExplorerData.explorerInvite == 1 then
		Comms.ImportantMessage(string.interp(l.EXPLORERS_INVITE, {clubname=l.EXPLORERS_CLUB}), l.EXPLORERS_CLUB)
	end
end

local onEnterSystem = function(ship)
	-- any ship can trigger this event, take no action unless its the player
	if not ship:IsPlayer() then return end

	ExplorerData.jumpsMade = (ExplorerData.jumpsMade or 0) + 1
	if ExplorerData.hyperSource ~= nil then
		ExplorerData.lightyearsTraveled = (ExplorerData.lightyearsTraveled or 0) + Game.system:DistanceTo(ExplorerData.hyperSource)
	end
end

local onLeaveSystem = function(ship)
	-- any ship can trigger this event, take no action unless its the player
	if not ship:IsPlayer() then return end

	ExplorerData.hyperSource = Game.system.path
end

local onSystemExplored = function(system)
	if ExplorerData.systemsExplored then
		ExplorerData.systemsExplored = ExplorerData.systemsExplored + 1
	else
		ExplorerData.systemsExplored = 1
	end

	if ExplorerData.systemsExplored >= 10 and ExplorerData.explorerInvite == 0 then
		ExplorerData.explorerInvite = 1
	end

	if ExplorerData.explorerInvite == 2 and Game.player:GetEquipCountOccupied('explorer_device') > 0 then
		Comms.ImportantMessage(string.interp(l.DATACOLLECTED, {longdevice=l.EXPLORER_DEVICE}), l.EXPLORERS_CLUB)
		if ExplorerData.bountylist == nil then
			ExplorerData.bountylist = {}
		end
		ExplorerData.bountylist[system.path] = { when = Game.time, processed = false }
	end
end

local explorerBBDelete = function (ref)
	ad = {}
end

local onCreateBB = function(station)
	ad.station = station
	local rand = Rand.New(ad.station.seed + -2017)

	-- Explorers club listed only on some stations
	if (true) then -- for debug
	--if (station.isHomeworld == 1 or 15 > rand:Integer(100)) then
		local text = l.BBS_EXPLORERS_LOCAL_CONTACT
		if (station.isHomeworld == 1) then
			text = l.BBS_EXPLORERS_HQ_CONTACT
		end
		ad.title = string.interp(text, {station = station.label})
		ad.gender = (rand:Integer(1) == 1)
		ad.npcname = NameGen.FullName(ad.gender, rand)
		ad.npc = Character.New({name = ad.npcname, female = ad.gender, seed = station.seed + -2017})

		station:AddAdvert({
			description = ad.title,
			icon        = "explorer",
			onChat      = explorerBBChat,
			onDelete    = explorerBBDelete
		})
	end
end

local loaded_data

local onGameStart = function()
	if loaded_data then
		ExplorerData.explorerInvite = loaded_data.inv
		ExplorerData.explorerRank = loaded_data.rank
		ExplorerData.systemsExplored = loaded_data.sysex
		ExplorerData.jumpsMade = loaded_data.jumps
		ExplorerData.lightyearsTraveled = loaded_data.travel
		ExplorerData.bountylist = loaded_data.bolist
		ExplorerData.hyperSource = loaded_data.hsrc
		loaded_data = nil
	end
	if Game.player:IsDocked() then
		onCreateBB(Game.player:GetDockedWith())
	end
end

Event.Register("onCreateBB", onCreateBB)
Event.Register("onGameStart", onGameStart)
Event.Register("onEnterSystem", onEnterSystem)
Event.Register("onLeaveSystem", onLeaveSystem)
Event.Register("onShipDocked", onShipDocked)
Event.Register("onSystemExplored", onSystemExplored)

local serialize = function()
	return {
		sysex = ExplorerData.systemsExplored,
		rank = ExplorerData.explorerRank,
		inv = ExplorerData.explorerInvite,
		jumps = ExplorerData.jumpsMade,
		travel = ExplorerData.lightyearsTraveled,
		bolist = ExplorerData.bountylist,
		hsrc = ExplorerData.hyperSource
	}
end

local unserialize = function(data)
	loaded_data = data
end

Serializer:Register("ExplorerClub", serialize, unserialize)
