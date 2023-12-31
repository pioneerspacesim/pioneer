-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = require 'pigui'
local Defs = require 'pigui.modules.new-game-window.defs'
local GameParam = require 'pigui.modules.new-game-window.game-param'
local Lang = require 'Lang'
local lc = Lang.GetResource("core")
local lui = Lang.GetResource("ui-core")

local Widgets = require 'pigui.modules.new-game-window.widgets'
local SectorMap = require 'SectorMap'
local SystemPath = require 'SystemPath'
local SystemOverviewWidget = require 'pigui.modules.system-overview-window'

local mapLayout, timeLayout, pathLayout


--
-- time
--
-- value: number, game time, 0 means 3200-01-01 00:00:00
--
local Time = GameParam.New(lui.GAME_TIME, "time")
Time.value = 0

Time.standard = false

function Time:pseudoDateTime()
	local year, month, day, hour, minute, second = util.gameTimeToTimeParts(self.value)
	return year * 10000 + month * 100 + day, hour * 10000 + minute * 100 + second
end

-- number 123456 -> numbers 12, 34, 56
local function threeParts(pvalue)
	local p3 = pvalue % 100
	local p2 = (pvalue - p3) % 10000
	local p1 = pvalue - p2 - p3
	return p1 * 0.0001, p2 * 0.01, p3
end

local function formatPseudoValue(pval, sep)
	local p1, p2, p3 = threeParts(pval)
	return tostring(p1) .. sep .. string.format('%.2d', p2) .. sep .. string.format('%.2d', p3)
end

function Time:draw()
	Widgets.centeredIn(timeLayout, function()
		ui.beginGroup()
		ui.alignTextToFramePadding()
		ui.text(lc.TIME_POINT)
		ui.sameLine()
		local pdate, ptime = self:pseudoDateTime()
		ui.nextItemWidth(timeLayout.dateWidth)
		local lockBecauseStandard = self.standard or self.lock
		local val_date, ch_date = Widgets.incrementDrag(lockBecauseStandard, "##startdate", pdate, 1, 0, 4000000000, formatPseudoValue(pdate, "-"))
		ui.sameLine()
		ui.text("-")
		ui.sameLine()
		ui.nextItemWidth(timeLayout.timeWidth)
		local ITEM_CHANGED = 1
		local ITEM_CHANGED_BY_TYPING = 2
		local val_time, ch_time = Widgets.incrementDrag(lockBecauseStandard, "##starttime", ptime, 1, 0, 235959, formatPseudoValue(ptime, ":"))
		if ch_time or ch_date then
			local year, month, day = threeParts(pdate)
			local hour, minute, second = threeParts(ptime)
			-- change only smaller units so that no errors occur
			if ch_date == ITEM_CHANGED then
				day = day + val_date - pdate
			elseif ch_date == ITEM_CHANGED_BY_TYPING then
				year, month, day = threeParts(val_date)
			end
			if ch_time == ITEM_CHANGED then
				second = second + val_time - ptime
			elseif ch_time == ITEM_CHANGED_BY_TYPING then
				hour, minute, second = threeParts(val_time)
			end
			-- seconds and days can be negative, but I don’t want to remove assertions in the function for other cases
			self.value = util.timePartsToGameTime(year, month, 1, hour, minute, 0) + 86400 * (day - 1) + second
		end
		if self.standard then
			self.value = util.standardGameStartTime()
		end
		local ch,value = ui.checkbox(lui.STANDARD_GAME_START_TIME, self.standard)
		if ch and (not self.lock) then
			self.standard = value
		end
		ui.endGroup()
	end)
end

function Time:fromStartVariant(variant)
	self.value = 0
	self.standard = true
	self:setLock(true)
end

function Time:isValid()
	if self.standard then
		self.value = util.standardGameStartTime()
	end
	return self.value >= 0
end

--
-- location
--
local Location = GameParam.New(lui.LOCATION, "location")

local State = {
	ORBIT = 0,
	DOCKED = 1,
	UNKNOWN = 2
}

local StateLabels = {
	[0] = lui.ORBIT,
	[1] = lc.DOCKED,
	[2] = lc.UNKNOWN
}

Location.value = {
	path = SystemPath.New(0, 0, 0),
	state = State.DOCKED
}

Location.systems = {}
Location.sysCombo = { systems = {}, labels = {}, selected = 0 }
Location.sysSelected = 0
Location.bodySelected = true
Location.galaxy = false
Location.map = false
Location.overview = false
Location.stateLabel = StateLabels[State.UNKNOWN]

function Location:setPath(path)

	assert(path)

	self.value.path = path

	self.sysCombo = { systems = self.galaxy:GetSector(path.sectorX, path.sectorY, path.sectorZ), labels = {}, selected = 0 }
	for _, system in ipairs(self.sysCombo.systems) do
		table.insert(self.sysCombo.labels, system.name)
	end

	table.insert(self.sysCombo.systems, 1, false)
	table.insert(self.sysCombo.labels, 1, lui.SELECT)

	self.sysCombo.selected = path:IsSectorPath() and 0 or path.systemIndex + 1

	local system = self.sysCombo.systems[self.sysCombo.selected + 1]

	if path:IsBodyPath() then
		self.bodySelected = system:GetBodyByPath(path)
	else
		self.bodySelected = false
	end

	if system then
		self.map:GotoSystemPath(self.sysCombo.systems[self.sysCombo.selected + 1].path)
	else
		self.map:GotoSectorPath(path:SectorOnly())
	end

	if self.bodySelected then
		local sbody = self.bodySelected
		if sbody.superType == 'STARPORT' then
			self.value.state = State.DOCKED
		else
			self.value.state = State.ORBIT
		end
	else
		self.value.state = State.UNKNOWN
	end
	self.stateLabel = StateLabels[self.value.state]
end

function Location:onClickSystem(path)
	if not self.lock then
		self:setPath(path)
	end
end

function Location:updateLayout()
	timeLayout = {
		child_id = "timebar",
		width = Defs.contentRegion.x * 0.6,
		height = Defs.lineHeight * 3,
		h_indent = 0,
		v_indent = 0,
		dateWidth = ui.calcTextSize("<-- 3200-00-00 -->").x,
		timeWidth = ui.calcTextSize("<-- 00-00-00 -->").x
	}
	mapLayout = {
		width = timeLayout.width,
		height = Defs.contentRegion.y - timeLayout.height - ui.getItemSpacing().y,
	}
	pathLayout = {
		labelWidth = 0,
		width = Defs.contentRegion.x - mapLayout.width - ui.getItemSpacing().x
	}
	if not self.map then self:initMap() end
	self:setPath(self.value.path)
end

function Location:initMap()
	self.map = SectorMap(function (path) self:onClickSystem(path) end)
	self.map:SetSize(Vector2(mapLayout.width, mapLayout.height))
	self.galaxy = self.map:GetGalaxy()
	self.overview = SystemOverviewWidget.New()
	self.overview.onBodySelected = function(_, sbody, _)
		self:onClickSystem(sbody.path)
	end
end

function Location:draw()

	if not self.map then return end

	ui.child("location_leftpanel", Vector2(mapLayout.width, Defs.contentRegion.y), function()
		self.map:Draw()
		Time:draw()
	end)
	ui.sameLine()

	ui.child("location_rightpanel", Vector2(Defs.contentRegion.x - mapLayout.width - Defs.gap.x, Defs.contentRegion.y), function()

		Widgets.alignLabel("Sector", pathLayout, function()
			ui.nextItemWidth(Defs.secWidth)
			local secX, changedX = Widgets.incrementDrag(self.lock, "##dragsectorX", self.value.path.sectorX, 1, -9375, 3125, "%.0f")
			ui.sameLine()
			ui.nextItemWidth(Defs.secWidth)
			local secY, changedY = Widgets.incrementDrag(self.lock, "##dragsectorY", self.value.path.sectorY, 1, -6250, 6250, "%.0f")
			ui.sameLine()
			ui.nextItemWidth(Defs.secWidth)
			local secZ, changedZ = Widgets.incrementDrag(self.lock, "##dragsectorZ", self.value.path.sectorZ, 1, -256, 256, "%.0f")
			if changedX or changedY or changedZ then
				self:setPath(SystemPath.New(secX, secY, secZ))
			end
		end)

		Widgets.alignLabel(lc.SYSTEM, pathLayout, function()
			local combo = self.sysCombo
			local changed, ret = Widgets.combo(self.lock, "##systems_in_sector", combo.selected, combo.labels)
			if changed then
				combo.selected = ret
				local system = combo.systems[ret+1]
				if system then
					self:setPath(system.path)
				else
					self:setPath(self.value.path:SectorOnly())
				end
			end
		end)

		local system = self.sysCombo.systems[self.sysCombo.selected + 1]
		if system then ui.textWrapped(system.shortDescription) end

		Widgets.alignLabel(lc.BODY .. " ", pathLayout, function()
			ui.text(self.bodySelected and self.bodySelected.name or lc.NONE)
		end)

		Widgets.alignLabel(lui.STATUS .. " ", pathLayout, function()
			ui.text(self.stateLabel)
		end)

		if system then
			self.overview:display(system, nil, { [self.bodySelected] = true })
		end
	end)
end

function Location:fromStartVariant(variant)

	self.value = {
		path = variant.location,
		state = State.UNKNOWN
	}
	self:setLock(true)
end

function Location:isValid()
	return self.bodySelected and true or false
end

Location.Time = Time
Location.TabName = lui.LOCATION

return Location
