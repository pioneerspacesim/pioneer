-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ShipDef = import("ShipDef")

local ShipType
ShipType = {

--
-- Function: GetShipType
--
-- Get a description object for the given ship id
--
-- > shiptype = ShipType.GetShipType(id)
--
-- Parameters:
--
--   id - the id of the ship to get the description object for
--
-- Example:
--
-- > local shiptype = ShipType.GetShipType("eagle_lrf")
--
-- Availability:
--
--   alpha 10
--
-- Status:
--
--   deprecated (will be removed in alpha 33)
--
GetShipType = function (id)
	if ShipDef[id] then return ShipDef[id] end
	error(string.format("Invalid ship name '%s'", id))
end,

--
-- Function: GetShipTypes
--
-- Returns an array of ship description objects that match the specified
-- filter
--
-- > shiptypes = ShipType.GetShipTypes(tag, filter)
--
-- Parameters:
--
--   tag - a <Constants.ShipTypeTag> to select the ship group to search. Using
--         "NONE" will check all ships.
--
--   filter - an optional function. If specified the function will be called
--            once for each ship type with the description object as the only
--            parameter. If the filter function returns true then the
--            ship name will be included in the array returned by
--            <GetShipTypes>, otherwise it will be omitted. If no filter
--            function is specified then all ships are returned.
--
-- Returns:
--
--   shiptypes - an array containing zero or more ship names for which the
--               relevant description object matched the filter
--
-- Example:
--
-- > local shiptypes = ShipType.GetShipTypes('SHIP', function (t)
-- >     local mass = t.hullMass
-- >     return mass >= 50 and mass <= 150
-- > end)
--
-- Availability:
--
--   alpha 10
--
-- Status:
--
--   deprecated (will be removed in alpha 33)
--
GetShipTypes = function (tag, filter)
	local ids = {}
	for id,def in pairs(ShipDef) do
		if (tag == "NONE" or def.tag == tag) and (not filter or filter(def)) then table.insert(ids,id) end
	end
	return ids
end,

}

return ShipType
