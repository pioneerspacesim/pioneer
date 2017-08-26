-- Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Game = import("Game")
local SpaceStation = import("SpaceStation")
local Event = import("Event")
local ChatForm = import("ChatForm")
local Lang = import("Lang")
local l = Lang.GetResource("core");

local ui = Engine.ui

local tabGroup

local rowRef = {}

local bbTable = ui:Table()
	:SetRowSpacing(5)
	:SetColumnSpacing(10)
	:SetRowAlignment("CENTER")
	:SetMouseEnabled(not Game.paused)
    
local bbSearchField = ui:TextEntry()
local searchText = "";

bbTable.onRowClicked:Connect(function (row)
	local station = Game.player:GetDockedWith()
	local ref = rowRef[station][row+1]
	local ad = SpaceStation.adverts[station][ref]

	local chatFunc = function (form, option)
		station:LockAdvert(ref)
		return ad.onChat(form, ref, option)
	end
	local removeFunc = function ()
		station:RemoveAdvert(ref)
	end
	local closeFunc = function ()
		station:UnlockAdvert(ref)
	end

	local form = ChatForm.New(chatFunc, removeFunc, closeFunc, ref, tabGroup)
	ui:NewLayer(form:BuildWidget())
end)

local updateTable = function (station)
	if Game.player:GetDockedWith() ~= station then return end

	bbTable:ClearRows()

	local adverts = SpaceStation.adverts[station]
	if not adverts then return end

	local rows = {}
    rowRef[station] = {}
    local rowNum = 1
	for ref,ad in pairs(adverts) do
		local icon = ad.icon or "default"
		local label = ui:Label(ad.description)
		if type(ad.isEnabled) == "function" and not ad.isEnabled(ref) then
			label:SetColor({ r = 0.4, g = 0.4, b = 0.4 })
		end
		if Game.paused then
			label:SetColor({ r = 0.3, g = 0.3, b = 0.3})
		end

        if searchText == "" 
            or searchText ~= "" and string.find(
                string.lower(ad.description),
                string.lower(searchText),
                1, true)
        then            
            table.insert(rows, {
                ui:Image("icons/bbs/"..icon..".png", { "PRESERVE_ASPECT" }),
                label,
            })
            
            rowRef[station][rowNum] = ref
            rowNum = rowNum+1
        end
        
	end

	bbTable:AddRows(rows)
end

local onGamePaused = function ()
	bbTable:SetMouseEnabled(false)
	updateTable(Game.player:GetDockedWith())
end

local onGameResumed = function ()
	bbTable:SetMouseEnabled(true)
	updateTable(Game.player:GetDockedWith())
end

bbSearchField.onChange:Connect(function (newSearchText)
    searchText = newSearchText
    updateTable(Game.player:GetDockedWith())
end)

Event.Register("onAdvertAdded", updateTable)
Event.Register("onAdvertRemoved", updateTable) -- XXX close form if open
Event.Register("onAdvertChanged", updateTable)
Event.Register("onGamePaused", onGamePaused)
Event.Register("onGameResumed", onGameResumed)

local bulletinBoard = function (args, tg)
	tabGroup = tg
	updateTable(Game.player:GetDockedWith())
	return 
        ui:Grid({79, 1, 20}, 1)
            :SetColumn(0, {bbTable})
            :SetColumn(2, {
                ui:VBox():PackEnd({
                    ui:Label(l.SEARCH):SetFont("HEADING_LARGE"),
                    ui:Expand("HORIZONTAL", bbSearchField),
                })
            })
end

return bulletinBoard
