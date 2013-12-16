-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Game = import("Game")
local Rand = import("Rand")
local Character = import("Character")
local Lang = import("Lang")

local InfoFace = import("ui/InfoFace")

local l = Lang.GetResource("ui-core")

local ui = Engine.ui

local lobby = function (tab)
	local station = Game.player:GetDockedWith()

	local rand = Rand.New(station.seed)
	local face = InfoFace.New(Character.New({ title = l.STATION_MANAGER }, rand))

	return
		ui:Grid(2,1)
			:SetColumn(0, {
				ui:VBox(10)
					:PackEnd(ui:Label(station.label):SetFont("HEADING_LARGE"))
			})
			:SetColumn(1, {
				face.widget
			})
end

return lobby
