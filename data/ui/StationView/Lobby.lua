-- Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Game = import("Game")
local Rand = import("Rand")
local Character = import("Character")
local Lang = import("Lang")
local Comms = import("Comms")

local InfoFace = import("ui/InfoFace")
local MessageBox = import("ui/MessageBox")

local l = Lang.GetResource("ui-core")

local ui = Engine.ui

local lobby = function (tab)
	local station = Game.player:GetDockedWith()

	local rand = Rand.New(station.seed)
	local face = InfoFace.New(Character.New({ title = l.STATION_MANAGER }, rand))

	local launchButton = ui:Button(l.REQUEST_LAUNCH):SetFont("HEADING_LARGE")
	launchButton.onClick:Connect(function ()
		local crimes, fine = Game.player:GetCrimeOutstanding()

		if not Game.player:HasCorrectCrew() then
			MessageBox.Message(l.LAUNCH_PERMISSION_DENIED_CREW)
			Comms.ImportantMessage(l.LAUNCH_PERMISSION_DENIED_CREW, station.label)
		elseif fine > 0 then
			MessageBox.Message(l.LAUNCH_PERMISSION_DENIED_FINED)
			Comms.ImportantMessage(l.LAUNCH_PERMISSION_DENIED_FINED, station.label)
		elseif not Game.player:Undock() then
			MessageBox.Message(l.LAUNCH_PERMISSION_DENIED_BUSY)
			Comms.ImportantMessage(l.LAUNCH_PERMISSION_DENIED_BUSY, station.label)
		else
			Game.SwitchView()
		end
	end)

	local tech_certified

	if station.techLevel == 11 then
		tech_certified = l.TECH_CERTIFIED_MILITARY
	else
		tech_certified = string.interp(l.TECH_CERTIFIED, { tech_level = station.techLevel})
	end

	return
		ui:Grid({48,4,48},1)
			:SetColumn(0, {
				ui:VBox(10):PackEnd({
					ui:Label(station.label):SetFont("HEADING_LARGE"),
					ui:Align("LEFT", tech_certified),
					ui:Expand(),
					ui:Align("MIDDLE", launchButton),
				})
			})
			:SetColumn(2, {
				face.widget
			})
end

return lobby
