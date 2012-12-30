--
-- Class: Ship
--
-- Class representing a ship. Inherits from <Body>.
--

-- This is a protected table (accessors only) in which details of each ship's crew
-- will be stored.
local CrewRoster = {}

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
    local ship_stats = self:Stats()
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
--   alpha 30
--
-- Status:
--
--   experimental
--
local function isNotAlreadyEnrolled(crewmember)
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

function Ship:Enroll(newCrewMember)
	if not (
		type(newCrewMember) == "table" and
		getmetatable(newCrewMember) and
		getmetatable(newCrewMember).class == 'Character'
	) then
		error("Ship:Enroll: newCrewMember must be a Character object")
	end
	if not CrewRoster[self] then CrewRoster[self] = {} end
	if #CrewRoster[self] < ShipType.GetShipType(self.shipId).maxCrew
	and isNotAlreadyEnrolled(newCrewMember)
	then
		table.insert(CrewRoster[self],newCrewMember)
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
--   alpha 30
--
-- Status:
--
--   experimental
--

function Ship:Dismiss(crewMember)
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
			return true
		end
	end
	return false
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
function Ship:HasMinimumCrew()
	return CrewRoster[self] and #CrewRoster[self] >= ShipType.GetShipType(self.shipId).minCrew
end
