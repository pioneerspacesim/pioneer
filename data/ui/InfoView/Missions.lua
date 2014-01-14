-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Lang = import("Lang")
local Game = import("Game")
local Format = import("Format")
local Character = import("Character")

local SmallLabeledButton = import("ui/SmallLabeledButton")
local SmartTable = import("ui/SmartTable")

local ui = Engine.ui
local l = Lang.GetResource("ui-core");

-- we keep MissionList to remember players preferences
-- (now it is column he wants to sort by)
local MissionList 
local missions = function (tabGroup)
	-- This mission screen
	local MissionScreen = ui:Expand()

	if #Character.persistent.player.missions == 0 then
		return MissionScreen:SetInnerWidget( ui:Label(l.NO_MISSIONS) )
	end

	local rowspec = {7,8,9,9,5,5,5} -- 7 columns
	if MissionList then 
		MissionList:Clear()
	else
		MissionList = SmartTable.New(rowspec) 
	end
	
	-- setup headers
	local headers = 
	{
		l.TYPE,
		l.CLIENT,
		l.LOCATION,
		l.DUE,
		l.REWARD,
		l.STATUS,
	}
	MissionList:SetHeaders(headers)
	
	-- we're not happy with default sort function so we specify one by ourselves
	local sortMissions = function (misList)
		local col = misList.sortCol
		local cmpByReward = function (a,b) 
			return a.data[col] >= b.data[col] 
		end
		local comparators = 
		{ 	-- by column num
			[5]	= cmpByReward,
		}
		misList:defaultSortFunction(comparators[col])
	end
	MissionList:SetSortFunction(sortMissions)

	for ref,mission in pairs(Character.persistent.player.missions) do
		-- Format the location
		local missionLocationName
		if mission.location.bodyIndex then
			missionLocationName = string.format('%s, %s [%d,%d,%d]', mission.location:GetSystemBody().name, mission.location:GetStarSystem().name, mission.location.sectorX, mission.location.sectorY, mission.location.sectorZ)
		else
			missionLocationName = string.format('%s [%d,%d,%d]', mission.location:GetStarSystem().name, mission.location.sectorX, mission.location.sectorY, mission.location.sectorZ)
		end
		-- Format the distance label
		local playerSystem = Game.system or Game.player:GetHyperspaceTarget()
		local dist = playerSystem:DistanceTo(mission.location)
		local distLabel = ui:Label(string.format('%.2f %s', dist, l.LY))
		local hyperjumpStatus = Game.player:GetHyperspaceDetails(mission.location)
		if hyperjumpStatus == 'CURRENT_SYSTEM' then
			distLabel:SetColor({ r = 0.0, g = 1.0, b = 0.2 }) -- green
		else
			if hyperjumpStatus == 'OK' then
				distLabel:SetColor({ r = 1.0, g = 1.0, b = 0.0 }) -- yellow
			else
				distLabel:SetColor({ r = 1.0, g = 0.0, b = 0.0 }) -- red
			end
		end
		-- Pack location and distance
		local locationBox = ui:VBox(2):PackEnd(ui:MultiLineText(missionLocationName))
									  :PackEnd(distLabel)
		
		-- Format Due info
		local dueLabel = ui:Label(Format.Date(mission.due))
		local days = math.max(0, (mission.due - Game.time) / (24*60*60))
		local daysLabel = ui:Label(string.format(l.D_DAYS_LEFT, days)):SetColor({ r = 1.0, g = 0.0, b = 1.0 }) -- purple
		local dueBox = ui:VBox(2):PackEnd(dueLabel):PackEnd(daysLabel)
		
		local moreButton = SmallLabeledButton.New(l.MORE_INFO)
		moreButton.button.onClick:Connect(function ()
			MissionScreen:SetInnerWidget(ui:VBox(10)
				:PackEnd({ui:Label(l.MISSION_DETAILS):SetFont('HEADING_LARGE')})
				:PackEnd((mission:GetClick())(mission)))
		end)

		local description = mission:GetTypeDescription()
		local row =
		{ -- if we don't specify widget, default one will be used 
			{data = description or l.NONE},
			{data = mission.client.name},
			{data = dist, widget = locationBox},
			{data = mission.due, widget = dueBox},
			{data = mission.reward, widget = ui:Label(Format.Money(mission.reward)):SetColor({ r = 0.0, g = 1.0, b = 0.2 })}, -- green
			-- nil description means mission type isn't registered.
			{data = (description and l[mission.status]) or l.INACTIVE},
			{widget = moreButton.widget}
		}
		MissionList:AddRow(row)
	end

	MissionScreen:SetInnerWidget(MissionList)

	return MissionScreen
end

return missions
