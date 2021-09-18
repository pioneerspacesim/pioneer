-- Copyright � 2008-2021 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local Event = require 'Event'
local Rand = require 'Rand'
local NameGen = require 'NameGen'
local Comms = require 'Comms'
local Character = require 'Character'
local Lang = require 'Lang'
local Equipment = require 'Equipment'
local FlightLog = require 'FlightLog'

local ExplorerGlobals = require 'ExplorerGlobals'
local l = Lang.GetResource("module-explorerclub")

local ad = {}

local explorerChat = function (form, ref, option)
	form:Clear()
	form:SetFace(ad.npc)
	form:SetTitle(ad.title)
	local invitestatus = ExplorerGlobals.getExplorerInvite()
	local player = Character.persistent.player
	-- initial dialog. varies if player has already joined (invitestatus)
	if option == 0 then
		if invitestatus == 0 then -- for testing, real = 1
			form:SetMessage(string.interp(l.INVITED_INTRODUCTION, {repname = ad.npcname, clubname = l.EXPLORERS_CLUB}))
			form:AddOption(string.interp(l.OPT_TELLMEMORE, {clubname = l.EXPLORERS_CLUB}),100)
			form:AddOption(l.OPT_WHATFORME,102)
			form:AddOption(l.OPT_YESDIRECT,101)
			form:AddOption(l.OPT_NOTHANKS,103)
		elseif invitestatus == 2 then
			form:SetMessage(string.interp(l.WELCOMESERVICES, {repname = ad.npcname, clubname = l.EXPLORERS_CLUB, station = ad.station.label}))
			form:AddOption(l.OPT_SUBMITNEWDATA,301)
		-- previously rejected maybe changed their mind?
		elseif invitestatus == 3 then
			form:SetMessage(l.REJECT_INTRODUCTION)
			form:AddOption(string.interp(l.OPT_TELLMEMORE, {clubname = l.EXPLORERS_CLUB}),201)
			form:AddOption(l.OPT_WHATFORME,202)
			form:AddOption(l.OPT_CHANGEDMYMIND,106)
			form:AddOption(l.OPT_STILLNOINTEREST,203)
		else
			form:SetMessage(l.SYSTEM_ERROR)
		end
	-- options 100+ is only possible if invitestatus is 1
	elseif option == 100 then
		form:SetMessage(string.interp(l.CLUBSTORY, {clubname = l.EXPLORERS_CLUB, shortclub = l.SHORTCLUB}))
		form:AddOption(l.OPT_WHATFORME,102)
		form:AddOption(l.OPT_OKLETSDOIT,106)
		form:AddOption(l.OPT_NOTHANKS,103)
	-- unquestionable enthusiasm, join without question
	elseif option == 101 then
		form:SetMessage(string.interp(l.NOTHINGTOSIGN, {clubname = l.EXPLORERS_CLUB}))
		form:AddOption(l.OPT_WHATTOKEN,105)
		form:AddOption(l.OPT_OKLETSDOIT,106)
		form:AddOption(l.OPT_NOTHANKS,103)
	-- whats in it for me?
	elseif option == 102 then
		form:SetMessage(string.interp(l.EXPLAINBENEFITS, {shortclub = l.SHORTCLUB}))
		form:AddOption(l.OPT_WHATTOKEN,105)
		form:AddOption(l.OPT_OKLETSDOIT,106)
		form:AddOption(l.OPT_NOTHANKS,103)
	-- no thanks, dont join
	elseif option == 103 then
		form:SetMessage(l.REJECTIONTEXT)
		ExplorerGlobals.setExplorerInvite(3) -- 3 = initation rejected
	-- explain the membership token
	elseif option == 105 then
		form:SetMessage(string.interp(l.EXPLAINTOKEN, {shortclub = l.SHORTCLUB}))
		form:AddOption(l.OPT_OKLETSDOIT,106)
		form:AddOption(l.OPT_NOTHANKS,103)
	-- joining
	elseif option == 106 then
		form:SetMessage(string.interp(l.WELCOMESPEECH, {clubname = l.EXPLORERS_CLUB, shortclub = l.SHORTCLUB, pname = player.name}))
		form:AddOption(l.OPT_THANKYOU,-1)
		ExplorerGlobals.setExplorerInvite(2) -- 2 = invitation accepted
		-- push a custom log entry to mark the occasion
		FlightLog.MakeCustomEntry(string.interp(l.EXPLORERJOINLOGENTRY, {clubname = l.EXPLORERS_CLUB}))
		Game.player:AddEquip(ExplorerGlobals:GetClubDevice())
		if Game.player:GetEquipCountOccupied('explorer_device') > 0 then
			Comms.Message(string.interp(l.COMMSJOINEDMESSAGE, {clubname = l.EXPLORERS_CLUB}), l.EXPLORERS_CLUB)
		end
	-- tell me about the club
	elseif option == 201 then
		form:SetMessage(string.interp(l.CLUBSTORY, {clubname = l.EXPLORERS_CLUB, shortclub = l.SHORTCLUB}))
		form:AddOption(l.OPT_WHATFORME,202)
		form:AddOption(l.OPT_CHANGEDMYMIND,106)
		form:AddOption(l.OPT_STILLNOINTEREST,203)
	-- whats in it for me?
	elseif option == 202 then
		form:SetMessage(string.interp(l.EXPLAINBENEFITS, {clubname = l.EXPLORERS_CLUB, shortclub = l.SHORTCLUB}))
		form:AddOption(l.OPT_WHATTOKEN,205)
		form:AddOption(l.OPT_CHANGEDMYMIND,106)
		form:AddOption(l.OPT_STILLNOINTEREST,203)
	-- still dont want to join
	elseif option == 203 then
		form:SetMessage(l.REJECTAGAIN)
	-- explain the token
	elseif option == 205 then
		form:SetMessage(string.interp(l.EXPLAINTOKEN, {shortclub = l.SHORTCLUB}))
		form:AddOption(l.OPT_CHANGEDMYMIND,106)
		form:AddOption(l.OPT_STILLNOINTEREST,203)
	elseif option == 301 then
		-- WIP to be continued...
		form:SetMessage(l.NONEWDATAFOUND)
	end
end

local explorerDelete = function (ref)
	ad = {}
end

local onCreateBB = function (station)
	--if x.getExplorerInvite == 0 then return end	-- explorer club not automatically listed
	ad.station = Game.player:GetDockedWith()
	ad.title = string.interp(l.EXPLORERS_CLUB_LOCAL_CONTACT, {station = ad.station.label})
	local rand = Rand.New(ad.station.seed + -2017)
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
Event.Register("onCreateBB", onCreateBB)

local onGameStart = function ()
	ExplorerGlobals:Init()
	rand = nil
end
Event.Register("onGameStart", onGameStart)
