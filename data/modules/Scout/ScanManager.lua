-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Event = require 'Event'
local Timer = require 'Timer'
local Serializer = require 'Serializer'

local utils = require 'utils'

--
-- Class: ScanManager
--
-- This file implements a generic ship component to manage and carry out
-- orbital or surface scan missions, provide a relatively realistic abstraction
-- for gathering scan data, and serve as the "backend" for a mission module
-- that generates scans and provides a reward to the player.
--
-- It does not currently have support for arbitrary player-driven scans, nor
-- is it designed to be integrated with some Point-of-Interest system to allow
-- for any kind of discovery-related gameplay. These are features that could be
-- very useful in the future, but there exists no gameplay reason to support
-- them at the time of writing
--
-- The ScanManager assumes that all sensors on the player's ship are 1D beam
-- sensors with a specified aperture and resolution. It is assumed to be able
-- to scan in all directions regardless of the player's orientation relative to
-- the surface.
--
-- Differing scanner capabilities can be provided by ship equipment. It is
-- assumed that an equipment item provides either surface scanning capability
-- or orbital scanning capability, but never both.
--
-- No attempt is made to de-duplicate the orbital coverage of a body - the
-- player can keep scanning in the exact same orbit and will gain scan data
-- at the same rate.
--
-- There is some room for future improvement of this module, as recorded in
-- these TODOs:
--
-- TODO: use either orbital inclination or player velocity relative to the
-- surface rotation of the body to modulate orbital scan data gain rate.
-- This should not be a 100% realistic factor, but provide a small bonus for
-- maneuvering to a more "interesting" orbit (e.g. 30%).
--
-- TODO: Allow defining surface or orbital scan missions to require data be
-- gathered within a certain distance of a surface location (e.g. lat/lon or a
-- SurfaceBookmark abstraction).
-- Missions of this type should have significantly lower coverage requirements.
--
-- TODO: the ScanManager should provide some sort of visual or audio feedback
-- to the player that a scan is ongoing. E.g. a "scan beam" visual element,
-- a visible sensor object on the outside of the ship, etc.
--
-- TODO: the ScanManager should be extensible to provide some kind of planetary
-- discovery scanning capaibility - hidden outposts, mineral concentrations,
-- or other procedurally-generated geological features should be able to create
-- some kind of "sensor return" that is integrated into the ScanManager.
-- This is a very low-priority feature and may require the ScanManager be
-- integrated into a larger "sensors suite" abstraction instead.

--=============================================================================

---@alias ScanID string

---@class ScanData
---@field meta table
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
		orbital = isOrbital or false,
		coverage = 0.0,
		complete = false
	}, ScanData.meta)
end

---@class ScanManager.SensorData
---@field resolutionMs number
---@field minAltitude number
---@field minResolution number
---@field apertureWidth number
---@field orbital boolean
---@field equip unknown

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
	MinAltitude = "MIN_ALTITUDE",
	OutOfRange = "OUT_OF_RANGE",
	NoSensors = "NO_SENSORS",
}

---@param ship Ship
function ScanManager:Constructor(ship)
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

	-- The list of available sensors on this craft
	---@type ScanManager.SensorData[]
	self.sensors = {}
	-- The currently activated sensor that will be used for this scan
	---@type ScanManager.SensorData?
	self.activeSensor = nil

	-- is the ship's current position within viable scan parameters?
	self.withinParameters = false

	-- calculate the distance the ship has moved since the last scan step
	---@type Vector3
	self.lastScanPos = nil

	-- is there a currently running scan callback
	self.activeCallback = false

	self:SetupShipEquipListener()
end

---@package
-- Register an equipment listener on the player's ship
function ScanManager:SetupShipEquipListener()
	self.ship.equipSet:AddListener(function(slot)
		if slot == "sensor" then
			self:UpdateSensorEquipInfo()
		end
	end)

	self:UpdateSensorEquipInfo()
end

---@private
-- Scan the ship's equipment and determine its sensor capabilities
-- Note: this function completely rebuilds the list of sensors when a sensor equipment item is changed on the ship
function ScanManager:UpdateSensorEquipInfo()
	local equip = self.ship.equipSet

	self.sensors = {}

	local sensors = equip:Get("sensor")
	if #sensors == 0 then
		self.activeSensor = nil
		self:ClearActiveScan()
		return
	end

	-- rebuild the list of sensors from the ship's equipment
	-- we don't attempt to preserve the existing list because we have no idea
	-- which sensor was added/removed and where
	for i, sensor in ipairs(sensors) do
		local sensorData = {}

		-- width of the aperture at one meter
		sensorData.apertureWidth = math.tan(math.deg2rad(sensor.stats.aperture) * 0.5) * 2
		-- resolution in meters / sample at one meter
		sensorData.resolutionMs = sensorData.apertureWidth / sensor.stats.resolution
		-- minimum altitude required to get sensor data
		sensorData.minAltitude = sensor.stats.minAltitude
		-- minimum effective resolution of sensor data
		sensorData.minResolution = sensorData.resolutionMs * sensorData.minAltitude
		-- is the sensor a surface or orbital scanner?
		sensorData.orbital = sensor.stats.orbital or false

		sensorData.equip = sensor

		table.insert(self.sensors, sensorData)

		if self.activeSensor and self.activeSensor.equip == sensor then
			self.activeSensor = sensorData
		end
	end

	-- if the old active sensor is no longer present on the craft, default to
	-- the first sensor in the list if any
	if not (self.activeSensor and utils.contains(sensors, self.activeSensor.equip)) then
		self.activeSensor = self.sensors[1]
		self:ClearActiveScan()
	end

end

---@param scan ScanData?
---@return Body?
function ScanManager:GetScanBodyByType(scan)
	scan = scan or self.activeScan
	assert(scan, "ScanManager:GetScanBodyByType: Error: scan was nil")
	assert(self.scanMap[scan.id], "ScanManager:GetScanBodyByType: Error: scan doesn't belong to current ScanManager")
	return scan.orbital and scan.bodyPath:GetSystemBody().body or self.ship.frameBody
end

---@param body Body
---@param scan ScanData?
---@return number altitude
---@return number resolution
function ScanManager:GetBodyState(body, scan)
	scan = scan or self.activeScan
	assert(scan, "ScanManager:GetBodyState: Error: scan was nil")
	assert(self.scanMap[scan.id], "ScanManager:GetBodyState: Error: scan doesn't belong to current ScanManager")
	assert(self.activeSensor, "ScanManager:GetBodyState(): have an active scan without an onboard sensor")

	local altitude
	local radius = body:GetSystemBody().radius

	if scan.orbital then
		-- altitude above sea-level
		altitude = self.ship:GetPositionRelTo(body):length() - radius
	else
		-- altitude above terrain
		altitude = self.ship:GetAltitudeRelTo(body)
	end

	-- calculate effective resolution in meters/sample at this altitude
	local resolution = self.activeSensor.resolutionMs * altitude
	return altitude, resolution
end

--=============================================================================

function ScanManager:GetActiveSensor()
	return self.activeSensor
end

function ScanManager:GetAvailableSensors()
	return self.sensors
end

---@return ScanManager.State
function ScanManager:GetState()
	if not self.activeSensor then
		return self.State.NoSensors
	end

	if self.activeScan then
		local body = self:GetScanBodyByType()
		assert(body)

		local altitude, resolution = self:GetBodyState(body)
		local isInRange = resolution <= self.activeScan.minResolution

		if altitude > self.activeSensor.minAltitude then
			return isInRange and self.State.Scanning or self.State.OutOfRange
		else
			return self.State.MinAltitude
		end
	else
		return self.State.Inactive
	end
end

-- Determine if the given scan can be carried out by the selected sensor,
-- and the maximum allowable mission parameters for the scan
---@param sBody SystemBody body to be scanning
---@param maxResolution number maximum scan resolution allowed, effectively meters/sample
function ScanManager:GetScanParameters(sBody, maxResolution, orbital, sensorIndex)
	local sensor = sensorIndex and self.sensors[sensorIndex] or self.activeSensor
	if not sensor then return nil end

	-- sensor cannot carry out this mission
	if not sensor.orbital == orbital then return nil end

	-- maximum altitude for the given resolution
	local maxAlt = maxResolution / sensor.resolutionMs
	-- one-half circumference due to orbit covering the entire sphere
	local bodyHalfCirc = sBody.radius * math.pi

	-- Amount of body coverage per orbit
	-- Math is a little bit cheat-y here, would be better to use great circle distance rather than straight-line
	local coverage = sensor.apertureWidth * maxAlt
	-- number of orbits required to achieve full coverage of the body
	local orbitToCov = bodyHalfCirc / coverage

	return {
		-- whether the given scan can actually be carried out
		canScan = maxAlt > sensor.minAltitude and sensor.minResolution <= maxResolution,
		-- the maximum altitude the ship can fly at to scan the body
		maxAltitude = maxAlt,
		-- the minimum altitude the ship can fly at to scan the body
		minAltitude = sensor.minAltitude,
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

-- Set the given sensor as the currently active data-source
function ScanManager:SetActiveSensor(index)
	local sensor = self.sensors[index]
	if not sensor then return false end

	self.activeSensor = sensor

	if self.activeScan and self.activeScan.orbital ~= self.activeSensor.orbital then
		self:ClearActiveScan()
	end
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

	if self.activeScan then
		Event.Queue("onScanPaused", self.ship, scan.id)
	end

	self.activeScan = scan
	self.withinParameters = false

	self:StartScanCallback()
end

-- Clear the currently active scan and running callback
function ScanManager:ClearActiveScan()
	if self.activeScan then
		table.insert(self.pendingScans, self.activeScan)
		Event.Queue("onScanPaused", self.ship, self.activeScan.id)
	end

	self.activeScan = nil
	self.activeCallback = nil
	self.withinParameters = false
end

-- Check if the given scan is within viable parameters (e.g. in correct body frame, etc)
---@param id ScanID id of the scan to activate
---@return boolean
function ScanManager:CanScanBeActivated(id)
	if not self.activeSensor then return false end

	local scan = self.scanMap[id]
	if not scan then return false end

	if scan.complete then return false end

	local body = self:GetScanBodyByType(scan)
	if not body then return false end

	if body.path ~= scan.bodyPath then
		return false
	end

	if scan.orbital then
		local altitude, resolution = self:GetBodyState(body, scan)
		if(resolution > (scan.minResolution * 1.10)) then
			return false
		end
	end

	if scan.orbital ~= self.activeSensor.orbital then
		return false
	end

	if scan.minResolution < self.activeSensor.minResolution then
		return false
	end

	return (scan.orbital or self.ship.frameRotating)
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
		self.activeCallback = nil
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
		if self.activeScan.orbital then
			return
		end
		local frameBody = body.frameBody

		if not frameBody or frameBody.path ~= self.activeScan.bodyPath then
			self:ClearActiveScan()
		end

	end
end

-- Start the scanner callback
---@param force boolean?
---@package
function ScanManager:StartScanCallback(force)
	local updateRate = self.activeScan.orbital and ORBITAL_SCAN_UPDATE_RATE or SURFACE_SCAN_UPDATE_RATE

	-- Immediately trigger a scan update for responsiveness
	self:OnUpdateScan(self.activeScan)

	-- Don't queue a new scan callback if we're already running one at the right frequency
	-- Force this check if this function was called from ScanManager:Unserialize() because
	-- callback is not yet setted up
	if updateRate == self.activeCallback and not force then
		return
	end

	self.activeCallback = updateRate

	Timer:CallEvery(updateRate, function()
		-- cancel this callback if the parameters have changed (it's been orphaned)
		if not self.activeScan or not self.activeSensor or self.activeCallback ~= updateRate then
			return true
		end

		return self:OnUpdateScan(self.activeScan)

	end)
end

-- Update the current active scan, assuming we're in the correct frame
---@param scan ScanData
---@return boolean cancel
---@package
function ScanManager:OnUpdateScan(scan)
	local body = self:GetScanBodyByType(scan)

	-- Somehow we don't have a valid body to be scanning
	if not body then
		logWarning("ScanManager: owning ship does not have a system body to scan!")
		self:ClearActiveScan()
		return true
	end

	local radius = body:GetSystemBody().radius
	local altitude, resolution = self:GetBodyState(body)
	local currentScanPos = self.ship:GetPositionRelTo(body):normalized()

	if scan.orbital then
		if(resolution > (scan.minResolution * 1.10)) then
			self:ClearActiveScan()
			return true
		end
	end

	-- Determine if we're currently in range to record scan data
	local withinParams = resolution <= scan.minResolution and altitude > self.activeSensor.minAltitude



	-- Use great-arc distance to calculate the amount of scan coverage in square meters
	-- distance = Δσ * radius = arctan( |n1 ⨯ n2| / n1 · n2 ) * radius
	-- See https://en.wikipedia.org/wiki/Great-circle_distance#Vector_version
	if withinParams and self.withinParameters then

		local crossTerm = currentScanPos:cross(self.lastScanPos):length()
		local dotTerm = currentScanPos:dot(self.lastScanPos)
		local dS = math.atan(crossTerm / dotTerm)

		local coverage
		local beamWidth = self.activeSensor.apertureWidth * altitude

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

		scan.coverage = scan.coverage + coverage

	elseif withinParams then
		Event.Queue("onScanRangeEnter", self.ship, scan.id)
	elseif self.withinParameters then
		Event.Queue("onScanRangeExit", self.ship, scan.id)
	end

	self.lastScanPos = currentScanPos
	self.withinParameters = withinParams

	if scan.coverage > scan.targetCoverage then
		scan.complete = true
		self.activeScan = nil
		self.activeCallback = nil

		table.insert(self.completedScans, scan)
		Event.Queue("onScanComplete", self.ship, scan.id)

		return true
	end


	return false
end

--=============================================================================

function ScanManager:Unserialize()
	setmetatable(self, ScanManager.meta)

	-- FIXME: the order of component deserialization is undefined
	-- The Unserialize() method should only mutate this component's state;
	-- these calls should be deferred into a PostLoad() method or event
	self:SetupShipEquipListener()

	-- Restore the scanning callback on loading a saved game
	if self.activeCallback then
		self:StartScanCallback(true)
	end

	return self
end

Serializer:RegisterClass('Scout.ScanData', ScanData)
Serializer:RegisterClass('Scout.ScanManager', ScanManager)

---@param body Body
Event.Register("onFrameChanged", function(body)
	local scanMgr = body:GetComponent("ScanManager")

	if scanMgr then
		scanMgr:OnEnteredFrame(body)
	end
end)

---@param ship Ship
Event.Register("onShipTypeChanged", function(ship)
	local scanMgr = ship:GetComponent("ScanManager")

	if scanMgr then
		scanMgr:SetupShipEquipListener()
	end
end)

return ScanManager
