-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- This file implements type information about C++ classes for Lua static analysis

---@meta

---@class Economy
local Economy = {}

package.core['Economy'] = Economy

---@class Economy.CPPCommodityInfo
---@field id integer
---@field producer integer
---@field name string
---@field l10n_key string
---@field inputs { id: integer, amount: number }[]
---@field price number

---@class Economy.CPPEconomyInfo
---@field id integer
---@field name string
---@field l10n_key { small: string, medium: string, large: string, huge: string }
---@field affinity { agricultural: number, industrial: number, metallicity: number }
---@field generation { agricultural: number, industrial: number, metallicity: number, population: number, random: number }

---@return table<string, Economy.CPPCommodityInfo>
function Economy.GetCommodities() end

---@param id integer
---@return Economy.CPPCommodityInfo
function Economy.GetCommodityById(id) end

---@return table<string, Economy.CPPEconomyInfo>
function Economy.GetEconomies() end

---@param id integer
---@return Economy.CPPEconomyInfo
function Economy.GetEconomyById(id) end


return Economy
