--
-- Class: Ship
--
-- Class representing a ship. Inherits from <Body>.
--

--
-- Group: Methods
--

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
function Ship:Refuel(amount)
	local t = Translate:GetTranslator()
    local currentFuel = self.fuel
    if currentFuel == 100 then
		Comms.Message(t('Fuel tank full.'))
        return 0
    end
    local ship_stats = self:GetStats()
    local needed = math.clamp(math.ceil(ship_stats.maxFuelTankMass - ship_stats.fuelMassLeft),0, amount)
    local removed = self:RemoveEquip('WATER', needed)
    self:SetFuelPercent(math.clamp(self.fuel + removed * 100 / ship_stats.maxFuelTankMass, 0, 100))
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
function Ship:Jettison(equip)
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
		Event.Queue("onCargoUnload", self, equip)
	else -- LANDED
		Event.Queue("onCargoUnload", self, equip)
	end
end

-- Style note: These function definitions use syntactic sugar not normally used
-- in Pioneer. Might want to think about changing them. XXX

--
-- Method: GetMinimumCrew
--
-- Return the minimum number of crew required for takeoff or launch
--
-- > number = ship:GetMinimumCrew()
--
-- Returns:
--
--   number - Minimum number of crew required for launch
--
-- Availability:
--
--   alpha 30
--
-- Status:
--
--   experimental
--
function Ship:GetMinimumCrew()
	return 1 -- Placeholder value
end

--
-- Method: FullCrew
--
-- Return the maximum number of crew that this ship can accommodate
--
-- > number = ship:GetFullCrew()
--
-- Returns:
--
--   number - Maximum number of crew this ship can accommodate
--
-- Availability:
--
--   alpha 30
--
-- Status:
--
--   experimental
--
function Ship:GetFullCrew()
	return 1 -- Placeholder value
end

--
-- Method: HasMinimumCrew
--
-- Determine whether a ship has the minimum crew required for launch
--
-- > canLaunch = ship:HasMinimumCrew()
--
-- Returns:
--
--   canLaunch - Boolean, true if ship has minimum required crew for launch, otherwise false/nil
--
-- Availability:
--
--   alpha 30
--
-- Status:
--
--   experimental
--
function ship:HasMinimumCrew()
	return true -- placeholder value
end
