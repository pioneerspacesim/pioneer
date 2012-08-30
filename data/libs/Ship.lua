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
--   type - a <Constants.EquipType> string for the missile type. specifying an
--          equipment that is not a missile will result in a Lua error
--
--   target - the <Ship> to fire the missile at
--
-- Return:
--
--   fired - true if the missile was fired, false if the ship has no missile
--           of the requested type
--
-- Availability:
--
--   alpha 10
--
-- Status:
--
--   experimental
--
function Ship:FireMissileAt(missile_type, target)
	for i,m in ipairs(self:GetEquip("MISSILE")) do
		if m == missile_type then
			if self:SpawnMissile(missile_type, target) then
				self:SetEquip("MISSILE", i, "NONE")
				return true
			end
			return false
		end
	end
	return false
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

