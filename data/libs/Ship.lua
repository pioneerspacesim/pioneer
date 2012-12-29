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

--------------------------------------
-- START OF TEMPORARY BODGE-UP CODE --
--------------------------------------
-- THIS IS TEMPORARY STUFF! This all needs to be moved into the ships' Lua files
-- and defined properly, with the following methods re-written as core API functions.
local ship_crew_min = {
	adder = 1,
	asp_explorer = 1,
	boa_freighter = 5,
	caribou = 6,
	cobra3 = 1,
	cobra = 1,
	constrictor = 3,
	eagle_lrf = 1,
	eagle_mk2 = 1,
	eagle_mk3 = 1,
	eagle_mk4 = 1,
	eye = 1,
	flowerfairy = 4,
	hammerhead = 8,
	imperial_courier = 1,
	imperial_trader = 3,
	ip_shuttle = 1,
	ladybird_starfighter = 1,
	lanner = 1,
	long_range_cruiser = 6,
	lynx_bulk_carrier = 4,
	meteor = 1,
	natrix = 1,
	sidewinder = 1,
	sirius_interdictor = 1,
	stardust = 3,
	talon_military_interceptor = 1,
	turtle = 1,
	viper_defence_craft = 1,
	viper_police_craft = 1,
	viper_x = 1,
	wave = 1,
}
local ship_crew_full = {
	adder = 1,
	asp_explorer = 2,
	boa_freighter = 8,
	caribou = 12,
	cobra3 = 3,
	cobra = 3,
	constrictor = 5,
	eagle_lrf = 1,
	eagle_mk2 = 1,
	eagle_mk3 = 1,
	eagle_mk4 = 1,
	eye = 1,
	flowerfairy = 6,
	hammerhead = 14,
	imperial_courier = 3,
	imperial_trader = 6,
	ip_shuttle = 1,
	ladybird_starfighter = 1,
	lanner = 3,
	long_range_cruiser = 20,
	lynx_bulk_carrier = 10,
	meteor = 1,
	natrix = 1,
	sidewinder = 1,
	sirius_interdictor = 2,
	stardust = 4,
	talon_military_interceptor = 1,
	turtle = 1,
	viper_defence_craft = 2,
	viper_police_craft = 2,
	viper_x = 1,
	wave = 1,
}
------------------------------------
-- END OF TEMPORARY BODGE-UP CODE --
------------------------------------

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
	return ship_crew_min[self.shipId]
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
	return ship_crew_full[self.shipId]
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
