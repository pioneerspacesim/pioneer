-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Ship = import_core("Ship")
local Game = import_core("Game")
local Engine = import("Engine")
local Event = import("Event")
local Serializer = import("Serializer")
local ShipDef = import("ShipDef")
local Timer = import("Timer")
local Lang = import("Lang")

local l = Lang.GetResource("ui-core")

-- Temporary mapping while waiting for new-equipment to embed this information.
local missile_names = {
	MISSILE_UNGUIDED="missile_unguided",
	MISSILE_GUIDED="missile_guided",
	MISSILE_SMART="missile_smart",
	MISSILE_NAVAL="missile_naval"
}

local compat = {}

--
-- Class: Ship
--
-- Class representing a ship. Inherits from <Body>.
--

-- class method
function Ship.MakeRandomLabel ()
	local letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	local a = Engine.rand:Integer(1, #letters)
	local b = Engine.rand:Integer(1, #letters)
	return string.format("%s%s-%04d", letters:sub(a,a), letters:sub(b,b), Engine.rand:Integer(10000))
end

-- This is a protected table (accessors only) in which details of each ship's crew
-- will be stored.
local CrewRoster = {}

--
-- Group: Methods
--

-- Method: GetEquipSlotCapacity
--
-- Get the maximum number of a particular type of equipment this ship can
-- hold. This is the number of items that can be held, not the mass.
-- <AddEquip> will take care of ensuring the hull capacity is not exceeded.
--
-- > capacity = shiptype:GetEquipSlotCapacity(slot)
--
-- Parameters:
--
--   slot - a <Constants.EquipSlot> string for the wanted equipment type
--
-- Returns:
--
--   capacity - the maximum capacity of the equipment slot
--
-- Availability:
--
--  alpha 10
--
-- Status:
--
--  experimental
--
function Ship:GetEquipSlotCapacity(slot)
	local c = compat.slots.old2new[slot]
	if c then
		debug.deprecated("Ship:GetEquipSlotCapacity")
		return self.equipSet:SlotSize(c)
	end
	return self.equipSet:SlotSize(slot)
end

-- Method: GetEquipCountOccupied
--
-- Return the number of item in a given slot
--
-- > if ship:GetEquipCountOccupied("engine") > 1 then HyperdriveOverLoadAndExplode(ship) end
--
-- Availability:
--
--  TBA
--
-- Status:
--
--  experimental
--
function Ship:GetEquipCountOccupied(slot)
	return self.equipSet:OccupiedSpace(slot)
end

-- Method: CountEquip
--
-- Get the number of a given equipment or cargo item in a given equipment slot
--
-- > count = ship:CountEquip(item, slot)
--
-- Parameters:
--
--   slot - a <Constants.EquipSlot> string for the slot
--
--   item - a <Constants.EquipType> string for the item
--
-- Return:
--
--   count - the number of the given item in the slot
--
-- Availability:
--
--  TBA
--
-- Status:
--
--  experimental
--
function Ship:GetEquipCount(slot, item)
	debug.deprecated("Ship:GetEquipCount")
	return self:CountEquip(item, slot)
end

function Ship:CountEquip(item, slot)
	if slot then
		local c = compat.slots.old2new[slot]
		if c then
			debug.deprecated("Ship:CountEquip")
			slot = c
		end
	end
	if type(item) == "string" then
		debug.deprecated("Ship:GetEquipCount")
		item = compat.equip.old2new[item]
	end
	return self.equipSet:Count(item, slot)
end

--
-- Method: AddEquip
--
-- Add an equipment or cargo item to its appropriate equipment slot
--
-- > num_added = ship:AddEquip(item, count)
--
-- Parameters:
--
--   item - a <Constants.EquipType> string for the item
--
--   count - optional. The number of this item to add. Defaults to 1.
--
-- Return:
--
--   num_added - the number of items added. Can be less than count if there
--               was not enough room.
--
-- Example:
--
-- > ship:AddEquip("ANIMAL_MEAT", 10)
--
-- Availability:
--
--   alpha 10
--
-- Status:
--
--   experimental
--
function Ship:AddEquip(item, count)
	if type(item) == "string" then
		debug.deprecated("Ship:AddEquip")
		item = compat.equip.old2new[item]
	end
	local ret = self.equipSet:Add(self, item, count)
	if ret > 0 then
		Event.Queue("onShipEquipmentChange", self, item)
	end
	return ret
end

--
-- Method: GetEquip
--
-- Get a list of equipment in a given equipment slot
--
-- > equip = ship:GetEquip(slot, index)
-- > equiplist = ship:GetEquip(slot)
--
-- Parameters:
--
--   slot - a <Constants.EquipSlot> string for the wanted equipment type
--
--   index - optional. The equipment position in the slot to fetch. If
--           specified the item at that position in the slot will be returned,
--           otherwise a table containing all items in the slot will be
--           returned instead.
--
-- Return:
--
--   equip - when index is specified, a <Constants.EquipType> string for the
--           item
--
--   equiplist - when index is not specified, a table of zero or more
--               <Constants.EquipType> strings for all the items in the slot
--
-- Availability:
--
--  alpha 10
--
-- Status:
--
--  experimental
--
Ship.GetEquip = function (self, slot, index)
	local c = compat.slots.old2new[slot]
	if c then
		debug.deprecated("Ship:GetEquip")
		slot = c
	end
	local ret = self.equipSet:Get(slot, index)
	if c then
		if type(index) == "number" then
			if ret then
				ret = compat.equip.new2old[ret]
			else
				ret = "NONE"
			end
		else
			local tmp = {}
			for i=1,self.equipSet:SlotSize(slot),1 do
				if ret[i] then
					tmp[i] = compat.equip.new2old[ret[i]]
				else
					tmp[i] = "NONE"
				end
			end
			ret = tmp
		end
	end
	return ret
end

--
-- Method: GetEquipFree
--
-- Get the amount of free space in a given equipment slot
--
-- > free = ship:GetEquipFree(slot)
--
-- Parameters:
--
--   slot - a <Constants.EquipSlot> string for the slot to check
--
-- Return:
--
--   free - the number of item spaces left in this slot
--
-- Availability:
--
--  alpha 10
--
-- Status:
--
--  experimental
--
Ship.GetEquipFree = function (self, slot)
	local c = compat.slots.old2new[slot]
	if c then
		debug.deprecated("Ship:GetEquipFree")
		return self.equipSet:FreeSpace(c)
	end
	return self.equipSet:FreeSpace(slot)
end

compat.slots = {}
compat.equip = {}
compat.slots.old2new={
	CARGO="cargo", ENGINE="engine", LASER="laser_front",
	MISSILE="missile", ECM="ecm", SCANNER="scanner", RADARMAPPER="radar_mapper",
	HYPERCLOUD="hypercloud", HULLAUTOREPAIR="hull_autorepair",
	ENERGYBOOSTER="energy_booster", ATMOSHIELD="atmo_shield", CABIN="cabin",
	SHIELD="shield", FUELSCOOP="fuel_scoop", CARGOSCOOP="cargo_scoop",
	LASERCOOLER="laser_cooler", CARGOLIFESUPPORT="cargo_life_support",
	AUTOPILOT="autopilot"
}
--
-- Method: SetEquip
--
-- Overwrite a single item of equipment in a given equipment slot
--
-- > ship:SetEquip(slot, index, equip)
--
-- Parameters:
--
--   slot - a <Constants.EquipSlot> string for the equipment slot
--
--   index - the position to store the item in
--
--   item - a <Constants.EquipType> string for the item
--
-- Example:
--
-- > -- add a laser to the rear laser mount
-- > ship:SetEquip("LASER", 1, "PULSECANNON_1MW")
--
-- Availability:
--
--  alpha 10
--
-- Status:
--
--  experimental
--
Ship.SetEquip = function (self, slot, index, equip)
	if type(item) == "string" then
		debug.deprecated("Ship:SetEquip")
		item = compat.equip.old2new[item]
	end
	self.equipSet:Set(self, slot, index, item)
	Event.Queue("onShipEquipmentChange", self)
end

--
-- Method: RemoveEquip
--
-- Remove one or more of a given equipment type from its appropriate cargo slot
--
-- > num_removed = ship:RemoveEquip(item, count)
--
-- Parameters:
--
--   item - a <Constants.EquipType> string for the item
--
--   count - optional. The number of this item to remove. Defaults to 1.
--
-- Return:
--
--   num_removed - the number of items removed
--
-- Example:
--
-- > ship:RemoveEquip("DRIVE_CLASS1")
--
-- Availability:
--
--  alpha 10
--
-- Status:
--
--  experimental
--

Ship.RemoveEquip = function (self, item, count)
	if type(item) == "string" then
		debug.deprecated("Ship:RemoveEquip")
		item = compat.equip.old2new[item]
	end
	local ret = self.equipSet:Remove(self, item, count)
	if ret > 0 then
		Event.Queue("onShipEquipmentChange", self, item)
	end
	return ret
end

Ship.HyperjumpTo = function (self, path)
	local engine = self:GetEquip("engine", 1)
	if not engine then
		return "NO_DRIVE"
	end
	return engine:HyperjumpTo(self, path)
end

Ship.CanHyperjumpTo = function(self, path)
	return self:GetHyperspaceDetails(path) == 'OK'
end

-- ship:GetHyperspaceDetails(destination)      --  get details of jump from current system to 'destination'
-- ship:GetHyperspaceDetails(source, destination) -- get details of jump from 'source' to 'destination'
Ship.GetHyperspaceDetails = function (self, source, destination)
	if destination == nil then
		if not Game.system then
			return "DRIVE_ACTIVE", 0, 0, 0
		end
		destination = source
		source = Game.system.path
	end

	local engine = self:GetEquip("engine", 1)
	if not engine then
		return "NO_DRIVE", 0, 0, 0
	elseif source:IsSameSystem(destination) then
		return "CURRENT_SYSTEM", 0, 0, 0
	end
	local distance, fuel, duration = engine:CheckJump(self, source, destination)
	local status = "OK"
	if not duration then
		duration = 0
		fuel = 0
		status = "OUT_OF_RANGE"
	elseif fuel > self:CountEquip(engine.fuel) then
		status = "INSUFFICIENT_FUEL"
	end
	return status, distance, fuel, duration
end

Ship.GetHyperspaceRange = function (self)
	local engine = self:GetEquip("engine", 1)
	if not engine then
		return 0, 0
	end
	return engine:GetRange(self)
end

compat.slots.new2old = {}
for k,v in pairs(compat.slots.old2new) do
	compat.slots.new2old[v] = k
end

local equipment = import("Equipment")
local cargo = equipment.cargo
local hyperspace = equipment.hyperspace
local laser = equipment.laser
local misc = equipment.misc

compat.equip.new2old = {
	[cargo.hydrogen]="HYDROGEN", [cargo.air_processors]="AIR_PROCESSORS", [cargo.animal_meat]="ANIMAL_MEAT",
	[cargo.battle_weapons]="BATTLE_WEAPONS", [cargo.carbon_ore]="CARBON_ORE", [cargo.computers]="COMPUTERS",
	[cargo.consumer_goods]="CONSUMER_GOODS", [cargo.farm_machinery]="FARM_MACHINERY", [cargo.fertilizer]="FERTILIZER",
	[cargo.fruit_and_veg]="FRUIT_AND_VEG", [cargo.grain]="GRAIN", [cargo.hand_weapons]="HAND_WEAPONS",
	[cargo.hydrogen]="HYDROGEN", [cargo.industrial_machinery]="INDUSTRIAL_MACHINERY", [cargo.liquid_oxygen]="LIQUID_OXYGEN",
	[cargo.liquor]="LIQUOR", [cargo.live_animals]="LIVE_ANIMALS", [cargo.medicines]="MEDICINES", [cargo.metal_alloys]="METAL_ALLOYS",
	[cargo.metal_ore]="METAL_ORE", [cargo.military_fuel]="MILITARY_FUEL", [cargo.mining_machinery]="MINING_MACHINERY",
	[cargo.narcotics]="NARCOTICS", [cargo.nerve_gas]="NERVE_GAS", [cargo.plastics]="PLASTICS",
	[cargo.precious_metals]="PRECIOUS_METALS", [cargo.radioactives]="RADIOACTIVES", [cargo.robots]="ROBOTS",
	[cargo.rubbish]="RUBBISH", [cargo.slaves]="SLAVES", [cargo.textiles]="TEXTILES", [cargo.water]="WATER",

	[misc.missile_unguided]="MISSILE_UNGUIDED", [misc.missile_guided]="MISSILE_GUIDED",
	[misc.missile_smart]="MISSILE_SMART", [misc.missile_naval]="MISSILE_NAVAL",
	[misc.atmospheric_shielding]="ATMOSPHERIC_SHIELDING", [misc.ecm_basic]="ECM_BASIC",
	[misc.ecm_advanced]="ECM_ADVANCED", [misc.scanner]="SCANNER", [misc.cabin]="CABIN",
	[misc.shield_generator]="SHIELD_GENERATOR", [misc.laser_cooling_booster]="LASER_COOLING_BOOSTER",
	[misc.cargo_life_support]="CARGO_LIFE_SUPPORT", [misc.autopilot]="AUTOPILOT",
	[misc.radar_mapper]="RADAR_MAPPER", [misc.fuel_scoop]="FUEL_SCOOP",
	[misc.cargo_scoop]="CARGO_SCOOP", [misc.hypercloud_analyzer]="HYPERCLOUD_ANALYZER",
	[misc.shield_energy_booster]="SHIELD_ENERGY_BOOSTER", [misc.hull_autorepair]="HULL_AUTOREPAIR",
	[hyperspace.hyperdrive_1]="DRIVE_CLASS1", [hyperspace.hyperdrive_2]="DRIVE_CLASS2", [hyperspace.hyperdrive_3]="DRIVE_CLASS3",
	[hyperspace.hyperdrive_4]="DRIVE_CLASS4", [hyperspace.hyperdrive_5]="DRIVE_CLASS5", [hyperspace.hyperdrive_6]="DRIVE_CLASS6",
	[hyperspace.hyperdrive_7]="DRIVE_CLASS7", [hyperspace.hyperdrive_8]="DRIVE_CLASS8", [hyperspace.hyperdrive_9]="DRIVE_CLASS9",
	[hyperspace.hyperdrive_mil1]="DRIVE_MIL1", [hyperspace.hyperdrive_mil2]="DRIVE_MIL2", [hyperspace.hyperdrive_mil3]="DRIVE_MIL3",
	[hyperspace.hyperdrive_mil4]="DRIVE_MIL4", [laser.pulsecannon_1mw]="PULSECANNON_1MW", [laser.pulsecannon_dual_1mw]="PULSECANNON_DUAL_1MW",
	[laser.pulsecannon_2mw]="PULSECANNON_2MW", [laser.pulsecannon_rapid_2mw]="PULSECANNON_RAPID_2MW",
	[laser.pulsecannon_4mw]="PULSECANNON_4MW", [laser.pulsecannon_10mw]="PULSECANNON_10MW",
	[laser.pulsecannon_20mw]="PULSECANNON_20MW", [laser.miningcannon_17mw]="MININGCANNON_17MW",
	[laser.small_plasma_accelerator]="SMALL_PLASMA_ACCEL", [laser.large_plasma_accelerator]="LARGE_PLASMA_ACCEL"
}
compat.equip.old2new = {}
for k,v in pairs(compat.equip.new2old) do
	compat.equip.old2new[v] = k
end

-- Method: FireMissileAt
--
-- Fire a missile at the given target
--
-- > fired = ship:FireMissileAt(type, target)
--
-- Parameters:
--
--   missile - a <Constants.EquipType> string for the missile type. specifying an
--          equipment that is not a missile will result in a Lua error.
--          Specifying "any" will launch the first available missile.
--          You can also provide a number matching the slot of the missile you wish
--          to launch.
--
--   target - the <Ship> to fire the missile at
--
-- Return:
--
--   fired - the fired missile
--
-- Availability:
--
--   alpha 10
--
-- Status:
--
--   experimental
--
function Ship:FireMissileAt(missile, target)
	local missile_object = false
	if type(missile) == "number" then
		missile_type = self:GetEquip("MISSILE", missile)
		if missile_type ~= "NONE" then
			missile_object = self:SpawnMissile(missile_names[missile_type])
			if missile_object ~= nil then
				self:SetEquip("MISSILE", missile, "NONE")
			end
		end
	else
		for i,m in ipairs(self:GetEquip("MISSILE")) do
			if m == missile or (missile == "any" and m ~= "NONE") then
				missile_object = self:SpawnMissile(missile_names[m])
				if missile_object ~= nil then
					self:SetEquip("MISSILE", i, "NONE")
					break
				end
			end
		end
	end

	if missile_object then
		if target then
			missile_object:AIKamikaze(target)
		end
		-- Let's keep a safe distance before activating this device, shall we ?
		Timer:CallEvery(2, function ()
			if not missile_object:exists() then -- Usually means it has already exploded
				return true
			end
			if missile_object:DistanceTo(self) < 300 then
				return false
			else
				missile_object:Arm()
				return true
			end
		end)
	end

	return missile_object
end

--
-- Method: Refuel
--
-- Use the content of the cargo to refuel
--
-- > used_units = ship:Refuel(1)
--
-- Parameters:
--
--   amount - the amount of fuel (in tons) to take from the cargo
--
-- Result:
--
--   used_units - how much fuel units have been used to fuel the tank.
--
-- Availability:
--
--   alpha 26
--
-- Status:
--
--   experimental
--
Ship.Refuel = function (self,amount)
	local currentFuel = self.fuel
	if currentFuel == 100 then
		Comms.Message(l.FUEL_TANK_FULL) -- XXX don't translate in libs
		return 0
	end
	local fuelTankMass = ShipDef[self.shipId].fuelTankMass
	local needed = math.clamp(math.ceil(fuelTankMass - self.fuelMassLeft),0, amount)
	local removed = self:RemoveEquip('HYDROGEN', needed)
	self:SetFuelPercent(math.clamp(self.fuel + removed * 100 / fuelTankMass, 0, 100))
	return removed
end


--
-- Method: Jettison
--
-- Jettison one unit of the given cargo type
--
-- > success = ship:Jettison(item)
--
-- On sucessful jettison, the <EventQueue.onJettison> event is triggered.
--
-- Parameters:
--
--   item - the item to jettison
--
-- Result:
--
--   success - true if the item was jettisoned, false if the ship has no items
--             of that type or the ship is not in open flight
--
-- Availability:
--
--   alpha 10
--
-- Status:
--
--   experimental
--
Ship.Jettison = function (self,equip)
	if self.flightState ~= "FLYING" and self.flightState ~= "DOCKED" and self.flightState ~= "LANDED" then
		return false
	end
	if self:RemoveEquip(equip, 1) < 1 then
		return false
	end
	if self.flightState == "FLYING" then
		self:SpawnCargo(equip)
		Event.Queue("onJettison", self, equip)
	elseif self.flightState == "DOCKED" then
		Event.Queue("onCargoUnload", self:GetDockedWith(), equip)
	elseif self.flightState == "LANDED" then
		Event.Queue("onCargoUnload", self.frameBody, equip)
	end
end

--
-- Method: Enroll
--
-- Enroll a [Character] as a member of the ship's crew
--
-- > success = ship:Enroll(newCrewMember)
--
-- Parameters:
--
--   newCrewMember - a [Character] instance
--
-- Returns:
--
--   success - True indicates that the Character became a member of the crew. False indicates
--             that the Character did not become a member of the crew, either because there
--             is no room for the Character on the crew roster, or because they are already
--             enrolled as crew on another ship.
--
-- Availability:
--
--   alpha 31
--
-- Status:
--
--   experimental
--
local isNotAlreadyEnrolled = function (crewmember)
	for ship,crew in pairs(CrewRoster) do
		for key,existingmember in pairs(crew) do
			if existingmember == crewmember
			then
				return false
			end
		end
	end
	return true
end

Ship.Enroll = function (self,newCrewMember)
	if not (
		type(newCrewMember) == "table" and
		getmetatable(newCrewMember) and
		getmetatable(newCrewMember).class == 'Character'
	) then
		error("Ship:Enroll: newCrewMember must be a Character object")
	end
	if not CrewRoster[self] then CrewRoster[self] = {} end
	if #CrewRoster[self] < ShipDef[self.shipId].maxCrew
	and isNotAlreadyEnrolled(newCrewMember)
	then
		newCrewMember:CheckOut() -- Don't want other scripts using our crew for missions etc
		table.insert(CrewRoster[self],newCrewMember)
		Event.Queue('onJoinCrew',self,newCrewMember) -- Signal any scripts that care!
		return true
	else
		return false
	end
end

--
-- Method: Dismiss
--
-- Dismiss a [Character] as a member of the ship's crew
--
-- > success = ship:Dismiss(crewMember)
--
-- Parameters:
--
--   crewMember - a [Character] instance
--
-- Returns:
--
--   success - True indicates that the Character is no longer a member of the crew. False
--             indicates that the Character was not removed, either because they were not
--             a member of the crew, or because they could not be removed because of a
--             special case. Currently the only special case is that the player's Character
--             cannot be dismissed from a crew.
--
-- Availability:
--
--   alpha 31
--
-- Status:
--
--   experimental
--

Ship.Dismiss = function (self,crewMember)
	if not CrewRoster[self] then return false end
	if not (
		type(crewMember) == "table" and
		getmetatable(crewMember) and
		getmetatable(crewMember).class == 'Character'
	) then
		error("Ship:Dismiss: crewMember must be a Character object")
	end
	if crewMember.player then return false end -- Can't dismiss the player
	for key,existingCrewMember in pairs(CrewRoster[self]) do
		if crewMember == existingCrewMember
		then
			table.remove(CrewRoster[self],key)
			Event.Queue('onLeaveCrew',self,crewMember) -- Signal any scripts that care!
			crewMember:Save() -- Crew member can pop up elsewhere
			return true
		end
	end
	return false
end

--
-- Method: GenerateCrew
--
-- Generates a full crew complement for a ship that has no initialised crew list.
-- Intended to be run automatically by [EachCrewMember] when querying arbitrary ships.
--
-- > ship:GenerateCrew()
--
-- Availability:
--
--   alpha 31
--
-- Status:
--
--   experimental
--
Ship.GenerateCrew = function (self)
	if CrewRoster[self] then return end -- Bottle out if there's ever been a crew
	for i = 1, ShipDef[self.shipId].maxCrew do
		local newCrew = Character.New()
		newCrew:RollNew(true)
		self:Enroll(newCrew)
	end
end

--
-- Method: CrewNumber
--
-- Returns the number of the current crew employed on the ship.
--
-- > ship:CrewNumber()
--
-- Availability:
--
--   20140404
--
-- Status:
--
--   experimental
--
Ship.CrewNumber = function (self)
	return #CrewRoster[self]
end

--
-- Method: EachCrewMember
--
-- Returns an iterator function which returns each crew member in turn
--
-- > for crew in ship:EachCrewMember() do print(crew.name) end
--
-- Returns:
--
--   crew - A [Character], once per crew member per call
--
-- Availability:
--
--   alpha 31
--
-- Status:
--
--   experimental
--
Ship.EachCrewMember = function (self)
	-- If there's no crew, magic one up.
	if not CrewRoster[self] then self:GenerateCrew() end
	-- Initialise and return enclosed iterator
	local i = 0
	return function ()
		i = i + 1
		return CrewRoster[self][i]
	end
end

--
-- Method: HasCorrectCrew
--
-- Determine whether a ship has the minimum crew required for launch
--
-- > canLaunch = ship:HasCorrectCrew()
--
-- Returns:
--
--   canLaunch - Boolean, true if ship has minimum required crew for launch, otherwise false/nil
--
-- Availability:
--
--   alpha 31
--
-- Status:
--
--   experimental
--
Ship.HasCorrectCrew = function (self)
	return (CrewRoster[self] and (
		#CrewRoster[self] >= ShipDef[self.shipId].minCrew and
		#CrewRoster[self] <= ShipDef[self.shipId].maxCrew
	))
end

-- LOADING AND SAVING

local loaded_data

local onGameStart = function ()
	if loaded_data then
		CrewRoster = loaded_data
		Event.Queue('crewAvailable') -- Signal any scripts that depend on initialised crew
	end
	loaded_data = nil
end

local serialize = function ()
	-- Remove non-existent ships first, or the serializer will choke
	for crewedShip,crew in pairs(CrewRoster) do
		if not crewedShip:exists() then
			CrewRoster[crewedShip] = nil
		end
	end
	return CrewRoster
end

local unserialize = function (data)
	loaded_data = data
end

-- Function to check whether ships exist after hyperspace, and if they do not,
-- to remove their crew from the roster.
local onEnterSystem = function (ship)
	if ship:IsPlayer() then
		for crewedShip,crew in pairs(CrewRoster) do
			if not crewedShip:exists() then
				CrewRoster[crewedShip] = nil
			end
		end
	end
end

local onLeaveSystem = function (ship)
	local engine = ship:GetEquip("engine", 1)
	if engine then
		engine:OnEnterHyperspace(ship)
	end
end

local onShipDestroyed = function (ship, attacker)
	-- When a ship is destroyed, mark is crew as dead
	-- and remove the ship's crew from CrewRoster
	if CrewRoster[ship] then
		for key,crewMember in pairs(CrewRoster[ship]) do
			crewMember.dead = true
		end
		CrewRoster[ship] = nil
	end
end

Event.Register("onEnterSystem", onEnterSystem)
Event.Register("onLeaveSystem", onLeaveSystem)
Event.Register("onShipDestroyed", onShipDestroyed)
Event.Register("onGameStart", onGameStart)
Serializer:Register("ShipClass", serialize, unserialize)

Ship.equipCompat = compat

return Ship
