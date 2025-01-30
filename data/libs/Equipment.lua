-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Serializer = require 'Serializer'

---@class Equipment
local Equipment = {}

---@type table<string, EquipType>
Equipment.new = {}

function Equipment.Get(id)
	return Equipment.new[id]
end

function Equipment.Register(id, type)
	Equipment.new[id] = type
	type.id = id

	Serializer:RegisterPersistent("Equipment." .. id, type)
end

return Equipment
