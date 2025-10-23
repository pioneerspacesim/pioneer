-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Json = require 'Json'
local Rand  = require 'Rand'

local utils = require 'utils'

-- Economic conditions are generic string tags applied to systems, planets, and
-- starports. They provide a moddable, data-driven way to express specific
-- attribute breakpoints as boolean flags.

---@enum Economy.ConditionOp
local Op = {
	GREATER = '>',
	LESS = '<',
	GE = '>=',
	LE = '<=',
	EQUAL = '='
}

---@class Economy.ConditionDef
---@field id string lowercase string id of this condition, used as i18n_key
---@field context string type of body this condition applies to
---@field hasRandomCond boolean? implementation detail
---@field conditions { field: string, op: Economy.ConditionOp, value: number|string }[]
---@field clone fun(self, table): self
local ConditionDef = utils.proto('Economy.ConditionDef')

---@param context SystemBody|StarSystem
function ConditionDef:evaluate(context)

	local random = self.hasRandomCond and Rand.New(context.seed .. "-cond-" .. self.id)

	for _, cond in ipairs(self.conditions) do

		local val = 0.0
		local pass = nil

		if cond.field == "random" then
			-- If we have a dependency on a random number, hasRandomCond will be true
			---@cast random Rand
			val = random:Number()
		else
			val = context[cond.field]
		end

		if cond.op == Op.GREATER then
			pass = val > cond.value
		elseif cond.op == Op.LESS then
			pass = val < cond.value
		elseif cond.op == Op.EQUAL then
			pass = val == cond.value
		elseif cond.op == Op.GE then
			pass = val >= cond.value
		elseif cond.op == Op.LE then
			pass = val <= cond.value
		end

		-- Implicit AND between each condition
		if not pass then
			return false
		end

	end

	return true

end

---@param cond string
local function parseCondition(cond)
	local field, op, val = cond:match("(%g+)%s*([><=]+)%s*(%g+)")
	assert(field and op and val, "Invalid condition definition string '" .. cond .. "'")

	return { field = field, op = op, value = tonumber(val) or val }
end

local function newCondDef(id, tab)
	local def = ConditionDef:clone({
		id = id,
		context = tab.context,
		conditions = utils.map_array(tab.conditions, parseCondition)
	})

	for _, cond in ipairs(def.conditions) do
		if cond.field == "random" then
			def.hasRandomCond = true
			break
		end
	end

	return def
end

local Conditions = {}

---@type Economy.ConditionDef[]
Conditions.planet = {}
---@type Economy.ConditionDef[]
Conditions.system = {}
---@type Economy.ConditionDef[]
Conditions.surface = {}
---@type Economy.ConditionDef[]
Conditions.orbital = {}

---@type table<string, Economy.ConditionDef>
Conditions.by_name = {}

---@param context SystemBody | StarSystem
---@param tags table<string, boolean>?
---@return table<string, boolean>
function Conditions.Evaluate(context, tags)

	tags = tags or {}
	local conditions = Conditions.planet

	if getmetatable(context).type == "StarSystem" then
		conditions = Conditions.system
	else
		if context.type == "STARPORT_SURFACE" then
			conditions = Conditions.surface
		elseif context.type == "STARPORT_ORBITAL" then
			conditions = Conditions.orbital
		end

		-- Optimization: use the body's type/superType as implicit conditions
		-- (no need to evaluate a custom condition def)
		tags[string.lower(context.type)] = true
		tags[string.lower(context.superType)] = true
	end

	for _, def in ipairs(conditions) do

		if def:evaluate(context) then
			tags[def.id] = true
		end

	end

	return tags

end

for id, def in pairs(Json.LoadJson('economy/conditions/basic.json')) do
	local cond = newCondDef(id, def)

	table.insert(Conditions[cond.context], cond)
	Conditions.by_name[cond.id] = cond
end

return Conditions
