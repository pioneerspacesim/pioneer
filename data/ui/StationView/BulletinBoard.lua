-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
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

local bbTable = ui:Table()
	:SetRowSpacing(5)
	:SetColumnSpacing(10)
	:SetRowAlignment("CENTER")
	:SetMouseEnabled(true)

bbTable.onRowClicked:Connect(function (row)
	local station = Game.player:GetDockedWith()
	local ref = rowRef[station][row+1]
	local ad = SpaceStation.adverts[station][ref]

	local chatFunc = function (form, option)
		return ad.onChat(form, ref, option)
	end
	local removeFunc = function ()
		station:RemoveAdvert(ref)
	end

	local form = ChatForm.New(chatFunc, removeFunc, ref, tabGroup)
	ui:NewLayer(form:BuildWidget())
end)

local updateTable = function (station)
	if Game.player:GetDockedWith() ~= station then return end

	bbTable:ClearRows()

	local adverts = SpaceStation.adverts[station]
	if not adverts then return end

	local rows = {}
	for ref,ad in pairs(adverts) do
		local icon = ad.icon or "default"
		local label = ui:Label(ad.description)
		if type(ad.isEnabled) == "function" and not ad.isEnabled(ref) then
			label:SetColor({ r = 0.4, g = 0.4, b = 0.4 })
		end
		table.insert(rows, {
			ui:Image("icons/bbs/"..icon..".png", { "PRESERVE_ASPECT" }),
			label,
		})
	end

	bbTable:AddRows(rows)
end

local updateRowRefs = function (station, ref)
	local adverts = SpaceStation.adverts[station]
	if not adverts then return end

	rowRef[station] = {}
	local rowNum = 1
	for ref,ad in pairs(adverts) do
		rowRef[station][rowNum] = ref
		rowNum = rowNum+1
	end

	updateTable(station)
end

Event.Register("onAdvertAdded", updateRowRefs)
Event.Register("onAdvertRemoved", updateRowRefs) -- XXX close form if open
Event.Register("onAdvertChanged", updateTable)

local bulletinBoard = function (args, tg)
	tabGroup = tg
	updateTable(Game.player:GetDockedWith())
	return bbTable
end

return bulletinBoard
