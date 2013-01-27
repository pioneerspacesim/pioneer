-- Temporary mapping while waiting for new-equipment to embed this information.
local missile_names = {
	MISSILE_UNGUIDED="missile_unguided",
	MISSILE_GUIDED="missile_guided",
	MISSILE_SMART="missile_smart",
	MISSILE_NAVAL="missile_naval"
}
--
-- Class: Ship
--
-- Class representing a ship. Inherits from <Body>.
--

--
-- Group: Methods
--

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
			if m == missile then
				missile_object = self:SpawnMissile(missile_names[missile])
				if missile_object ~= nil then
					self:SetEquip("MISSILE", i, "NONE")
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

