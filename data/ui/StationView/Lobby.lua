-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Game = import("Game")
local Rand = import("Rand")
local Character = import("Character")
local Lang = import("Lang")
local Comms = import("Comms")

local InfoFace = import("ui/InfoFace")

local l = Lang.GetResource("ui-core")

-- XXX for LAUNCH_PERMISSION_DENIED_BUSY, still needed by WorldView
local lcore = Lang.GetResource("core")

local ui = Engine.ui

local lobby = function (tab)
	local station = Game.player:GetDockedWith()

	local rand = Rand.New(station.seed)
	local face = InfoFace.New(Character.New({ title = l.STATION_MANAGER }, rand))

	local launchButton = ui:Button(l.REQUEST_LAUNCH):SetFont("HEADING_LARGE")
	launchButton.onClick:Connect(function ()
		if not Game.player:Undock() then
			Comms.ImportantMessage(station.label, lcore.LAUNCH_PERMISSION_DENIED_BUSY)
		else
			Game.SwitchToWorldView()
		end
	end)

	return
		ui:Grid({48,4,48},1)
			:SetColumn(0, {
				ui:VBox(10):PackEnd({
					ui:Label(station.label):SetFont("HEADING_LARGE"),
					ui:Expand(),
					ui:Align("MIDDLE", launchButton),
				})
			})
			:SetColumn(2, {
				face.widget
			})
end

return lobby
