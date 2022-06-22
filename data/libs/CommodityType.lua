-- Copyright © 2008-2022 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Lang = require 'Lang'
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
	self.name          = name
	self.l10n_key      = data.l10n_key      or "UNKNOWN_CARGO"
	self.l10n_resource = data.l10n_resource or CARGOLANGRESOURCE
	self.price         = data.price         or 0
	self.mass          = data.mass          or 1
	self.economy_type  = data.economy_type  or nil
	self.purchasable   = data.purchasable   or false
	self.icon_name     = data.icon_name     or ""

	local l = Lang.GetResource(self.l10n_resource)
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

---@type { [string]: CommodityType }
CommodityType.registry = {}

-- Function: RegisterCommodity
--
-- Add a CommodityType to the registry under a specific name. Commodity names
-- should be used to uniquely identify commodities instead of table identities.
--
-- Static member function.
function CommodityType.RegisterCommodity(name, info)
	assert(not CommodityType.registry[name])

	CommodityType.registry[name] = CommodityType.New(name, info)
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

return CommodityType
