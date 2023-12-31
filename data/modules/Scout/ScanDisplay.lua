-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game        = require 'Game'
local Lang        = require 'Lang'
local ItemCard    = require 'pigui.libs.item-card'

local utils = require 'utils'
local ui = require 'pigui'

local lui = Lang.GetResource("ui-core")
local ls = Lang.GetResource("module-scout")

local colors = ui.theme.colors
local icons = ui.theme.icons

local pionillium = ui.fonts.pionillium

local sizes = ui.rescaleUI({
	leadIconSize = Vector2(32),
})

local gameView = require 'pigui.views.game'

---@class UI.ScanCard : UI.ItemCard
local ScanCard = utils.inherits(ItemCard, "UI.ScanCard")

ScanCard.detailFields = 3
ScanCard.rounding = 1
ScanCard.iconSize = sizes.leadIconSize

ScanCard.backgroundColor = ui.theme.styleColors.gray_800
ScanCard.hoveredColor = ui.theme.styleColors.gray_700
ScanCard.selectedColor = ui.theme.styleColors.primary_600

function ScanCard:drawTooltip(data, isHighlighted)
	if data.scan.complete then
		ui.setTooltip(ls.COMPLETED_SCAN_TOOLTIP)
	elseif data.isActive then
		ui.setTooltip(ls.ACTIVE_SCAN_TOOLTIP)
	elseif isHighlighted then
		ui.setTooltip(ls.AVAILABLE_SCAN_TOOLTIP)
	else
		ui.setTooltip(ls.PENDING_SCAN_TOOLTIP)
	end
end

function ScanCard:drawTitle(data, textWidth, isHighlighted)
	local pos = ui.getCursorScreenPos()
	local size = ui.calcTextSize(data.completion)
	local textArea = pos + Vector2(textWidth - size.x - self.lineSpacing.x, ui.getTextLineHeight())

	ui.withClipRect(pos, textArea, function()
		ui.text(data.title)
	end)

	ui.setCursorScreenPos(pos + Vector2(textWidth - size.x, 0))
	ui.text(data.completion)
end

local scanDisplay = {
	side = "left",
	icon = icons.equip_orbit_scanner,
	tooltip = ls.SCAN_MANAGER,
	exclusive = false,

	debugReload = function()
		package.reimport('pigui.libs.item-card')
		package.reimport()
	end,

	ship = nil,
	activeTab = 1

}

local minRes = 5
local coverage = 1

function scanDisplay:drawDebug()
	local scanMgr = self.scanMgr

	if ui.button("Add SScan") then
		self:addTestScan(self.scanMgr)
	end
	ui.sameLine()
	if ui.button("Add OScan") then
		self:addTestScan(self.scanMgr, true)
	end

	local sysBody = self.ship.frameBody:GetSystemBody()
	assert(sysBody)

	minRes = ui.dragFloat("MinRes", minRes, 1.0, 0.1, 100, "%f")
	coverage = ui.dragFloat("Coverage", coverage, 0.5, 0.5, 100, "%fkm^2")

	local activeScan = scanMgr:GetActiveScan()
	if activeScan then
		local altitude, resolution = scanMgr:GetBodyState(scanMgr:GetScanBodyByType())

		ui.text("Current Alt: " .. altitude)
		ui.text("Current Res: " .. resolution)

		ui.newLine()
	end

	local params = scanMgr:GetScanParameters(sysBody, minRes)

	if not params then
		ui.text("invalid parameters")
	else
		ui.text("Can Scan: " .. tostring(params.canScan))
		ui.text("Max Altitude: " .. params.maxAltitude)
		ui.text("Min Altitude: " .. params.minAltitude)
		ui.text("Orbits to Cover: " .. params.orbitsToCover)
	end

	ui.newLine()
	ui.text("State: " .. scanMgr:GetState())
	ui.newLine()

	ui.text("Num pending: " .. #scanMgr:GetPendingScans())
	ui.text("Num complete: " .. #scanMgr:GetCompletedScans())

	local active = scanMgr:GetActiveScan()
	if active then
		ui.newLine()
		ui.text("Active Scan:")

		ui.text("id: ", active.id)
		ui.text("coverage: " .. active.coverage)
		ui.text("target: " .. active.targetCoverage)
		ui.text("complete: " .. tostring(active.complete))
		ui.text("orbital: " .. tostring(active.orbital))
	end

end

---@param scanMgr ScanManager
function scanDisplay:addTestScan(scanMgr, orbit)
	if orbit then
		return scanMgr:AddNewOrbitalScan(self.ship.frameBody.path, minRes, coverage)
	else
		return scanMgr:AddNewSurfaceScan(self.ship.frameBody.path, minRes, coverage)
	end
end

function scanDisplay:drawEmptyActiveScan()
	ui.beginGroup()

	local pos = ui.getCursorScreenPos()
	local _, _, size = ScanCard:drawBackground(false)

	local text = ls.NO_AVAILABLE_SCANNER
	local tooltip = ls.NO_AVAILABLE_SCANNER_TOOLTIP

	if self.scanMgr:GetActiveSensor() then
		text = ls.NO_ACTIVE_SCAN
		tooltip = ls.NO_ACTIVE_SCAN_TOOLTIP
	end

	ui.withFont(pionillium.heading, function()
		local textSize = ui.calcTextSize(text)
		ui.setCursorScreenPos(pos + (size - textSize) * 0.5)

		ui.text(text)
	end)

	ui.endGroup()

	if ui.isItemHovered() then
		ui.setTooltip(tooltip)
	end
end

---@param scan ScanData
function scanDisplay:drawScanInfo(scan, isHighlighted)
	local sBody = scan.bodyPath:GetSystemBody()
	local scanMgr = self.ship:GetComponent("ScanManager")

	-- displayed when the active scanner cannot carry out the scan
	local altitude = string.upper(ls.INVALID_SCANNER)
	local target = ""

	local params = scanMgr:GetScanParameters(sBody, scan.minResolution, scan.orbital)
	if params and params.canScan then
		altitude = ui.Format.Distance(params.maxAltitude)
	end

	if scan.orbital then
		target = string.format("%.1f%%", scan.targetCoverage * 100.0)
	else
		target = ui.Format.Distance(scan.targetCoverage * 1000.0, "%.1f")
	end

	local completion = math.min(1.0, scan.coverage / scan.targetCoverage)

	local data = {
		title = sBody.name .. ", " .. scan.bodyPath:GetStarSystem().name,
		target = target,
		completion = string.format("%2.1f%%", completion * 100.0),
		isActive = self.scanMgr:GetActiveScan() == scan,
		scan = scan,
		icon = scan.orbital and icons.map or icons.scanner,
		{ icons.comms, target, ls.SCAN_TARGET_COVERAGE },
		{ icons.scanner, ui.Format.Distance(scan.minResolution, "%.1f"), ls.SCAN_MAXIMUM_SPATIAL_RESOLUTION },
		{ icons.altitude, altitude, ls.SCAN_MAXIMUM_ALTITUDE },
	}

	return ScanCard:draw(data, isHighlighted)
end

-- Return a sorted copy of the given scan list for display
---@param scanList ScanData[]
function scanDisplay:sortScanList(scanList)
	local pending = table.copy(scanList)

	local canBeActivated = utils.map_table(pending, function(_, scan)
		return scan, self.scanMgr:CanScanBeActivated(scan.id)
	end)

	table.sort(pending, function(a, b)
		if canBeActivated[a] ~= canBeActivated[b] then
			return canBeActivated[a]
		end

		if a.orbital ~= b.orbital then
			return a.orbital
		end

		return a.minResolution > b.minResolution
	end)

	return pending, canBeActivated
end

function scanDisplay:drawActiveSensor()
	local sensors = self.scanMgr:GetAvailableSensors()
	local activeSensor = self.scanMgr:GetActiveSensor()

	local index = 0
	local names = {}
	for i, s in ipairs(sensors) do
		if s == activeSensor then index = i end

		table.insert(names, sensors[i].equip:GetName())
	end

	-- TODO: display info about the scanner
	ui.nextItemWidth(-1.0)
	local changed, newIdx = ui.combo("##Active Sensor", index - 1, names)
	if changed then
		self.scanMgr:SetActiveSensor(newIdx + 1)
	end
end

function scanDisplay:drawTitle()
	ui.text(ls["STATE_" .. self.scanMgr:GetState()])
	ui.sameLine()

	-- Draw debug display button
	if self.showDebug or (ui.shiftHeld() and ui.altHeld()) then
		local buttonSize = Vector2(ui.getLineHeight() - ui.theme.styles.MainButtonPadding * 2)

		ui.addCursorPos(Vector2(ui.getContentRegion().x - buttonSize.x * 1.5, 0))
		if ui.mainMenuButton(icons.alert1, "Debug Display", false, buttonSize) then
			self.showDebug = not self.showDebug
		end
	end
end

function scanDisplay:drawBody()

	ui.spacing()

	local activeScan = self.scanMgr:GetActiveScan()

	-- Draw active scan area
	if activeScan then
		local clicked = self:drawScanInfo(activeScan, true)

		if clicked then
			self.scanMgr:ClearActiveScan()
		end
	else
		self:drawEmptyActiveScan()
	end

	-- Draw current sensor selector
	if #self.scanMgr:GetAvailableSensors() > 0 then
		ui.spacing()

		self:drawActiveSensor()
	end

	ui.spacing()
	ui.separator()
	ui.spacing()

	ui.withFont(pionillium.heading, function()
		ui.textAligned(ls.STORED_SCAN_DATA, 0.5)
	end)

	ui.spacing()

	-- Draw list of pending/complete scans

	local buttonSize = Vector2(ui.getContentRegion().x / 2, ui.getButtonHeight())
	local selectedBtn = ui.theme.buttonColors.selected
	local deselectedBtn = ui.theme.buttonColors.deselected

	if ui.button(ls.PENDING_SCANS % { #self.scanMgr:GetPendingScans() }, buttonSize, self.activeTab == 1 and selectedBtn or deselectedBtn) then
		self.activeTab = 1
	end
	ui.sameLine(0, 0)
	if ui.button(ls.COMPLETED_SCANS % { #self.scanMgr:GetCompletedScans() }, buttonSize, self.activeTab == 2 and selectedBtn or deselectedBtn) then
		self.activeTab = 2
	end

	if self.activeTab == 1 then
		local pending, canBeActivated = self:sortScanList(self.scanMgr:GetPendingScans())

		for _, scan in ipairs(pending) do
			if self:drawScanInfo(scan, canBeActivated[scan]) then
				self.scanMgr:SetActiveScan(scan.id)
			end
		end
	else
		local complete, _ = self:sortScanList(self.scanMgr:GetCompletedScans())
		for _, scan in ipairs(complete) do
			self:drawScanInfo(scan)
		end
	end

	if self.showDebug then
		ui.separator()

		self:drawDebug()
	end
end

function scanDisplay:refresh()
	self.ship = Game.player
	self.scanMgr = self.ship:GetComponent("ScanManager")
end

gameView.registerSidebarModule("scout.scanDisplay", scanDisplay)
