-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Lang = require 'Lang'
local Serializer = require 'Serializer'
local utils = require 'utils'

-- Default l10n resource for commodity types
local CARGOLANGRESOURCE = "commodity"

--
-- Class: CommodityType
--
-- CommodityType is a backing store for commodity data. Commodities are
-- registered at game startup and serialized by name/ID instead of using table
-- identities to refer to commodities.
--

---@class CommodityType
local CommodityType = utils.class('CommodityType')


-- Constructor:
--
-- Construct a CommodityType from passed commodity data and common defaults.
-- The following data fields are used if present:
--
-- name          - string, unique ID of the commodity
--
-- l10n_key      - string, translation key used when displaying this commodity
--                 to the user (e.g. in ship cargo display)
--
-- l10n_resource - string, translation resource to look up the translation key
--                 in. Defaults to "commodity".
--
-- price         - number, average expected purchase price of the commodity
--                 before any price adjustments due to supply or demand.
--
-- mass          - number, mass per 1m^3 ideal volume unit of cargo.
--
-- life_support  - optional number, the required life support level of the
--                 vessel to preserve this commodity
--
-- economy_type  - optional string, economy ID of the primary producer of this
--                 commodity. Defaults to nil.
--
-- purchasable   - boolean, controls whether the commodity type will be
--                 considered for sale or purchase at stations. Mission items
--                 should usually not be purchasable. Defaults to false.
--
-- icon_name     - string, name of the icon file used for this commodity.
--                 Defaults to "".
--
function CommodityType:Constructor(name, data)
	-- Initialize sane defaults
	self.name          = name
	self.l10n_key      = "UNKNOWN_CARGO"
	self.l10n_resource = CARGOLANGRESOURCE
	self.price         = 0
	self.mass          = 1
	self.life_support  = 0
	self.economy_type  = nil
	self.purchasable   = false
	self.icon_name     = "Default"

	-- Overwrite defaults with any custom data the registrar needs to include
	for k, v in pairs(data) do self[k] = v end

	local l = Lang.GetResource(self.l10n_resource)
	---@type { name: string, description: string }
	self.lang = {
		name = l[self.l10n_key],
		description = l:get(self.l10n_key .. "_DESCRIPTION") or ""
	}
end

-- Method: GetName()
--
-- Returns the translated name of this commodity
function CommodityType:GetName()
	return self.lang.name
end

-- Method: GetDescription()
--
-- Returns the translated description of this commodity
function CommodityType:GetDescription()
	return self.lang.description
end

---@type table<string, CommodityType>
CommodityType.registry = {}

-- Function: RegisterCommodity
--
-- Add a CommodityType to the registry under a specific name. Commodity names
-- should be used to uniquely identify commodities instead of table identities.
--
-- Static member function.
function CommodityType.RegisterCommodity(name, info)
	assert(not CommodityType.registry[name])

	local commodity = CommodityType.New(name, info)

	CommodityType.registry[name] = commodity
	Serializer:RegisterPersistent("CommodityType." .. name, commodity)

	return commodity
end

-- Function: GetCommodity
--
-- Return the CommodityType registered for the given name or nil.
--
-- Static member function.
---@return CommodityType
function CommodityType.GetCommodity(name)
	return CommodityType.registry[name]
end

-- Serialize commodity types by object, but load by name
function CommodityType:Serialize()
	return self
end

-- Ensure loaded commodity types always point at the 'canonical' instance of the commodity;
-- commodity types not defined by the current version of the code will be loaded verbatim
function CommodityType.Unserialize(data)
	setmetatable(data, CommodityType.meta)

	if not CommodityType.registry[data.name] then
		logWarning('Commodity type ' .. data.name .. ' could not be found, are you loading an outdated save?')
		CommodityType.registry[data.name] = data
	end

	return data
end

Serializer:RegisterClass('CommodityType', CommodityType)

return CommodityType
