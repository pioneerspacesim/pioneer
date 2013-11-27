-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Game = import("Game")
local SpaceStation = import("SpaceStation")
local Event = import("Event")
local ChatForm = import("ChatForm")
local utils = import("utils")

local ui = Engine.ui

local tabGroup

local rowRef = {}

local bbTable = ui:Table():SetMouseEnabled(true)
bbTable.onRowClicked:Connect(function (row)
	local station = Game.player:GetDockedWith()

	local ref = rowRef[row+1]
	local onChat = SpaceStation.adverts[station][ref][2]
	local onDelete = SpaceStation.adverts[station][ref][3]

	local chatFunc = function (form, option)
		return onChat(form, ref, option)
	end
	local removeFunc = onDelete and function ()
		station:RemoveAdvert(ref)
	end or nil

	local form = ChatForm.New(chatFunc, removeFunc, ref, tabGroup)
	ui:NewLayer(form:BuildWidget())
end)

local updateTable = function (station, ref)
	if Game.player:GetDockedWith() ~= station then return end

	bbTable:ClearRows()

	local adverts = SpaceStation.adverts[station]
	if not adverts then return end

	rowRef = {}
	local rows = {}
	local rowNum = 1
	for ref,ad in pairs(adverts) do
		rowRef[rowNum] = ref
		rowNum = rowNum+1
		table.insert(rows, ad[1])
	end

	bbTable:AddRows(rows)
end

Event.Register("onAdvertAdded", updateTable)
Event.Register("onAdvertRemoved", updateTable) -- XXX close form if open

local bulletinBoard = function (args, tg)
	tabGroup = tg
	return bbTable
end

return bulletinBoard
