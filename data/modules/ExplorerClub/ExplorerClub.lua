-- Copyright � 2008-2021 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Character = require 'Character'
local Comms = require 'Comms'
local Event = require 'Event'
local FlightLog = require 'FlightLog'
local Format = require 'Format'
local Game = require 'Game'
local Lang = require 'Lang'
local NameGen = require 'NameGen'
local Rand = require 'Rand'

local ExplorerGlobals = require 'ExplorerGlobals'
local l = Lang.GetResource("module-explorerclub")

local ad = {}

local explorerChat = function (form, ref, option)
	form:Clear()
	form:SetFace(ad.npc)
	form:SetTitle(ad.title)
	local invitestatus = ExplorerGlobals.getExplorerInvite()
	local player = Character.persistent.player
	local bothcost = ExplorerGlobals.deviceCost + ExplorerGlobals.memberCost
	local substrings = {player = player.name, repname = ad.npcname, station = ad.station.label,
		clubname = l.EXPLORERS_CLUB, shortclub = l.SHORT_CLUB, device= l.SHORT_DEVICE,
		devicecost = Format.Money(ExplorerGlobals.deviceCost), membercost = Format.Money(ExplorerGlobals.memberCost),
		bothcost = Format.Money(bothcost)}
	-- initial dialog. varies if player has already joined (invitestatus)
	if option == 0 then
		if invitestatus == 0 then
			form:SetMessage(string.interp(l.BBS_INVITED_INTRO, substrings))
			form:AddOption(string.interp(l.BBSOPT_TELLMEMORE, substrings), 100)
			form:AddOption(l.BBSOPT_WHATFORME, 102)
			form:AddOption(l.BBSOPT_YESDIRECT, 101)
			form:AddOption(l.BBSOPT_NOTHANKS, 103)
		elseif invitestatus == 2 then
			form:SetMessage(string.interp(l.BBS_WELCOMESERVICES, substrings))
			form:AddOption(l.BBSOPT_SUBMITNEWDATA, 301)
			if Game.player:GetEquipCountOccupied('explorer_device') == 0 then
				form:AddOption(string.interp(l.BBSOPT_NEED_DEVICE, substrings), 125)
			end
		-- previously rejected maybe changed their mind?
		elseif invitestatus == 3 then
			form:SetMessage(l.BBS_REJECT_INTRO)
			form:AddOption(string.interp(l.BBSOPT_TELLMEMORE, substrings), 201)
			form:AddOption(l.BBSOPT_WHATFORME, 202)
			form:AddOption(l.BBSOPT_CHANGEDMYMIND, 106)
			form:AddOption(l.BBSOPT_STILLNOINTEREST, 203)
		else
			form:SetMessage(l.BBS_SYSTEM_ERROR)
		end
	-- options 100+ is only possible if invitestatus is 1
	elseif option == 100 then
		form:SetMessage(string.interp(l.BBS_CLUBSTORY, substrings))
		form:AddOption(l.BBSOPT_WHATFORME, 102)
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
		ExplorerGlobals.setExplorerInvite(3) -- 3 = initation rejected
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
--[[	form:SetMessage(string.interp(l.BBS_WELCOMESPEECH, substrings))
		form:AddOption(l.BBSOPT_THANKYOU, -1)
		ExplorerGlobals.setExplorerInvite(2) -- 2 = invitation accepted
		-- push a custom log entry to mark the occasion
		FlightLog.MakeCustomEntry(string.interp(l.LOG_EXPLORER_JOIN, substrings))
		Game.player:AddEquip(ExplorerGlobals:GetExplorerDevice())
		if Game.player:GetEquipCountOccupied('explorer_device') > 0 then
			Comms.Message(string.interp(l.COMM_EXPLORER_JOIN, substrings), l.EXPLORERS_CLUB)
		end
-- ]]
	-- only membership
	elseif option == 120 then
		local cash = Game.player:GetMoney()
		if (cash < ExplorerGlobals.memberCost) then
			form:SetMessage(string.interp(l.BBS_NOFUNDS, substrings))
		else
			Game.player:AddMoney(-ExplorerGlobals.memberCost)
			form:SetMessage(string.interp(l.BBS_WELCOME_ONLY, substrings))
			form:AddOption(l.BBSOPT_THANKYOU, -1)
			ExplorerGlobals.setExplorerInvite(2) -- 2 = player is now a member
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
			ExplorerGlobals.setExplorerInvite(2) -- 2 = player is now a member
			-- push a custom log entry to mark the occasion
			FlightLog.MakeCustomEntry(string.interp(l.LOG_EXPLORER_JOIN, substrings))
			Game.player:AddEquip(ExplorerGlobals:GetExplorerDevice())
			if Game.player:GetEquipCountOccupied('explorer_device') > 0 then
				Comms.Message(string.interp(l.COMM_EXPLORER_JOIN, substrings), l.EXPLORERS_CLUB)
			end
		end
	elseif option == 125 then
		local cash = Game.player:GetMoney()
		if (cash < ExplorerGlobals.deviceCost) then
			form:SetMessage(string.interp(l.BBS_NOFUNDS, substrings))
		else
			form:SetMessage(string.interp(l.BBS_NEW_DEVICE_INSTALLED, substrings))
			form:AddOption(l.BBSOPT_THANKYOU, -1)
			Game.player:AddMoney(-ExplorerGlobals.deviceCost)
			Game.player:AddEquip(ExplorerGlobals:GetExplorerDevice())
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
		form:SetMessage(l.BBS_NONEWDATAFOUND)
	end
end

local explorerDelete = function (ref)
	ad = {}
end

local onCreateBB = function (station)
	ad.station = Game.player:GetDockedWith()
	local rand = Rand.New(ad.station.seed + -2017)

	-- Explorers club listed only on some stations
	if (ad.station.isHomeworld == 1 or 15 > rand:Integer(100)) then
		local text = l.BBS_EXPLORERS_LOCAL_CONTACT
		if (ad.station.isHomeworld == 1) then
			text = l.BBS_EXPLORERS_HQ_CONTACT
		end
		ad.title = string.interp(text, {station = ad.station.label})
		ad.gender = (rand:Integer(1) == 1)
		ad.npcname = NameGen.FullName(ad.gender, rand)
		ad.npc = Character.New({name = ad.npcname, female = ad.gender, seed = station.seed + -2017})

		ad.station:AddAdvert({
			description = ad.title,
			icon        = "explorerclub",
			onChat      = explorerChat,
			onDelete    = explorerDelete
		})
	end
end
Event.Register("onCreateBB", onCreateBB)

local onGameStart = function ()
	ExplorerGlobals:Init()
	rand = nil
end
Event.Register("onGameStart", onGameStart)
