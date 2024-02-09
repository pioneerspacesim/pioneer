-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Character	= require 'Character'
local Game		= require 'Game'
local Lang		= require 'Lang'
local Mission	= require 'Mission'
local Space		= require 'Space'

local ui		= require 'pigui'
local textTable	= require 'pigui.libs.text-table'
local PiGuiFace	= require 'pigui.libs.face'
local InfoView	= require 'pigui.views.info-view'

local pionillium = ui.fonts.pionillium
local orbiteer = ui.fonts.orbiteer
local l = Lang.GetResource("ui-core")

local columnPadding = ui.rescaleUI(Vector2(18, 18), Vector2(1600, 900))
local itemSpacing = ui.rescaleUI(Vector2(6, 12), Vector2(1600, 900))
local detailsSpacing = ui.rescaleUI(Vector2(6, 10), Vector2(1600, 900))
local framePadding = ui.rescaleUI(Vector2(8, 4), Vector2(1600, 900))
local face = nil

local function setLocationAsTarget(location)
	if location:isa("Body") and location:IsDynamic() then
		Game.player:SetNavTarget(location)
		ui.playSfx("OK")
	elseif Game.system and location:IsSameSystem(Game.system.path) then
		if location.bodyIndex then
			Game.player:SetNavTarget(Space.GetBody(location.bodyIndex))
			ui.playSfx("OK")
		end
	elseif not Game.InHyperspace() then
		-- if a specific systembody is given, set the sector map to the correct star (if the system is multiple)
		Game.sectorView:SwitchToPath(location:IsBodyPath() and location:GetSystemBody().nearestJumpable.path or location:GetStarSystem().path)
		ui.playBoinkNoise()
	end
end

local activeMission = nil
local function drawMissionDescription(missionDesc)
	local contentRegion = ui.getContentRegion()
	local leftColWidth = contentRegion.x / 1.618 - columnPadding.x / 2
	ui.child("MissionDetailsColumn", Vector2(leftColWidth, 0), function()
		ui.withFont(orbiteer.heading, function() ui.text(l.MISSION_DETAILS) end)
		ui.newLine()

		if missionDesc.description then
			ui.textWrapped(missionDesc.description or "<ERROR> No Description Available.")
			ui.newLine()
		end

		ui.withStyleVars({ItemSpacing=detailsSpacing}, function()
			textTable.draw(missionDesc.details)
		end)
		ui.newLine()

		if missionDesc.customDetails then
			missionDesc:customDetails()
			ui.newLine()
		end

		if ui.button(l.GO_BACK, Vector2(0, 0)) then
			activeMission = nil
		end

		if missionDesc.location then
			ui.sameLine()
			if ui.button(l.SET_AS_TARGET, Vector2(0, 0)) then setLocationAsTarget(missionDesc.location) end
		end

		if missionDesc.returnLocation then
			ui.sameLine()
			if ui.button(l.SET_RETURN_ROUTE, Vector2(0, 0)) then setLocationAsTarget(missionDesc.returnLocation) end
		end
	end)

	ui.sameLine(0, columnPadding.x)

	ui.child("MissionGiverFace", Vector2(0, 0), function()
		if not face or face.character ~= missionDesc.client then
			face = PiGuiFace.New(missionDesc.client)
		end

		face:render()
	end)
end

local rowCache = nil

local function makeMissionRows()
	rowCache = {
		separated = true,
		{ l.TYPE, l.CLIENT, l.LOCATION, l.DUE, l.REWARD, l.STATUS, font = orbiteer.heading }
	}

	for _, mission in pairs(Character.persistent.player.missions) do
		local locationName = mission.location:GetStarSystem().name -- ui.Format.SystemPath(mission.location)
		if mission.location.bodyIndex then
			locationName = mission.location:GetSystemBody().name .. ", " .. locationName
		end

		local playerSystem = Game.system or Game.player:GetHyperspaceTarget()
		local days = math.max(0, (mission.due - Game.time) / (24*60*60))

		-- Use AU for interplanetary, LY for interstellar distances
		local dist, dist_display
		if mission.location:IsSameSystem(playerSystem.path) then
			if mission.location:IsBodyPath() then
				local body = mission.location:GetSystemBody().body
				dist = Game.player:GetPositionRelTo(body):length()
				dist_display = "\n" .. ui.Format.Distance(dist)
			else
				dist_display = "\n-"
			end
		else
			dist = playerSystem:DistanceTo(mission.location)
			dist_display = string.format("\n%.2f %s", dist, l.LY)
		end

		local row = {
			mission:GetTypeDescription(),
			mission.client.name,
			locationName .. dist_display,
			ui.Format.Date(mission.due) .."\n".. string.format(l.D_DAYS_LEFT, days),
			ui.Format.Money(mission.reward),
		}

		local makeForm = mission:GetClick()
		if makeForm then
			row[6] = function()
				ui.text(mission.status and l[mission.status] or l.INACTIVE)
				ui.sameLine(ui.getColumnWidth() - (framePadding.x * 2) - ui.calcTextSize(l.MORE_INFO).x)
				if ui.button(l.MORE_INFO.."##"..tostring(mission), Vector2(0, 0)) then
					activeMission = makeForm(mission)
				end
			end
		end

		table.insert(rowCache, row)
	end
end

InfoView:registerView({
    id = "missions",
    name = l.MISSIONS,
    icon = ui.theme.icons.star,
    showView = true,
	draw = function()
		ui.withFont(pionillium.body, function()
			if not rowCache then makeMissionRows() end
			if activeMission then
				drawMissionDescription(activeMission)
				return
			end

			ui.withStyleVars({ItemSpacing = itemSpacing, FramePadding = framePadding}, function()
				textTable.drawTable(6, nil, rowCache)
			end)
		end)
	end,
	refresh = function()
		rowCache = nil
		activeMission = nil
	end,
	debugReload = function()
		package.reimport()
	end
})
