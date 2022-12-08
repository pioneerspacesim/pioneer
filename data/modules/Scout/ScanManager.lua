-- Copyright © 2008-2022 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Event = require 'Event'
local Timer = require 'Timer'
local Serializer = require 'Serializer'

local utils = require 'utils'

--=============================================================================

---@alias ScanID string

---@class ScanData
--
---@field id ScanID
---@field bodyPath SystemPath
---@field minResolution number
---@field targetCoverage number
---@field orbital boolean
---@field coverage number
---@field complete boolean
--
local ScanData = utils.inherits(nil, "Scout.ScanData")

---@return ScanData
function ScanData.New(id, bodyPath, minResolution, minCoverage, isOrbital)
	return setmetatable({
		id = id,
		bodyPath = bodyPath,
		minResolution = minResolution,
		targetCoverage = minCoverage,
		orbital = isOrbital,
		coverage = 0.0,
		complete = false
	}, ScanData.meta)
end

--=============================================================================

-- Update different scan types at different rates
local SURFACE_SCAN_UPDATE_RATE = 1
local ORBITAL_SCAN_UPDATE_RATE = 60

-- Square meters to square kilometers
local SQUARE_KILOMETERS = 10^6

local function makeScanID(count, path, orbital)
	return string.format("scan-%d-%s-%s", count, path, orbital and "orbit" or "surface")
end

--=============================================================================

---@class ScanManager
---@field New fun(s: Ship): ScanManager
local ScanManager = utils.class("Scout.ScanManager")

---@enum ScanManager.State
ScanManager.State = {
	Inactive = "INACTIVE",
	Scanning = "SCANNING",
	OutOfRange = "OUT_OF_RANGE",
	Completed = "COMPLETED",
	NoSensors = "NO_SENSORS",
}

---@param ship Ship
function ScanManager:Constructor(ship)
	ship.equipSet:AddListener(function(slot)
		if slot == "sensor" then
			self:UpdateSensorEquipInfo()
		end
	end)

	self.ship = ship

	---@type ScanData
	self.activeScan = nil
	---@type ScanData[]
	self.pendingScans = {}
	---@type ScanData[]
	self.completedScans = {}

	---@type table<ScanID, ScanData>
	self.scanMap = {}
	self.scanId = 1

	-- Do we have a sensor onboard this craft?
	-- TODO: allow multiple independent or supporting scanners
	self.hasSensor = false

	-- is the ship's current position within viable scan parameters?
	self.withinParameters = false

	-- calculate the distance the ship has moved since the last scan step
	---@type Vector3
	self.lastScanPos = nil

	self:UpdateSensorEquipInfo()
end

---@private
-- Scan the ship's equipment and determine its sensor capabilities
function ScanManager:UpdateSensorEquipInfo()
	local equip = self.ship.equipSet

	local sensor = equip:Get("sensor", 1)
	if not sensor then
		self.hasSensor = false
		return
	end

	-- TODO: support multiple disparate sensors on the same craft

	-- width of the aperture at one meter
	self.apertureWidth = math.tan(math.deg2rad(sensor.stats.aperture) * 0.5) * 2
	-- resolution in meters / sample at one meter
	self.resolutionMs = self.apertureWidth / sensor.stats.resolution
	-- minimum altitude required to get sensor data
	self.minAltitude = sensor.stats.minAltitude

	self.hasSensor = true
end

---@param body Body
---@return number altitude
---@return number resolution
function ScanManager:GetBodyState(body)
	local altitude
	local radius = body:GetSystemBody().radius

	if self.activeScan.orbital then
		-- altitude above sea-level
		altitude = self.ship:GetPositionRelTo(body):length() - radius
	else
		-- altitude above terrain
		altitude = self.ship:GetAltitudeRelTo(body)
	end

	-- calculate effective resolution in meters/sample at this altitude
	local resolution = self.resolutionMs * altitude
	return altitude, resolution
end

--=============================================================================

function ScanManager:GetHasSensor()
	return self.hasSensor
end

---@return ScanManager.State
function ScanManager:GetState()
	if not self.hasSensor then
		return self.State.NoSensors
	end

	if self.activeScan then
		local altitude, resolution = self:GetBodyState(self.ship.frameBody)
		local isInRange = resolution <= self.activeScan.minResolution and altitude > self.minAltitude

		return isInRange and self.State.Scanning or self.State.OutOfRange
	else
		return (#self.completedScans > 0 and #self.pendingScans == 0) and
			self.State.Completed or
			self.State.Inactive
	end
end

-- Determine if the given scan can be carried out by this craft,
-- and the maximum allowable mission parameters for the scan
---@param sBody SystemBody body to be scanning
---@param minResolution number minimum scan resolution required, effectively meters/sample
function ScanManager:GetScanParameters(sBody, minResolution)
	if not self.hasSensor then return nil end

	-- maximum altitude for the given resolution
	local maxAlt = minResolution / self.resolutionMs
	-- one-half circumference due to orbit covering the entire sphere
	local bodyHalfCirc = sBody.radius * math.pi

	-- Amount of body coverage per orbit
	-- Math is a little bit cheat-y here, would be better to use great circle distance rather than straight-line
	local coverage = self.apertureWidth * maxAlt
	-- number of orbits required to achieve full coverage of the body
	local orbitToCov = bodyHalfCirc / coverage

	return {
		-- whether the given scan can actually be carried out
		canScan = maxAlt > self.minAltitude,
		-- the maximum altitude the ship can fly at to scan the body
		maxAltitude = maxAlt,
		-- the minimum altitude the ship can fly at to scan the body
		minAltitude = self.minAltitude,
		-- the number of orbits at maximum altitude required to fully cover the body
		orbitsToCover = orbitToCov
	}
end

-- Return the list of pending scans for display purposes.
---@return ScanData[]
function ScanManager:GetPendingScans()
	return self.pendingScans
end

-- Return the list of completed scans for display purposes.
---@return ScanData[]
function ScanManager:GetCompletedScans()
	return self.completedScans
end

-- Return the active scan for display purposes.
---@return ScanData?
function ScanManager:GetActiveScan()
	return self.activeScan
end

--=============================================================================

-- Add a new surface scan mission profile to the scan manager.
-- Allows the ship's scanner(s) to begin gathering data once the scan is activated.
---@param bodyPath SystemPath path of the body to scan
---@param minResolution number minimum scanner resolution required to accept scan data
---@param requiredCoverage number how many square kilometers need to be covered for this scan to be considered complete
---@return ScanID ID used to look up the resulting scan datum
function ScanManager:AddNewSurfaceScan(bodyPath, minResolution, requiredCoverage)
	local id = makeScanID(self.scanId, bodyPath, false)
	self.scanId = self.scanId + 1

	local scanData = ScanData.New(id, bodyPath, minResolution, requiredCoverage, false)

	self.scanMap[id] = scanData
	table.insert(self.pendingScans, scanData)

	return id
end

-- Add a new orbital scan mission profile to the scan manager.
-- Allows the ship's scanner(s) to begin gathering data once the scan is activated.
---@param bodyPath SystemPath path of the body to scan
---@param minResolution number minimum scanner resolution required to accept scan data
---@param requiredCoverage number the percentage of the body's surface that needs to be scanned
---@return ScanID ID used to look up the resulting scan datum
function ScanManager:AddNewOrbitalScan(bodyPath, minResolution, requiredCoverage)
	local id = makeScanID(self.scanId, bodyPath, false)
	self.scanId = self.scanId + 1

	local scanData = ScanData.New(id, bodyPath, minResolution, requiredCoverage, true)

	self.scanMap[id] = scanData
	table.insert(self.pendingScans, scanData)

	return id
end

-- Set the given scan ID as the current scan
---@param id ScanID
function ScanManager:SetActiveScan(id)
	local scan = self.scanMap[id]

	if not scan or scan.complete or self.activeScan == scan then
		return
	end

	if not self:CanScanBeActivated(id) then
		return
	end

	utils.remove_elem(self.pendingScans, scan)
	table.insert(self.pendingScans, self.activeScan)

	self.activeScan = scan
	self.withinParameters = false

	self:StartScanCallback()
end

-- Clear the currently active scan
function ScanManager:ClearActiveScan()
	table.insert(self.pendingScans, self.activeScan)
	self.activeScan = nil
end

-- Check if the given scan is within viable parameters (e.g. in correct body frame, etc)
---@param id ScanID id of the scan to activate
---@return boolean
function ScanManager:CanScanBeActivated(id)
	if not self.hasSensor then return false end

	local scan = self.scanMap[id]
	if not scan then return false end

	local frameBody = self.ship.frameBody
	if not frameBody then return false end

	return frameBody.path == scan.bodyPath and (scan.orbital or self.ship.frameRotating)
end

-- Remove a completed scan from the scanner's data storage
---@param id unknown an ID returned from AddNewScan
---@return ScanData? scan the completed scan data
---@nodiscard
function ScanManager:AcceptScanComplete(id)
	local scan = self.scanMap[id]

	if not scan or not scan.complete then
		return nil
	end

	self.scanMap[id] = nil
	utils.remove_elem(self.completedScans, scan)

	return scan
end

-- Cancel a pending or active scan completely (e.g. mission failure)
---@param id unknown an ID returned from AddNewScan
---@return ScanData? scan the completed scan data
---@nodiscard
function ScanManager:CancelScan(id)
	local scan = self.scanMap[id]

	if not scan or scan.complete then
		return nil
	end

	self.scanMap[id] = nil

	if scan == self.activeScan then
		self.activeScan = nil
	else
		utils.remove_elem(self.pendingScans, scan)
	end

	return scan
end

--=============================================================================

-- Triggered when the ship has entered a new frame, updates active scan
---@param body Body
---@package
function ScanManager:OnEnteredFrame(body)
	if self.activeScan then

		local frameBody = body.frameBody

		if not frameBody or frameBody.path ~= self.activeScan.bodyPath then
			self:ClearActiveScan()
		end

	end
end

-- Start the scanner callback
---@package
function ScanManager:StartScanCallback()
	local scan = self.activeScan
	local updateRate = scan.orbital and ORBITAL_SCAN_UPDATE_RATE or SURFACE_SCAN_UPDATE_RATE

	-- TODO: generating a new callback every time we start a scan isn't the most performant,
	-- but it's the best way to handle it without the ability to cancel or modify an ongoing timer
	Timer:CallEvery(updateRate, function()

		if self.activeScan ~= scan then return true end

		return self:OnUpdateScan(scan)

	end)

	-- Immediately trigger a scan update for responsiveness
	self:OnUpdateScan(scan)
end

-- Update the current active scan, assuming we're in the correct frame
---@param scan ScanData
---@return boolean cancel
---@package
function ScanManager:OnUpdateScan(scan)
	local body = self.ship.frameBody
	if not body then return true end

	local radius = body:GetSystemBody().radius
	local altitude, resolution = self:GetBodyState(body)
	local currentScanPos = self.ship:GetPositionRelTo(body):normalized()

	-- Determine if we're currently in range to record scan data
	local withinParams = resolution <= scan.minResolution

	-- print("altitude", altitude)
	-- print("resolution", resolution)
	-- print("within params", withinParams)

	-- Use great-arc distance to calculate the amount of scan coverage in square meters
	-- distance = Δσ * radius = arctan( |n1 ⨯ n2| / n1 · n2 ) * radius
	-- See https://en.wikipedia.org/wiki/Great-circle_distance#Vector_version
	if withinParams and self.withinParameters then

		local crossTerm = currentScanPos:cross(self.lastScanPos):length()
		local dotTerm = currentScanPos:dot(self.lastScanPos)
		local dS = math.atan(crossTerm / dotTerm)

		local coverage
		local beamWidth = self.apertureWidth * altitude

		if scan.orbital then
			-- percent of total coverage gained per orbit, calculated at the widest point of the body
			local covPctPerOrbit = beamWidth / (radius * math.pi)
			local orbitPercent = dS / (math.pi * 2)

			coverage = covPctPerOrbit * orbitPercent
		else
			local distance = dS * radius
			-- total coverage gain in square kilometers
			coverage = beamWidth * distance / SQUARE_KILOMETERS
		end

		-- print("beam width", beamWidth)
		-- print("coverage", coverage)
		-- print("target", scan.targetCoverage)

		scan.coverage = scan.coverage + coverage

	end

	self.lastScanPos = currentScanPos
	self.withinParameters = withinParams

	if scan.coverage > scan.targetCoverage then
		scan.complete = true
		self.activeScan = nil

		table.insert(self.completedScans, scan)
		return true
	end


	return false
end

--=============================================================================

Serializer:RegisterClass('Scout.ScanData', ScanData)
Serializer:RegisterClass('Scout.ScanManager', ScanManager)

---@param body Body
Event.Register("onFrameChanged", function(body)
	local scanMgr = body:GetComponent("ScanManager")

	if scanMgr then
		scanMgr:OnEnteredFrame(body)
	end
end)

return ScanManager
