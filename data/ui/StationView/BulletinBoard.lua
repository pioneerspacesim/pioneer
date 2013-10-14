-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Game = import("Game")
local SpaceStation = import("SpaceStation")
local ChatForm = import("ChatForm")
local utils = import("utils")

local ui = Engine.ui

local bulletinBoard = function (args)
	local station = Game.player:GetDockedWith()

	local bbTable =
		ui:Table()
			:SetMouseEnabled(true)

	bbTable:AddRows(utils.build_array(utils.map(function (k,v) return k,{v[1]} end, ipairs(SpaceStation.adverts[station]))))

	bbTable.onRowClicked:Connect(function (row)
		local ref = row+1
		local onChat = SpaceStation.adverts[station][ref][2]
		local onDelete = SpaceStation.adverts[station][ref][3]
        local chatFunc = function (form, option)
			return onChat(form, ref, option)
		end
		local removeFunc = onDelete and function ()
			station:RemoveAdvert(ref)
		end or nil
		local form = ChatForm.New(chatFunc, removeFunc)
		ui:NewLayer(form:BuildWidget())
	end)

	return bbTable
end

return bulletinBoard
