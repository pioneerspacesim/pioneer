-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Json = require 'Json'
local FileSystem = require 'FileSystem'

local utils = require 'utils'

-- This file implements a generic "station industry".
--
-- It is purposefully written in such a way that industry selection and
-- aggregation can be moved to C++ in a trivial fashion to improve
-- performance.
--
-- To that end, please do not mutate the list of available industries
-- at runtime from Lua; if you plan to mod the list of industries, you
-- should use a JSON patch to one of the data/economy/industries/*.json
-- files instead.

-- Modifiers to an industry's commodity consumption/production
---@class Economy.IndustryModifier
---@field inputs table<string, integer>
---@field outputs table<string, integer>

---@class Economy.IndustryDef
---@field id string
---@field clone fun(self, table): self
local IndustryDef = utils.proto('Economy.IndustryDef')

-- Required host starport type for this industry
---@type "orbital"|"surface"?
IndustryDef.context = nil

-- List of conditions required to select this industry (all must be met)
---@type string[]
IndustryDef.conditions = {}

-- List of conditions required to select this industry (one must be met)
---@type string[]
IndustryDef.conditions_any = {}

-- Commodities consumed by this industry
---@type table<string, integer>
IndustryDef.inputs = {}
-- Commodities produced by this industry
---@type table<string, integer>
IndustryDef.outputs = {}

-- Map of optional conditions to demand/production modifiers
---@type table<string, Economy.IndustryModifier>
IndustryDef.modifiers = {}

-- Amount of workforce required to operate this industry
IndustryDef.workforce = 1

local function parseModifierString(i, o, str)
	local io, comm, effect, val = string.match(str, "([io]):(%g+)([%+%-])(%d+)")
	local amt = effect == "-" and -tonumber(val) or tonumber(val)

	if io == "i" then
		i[comm] = (i[comm] or 0) + amt
	else
		o[comm] = (o[comm] or 0) + amt
	end
end

local function parseModifiers(tbl)
	local modifiers = {}

	for id, mod in pairs(tbl) do

		local inputs = {}
		local outputs = {}

		for _, str in ipairs(mod) do
			parseModifierString(inputs, outputs, str)
		end

		modifiers[id] = { inputs = inputs, outputs = outputs }
	end

	return modifiers
end

local Industry = {}

---@return Economy.IndustryDef
function Industry.NewIndustry(id, tbl)
	local def = IndustryDef:clone(tbl)

	def.id = id
	def.modifiers = parseModifiers(tbl.modifiers or {})

	return def
end

---@type table<string, Economy.IndustryDef>
Industry.industries = {}

-- List of industry ids present in the industries table
---@type string[]
Industry.industryList = {}

---@param tags table<string, boolean>
---@param conditions string[]
local function checkConditions(tags, conditions)
	for _, str in ipairs(conditions) do
		local inv, cond = str:match("(!?)(%g+)")

		-- Logical XNOR: return false if condition is missing when we want it, or present when we don't
		if (inv ~= "") == (tags[cond] and true or false) then
			return false
		end
	end

	return true
end

local function checkConditionsAny(tags, conditions)
	for _, cond in ipairs(conditions) do
		if tags[cond] then return true end
	end

	return #conditions == 0
end

-- Apply the set of condition modifiers to commodity values
---@param mods table<string, integer>
---@param vals table<string, integer>
local function applyModifiers(mods, vals)
	for id, amt in pairs(mods) do
		if id == '*' then
			for k, v in pairs(vals) do
				vals[k] = v + amt
			end
		else
			vals[id] = (vals[id] or 0) + amt
		end
	end
end

-- Compute the maximum commodity value using the logic:
-- 1. A commodity output of N can supply a singular industry input of amount N
-- 2. A commodity output of N can supply infinite industry inputs of amount N-1
-- 3. A singular industry with an output of N produces a station-wide output of N
-- 4. 2+ industries with outputs of N produce a station-wide output of N+1
--
-- Thus, two or more commodity values of the same underlying N sum to N+1,
-- but two values of N do not sum with a third value of N+1 to produce N+2.
local function sumIndustryVal(value, current, maxValue)
	if value == maxValue then
		return math.max(value + 1, current), maxValue
	else
		return math.max(value, current or 0), math.max(value, maxValue or 0)
	end
end

-- Generate the list of industries present on this starport and the economic effects
-- of those industries.
--
---@param sbody SystemBody
---@param tags table<string, boolean>
---@param sizeClass integer
---@param rand Rand
---
---@return string[] industries
---@return table<string, number> production
---@return table<string, number> demand
function Industry.GenerateIndustries(sbody, tags, sizeClass, rand, supply, demand)
	local avail_industries = Industry.GetAvailableIndustries(sbody, tags)
	local numIndustries = 2 + sizeClass
	local industries = {}

	-- print(sbody.name .. "\nValid industries: " .. table.concat(avail_industries, ",\t"))

	-- TODO: smarter industry selection loop
	for i = 1, numIndustries do
		local idx = rand:Integer(1, #avail_industries)

		table.insert(industries, avail_industries[idx])
		table.remove(avail_industries, idx)

		if #avail_industries == 0 then
			break
		end
	end

	supply = supply or {}
	demand = demand or {}

	Industry.ComputeSupplyDemand(industries, tags, supply, demand)

	return industries, supply, demand
end

-- Sum together all supply and demand numbers for the given list of industries, computing modifiers
function Industry.ComputeSupplyDemand(industries, tags, supply, demand)
	local maxSupply = table.copy(supply)
	local maxDemand = table.copy(demand)

	for _, id in ipairs(industries) do

		local s, d = Industry.ComputeModifiers(Industry.industries[id], tags)

		for commodity, value in pairs(s) do
			supply[commodity], maxSupply[commodity] = sumIndustryVal(value, supply[commodity], maxSupply[commodity])
		end

		for commodity, value in pairs(d) do
			demand[commodity], maxDemand[commodity] = sumIndustryVal(value, demand[commodity], maxDemand[commodity])
		end

	end
end

-- Get a list of industry IDs which are valid for this specific body and set of condition tags.
--
---@param sbody SystemBody
---@param tags table<string, boolean>
---@return string[]
function Industry.GetAvailableIndustries(sbody, tags)
	local context = sbody.type == "STARPORT_ORBITAL" and "orbital" or "surface"

	return utils.filter_array(Industry.industryList, function(id)
		local industry = Industry.industries[id]

		return (not industry.context or industry.context == context)
			and checkConditions(tags, industry.conditions)
			and checkConditionsAny(tags, industry.conditions_any)
	end)
end

-- Compute the total modified supply and demand for a specific industry, given the local conditions.
--
---@param industry Economy.IndustryDef
---@param tags table<string, boolean>
function Industry.ComputeModifiers(industry, tags)
	local supply = table.copy(industry.outputs)
	local demand = table.copy(industry.inputs)

	for cond, mod in pairs(industry.modifiers) do
		if tags[cond] then

			applyModifiers(mod.outputs, supply)
			applyModifiers(mod.inputs, demand)

		end
	end

	return supply, demand
end

local industry_files = FileSystem.ReadDirectory('data://economy/industries')

for _, file in ipairs(industry_files) do
	table.merge(Industry.industries, Json.LoadJson(file.path), function(id, tbl) return id, Industry.NewIndustry(id, tbl) end)
end

Industry.industryList = utils.build_array(utils.keys(Industry.industries))
table.sort(Industry.industryList)

return Industry
