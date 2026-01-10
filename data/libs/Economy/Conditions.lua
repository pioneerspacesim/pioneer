-- Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Json = require 'Json'
local Rand  = require 'Rand'
local FileSystem = require 'FileSystem'

local utils = require 'utils'

-- Interface: Economy.Conditions
--
-- Economic conditions are generic string tags applied to systems, planets, and
-- starports. They provide a moddable, data-driven way to express specific
-- attribute breakpoints as boolean flags.
--
-- Conditions are defined by JSON objects authored in data/economy/conditions/*.json.
-- A definition of the syntax is as follows:
--
-- >  "context": "system" | "planet" | "orbital" | "surface"
-- >  "required": string[]
--
-- For a condition to be applied to a body, all of the expressions in the `required`
-- field must evaluate to true.
--
-- Expression syntax is fairly simple, and takes the form:
--
-- >  <param> OP <value>
--
-- The parameter is any parameter of a <SystemBody> object, or the special value "random".
-- OP is defined below in <Economy.ConditionOp>.
-- The value is either a floating-point number value or a string identifier (no spaces allowed).
-- The comparison specified by OP is evaluated between the value stored in the <SystemBody> being
-- evaluated and the literal value in the expression.
--
-- Note that comparisons between a number and a string (e.g. type >= 0.4) are undefined
-- and likely to cause Lua runtime errors.
--
-- If the parameter is the special string "random", a deterministic random number from 0..1 is
-- generated and compared to the numeric value specified in the expression. This deterministic
-- random number is designed to be persistent across multiple runs of the program and will not
-- change if additional conditions are defined (e.g. by a mod).

-- Interface: Economy

-- Enum: ConditionOp
--
-- GREATER - evaluates to true if the value of `parameter` is numerically greater than the literal value.
-- LESS    - evaluates to true if the value of `parameter` is numerically smaller than the literal value.
-- GE      - evaluates to true if the value of `parameter` is numerically greater or equal to the literal value.
-- LE      - evaluates to true if the value of `parameter` is numerically smaller or equal to the literal value.
-- EQUAL   - evaluates to true if the value of `parameter` is equal to the number or string specified as the literal value.

---@enum Economy.ConditionOp
local Op = {
	GREATER = '>',
	LESS = '<',
	GE = '>=',
	LE = '<=',
	EQUAL = '='
}

-- Class: Economy.ConditionDef
--
-- Defines an economic condition tag.

---@class Economy.ConditionDef
---@field id string lowercase string id of this condition, used as i18n_key
---@field context string type of body this condition applies to
---@field hasRandomCond boolean? implementation detail
---@field required { field: string, op: Economy.ConditionOp, value: number|string }[]
---@field clone fun(self, table): self
local ConditionDef = utils.proto('Economy.ConditionDef')

-- Function: evaluate
--
-- Returns whether this condition applies to the passed context object.
--
-- Parameters:
--
--    context - a <SystemBody> or <StarSystem> object to be evaluated against this condition
--
---@param context SystemBody|StarSystem
function ConditionDef:evaluate(context)

	local random = self.hasRandomCond and Rand.New(context.seed .. "-cond-" .. self.id)

	for _, cond in ipairs(self.required) do

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

-- Parse a condition definition expression, returning a table
---@param cond string
---@return { field: string, op: string, value: string|number }
local function parseCondition(cond)
	local field, op, val = cond:match("(%g+)%s*([><=]+)%s*(%g+)")
	assert(field and op and val, "Invalid condition definition string '" .. cond .. "'")

	return { field = field, op = op, value = tonumber(val) or val }
end

-- Process a JSON object condition definition and convert it to a ConditionDef object
local function newCondDef(id, tab)
	local def = ConditionDef:clone({
		id = id,
		context = tab.context,
		required = utils.map_array(tab.required, parseCondition)
	})

	for _, cond in ipairs(def.required) do
		if cond.field == "random" then
			def.hasRandomCond = true
			break
		end
	end

	return def
end

-- Interface: Economy.Conditions
local Conditions = {}

-- Table: planet
-- A list of conditions which apply to the "planet" context. Evaluated against <SystemBodies>
-- which are not starports.
---@type Economy.ConditionDef[]
Conditions.planet = {}
-- Table: system
-- A list of conditions which apply to the "system" context. Evaluated against <StarSystems>.
---@type Economy.ConditionDef[]
Conditions.system = {}
-- Table: surface
-- A list of conditions which apply to the "surface" context. Evaluated against <SystemBodies>
-- with a type of STARPORT_SURFACE.
---@type Economy.ConditionDef[]
Conditions.surface = {}
-- Table: orbital
-- A list of conditions which apply to the "orbital" context. Evaluated against <SystemBodies>
-- with a type of STARPORT_ORBITAL.
---@type Economy.ConditionDef[]
Conditions.orbital = {}

-- Table: by_name
-- A map of all registered condition definitions keyed by their names.
---@type table<string, Economy.ConditionDef>
Conditions.by_name = {}

-- Function: Evaluate
--
-- Given an input context object (either a <SystemBody> or <StarSystem>) and an
-- optional set of pre-existing condition tags, evaluate all applicable <ConditionDefs>
-- against the context and add them to the condition tags.
--
-- Parameters:
--    context: a <SystemBody> or <StarSystem> object
--    tags: optional, a table mapping strings to boolean true values.
--
-- Returns:
--    tags: the input list of tags or a new table, with all applicable condition IDs listed as true.
--
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

local function loadJsonFile(filepath)
	for id, def in pairs(Json.LoadJson(filepath)) do
		local cond = newCondDef(id, def)

		table.insert(Conditions[cond.context], cond)
		Conditions.by_name[cond.id] = cond
	end
end

for _, fileinfo in ipairs(FileSystem.ReadDirectory('data://economy/conditions')) do
	if fileinfo.name:match(".json$") then
		loadJsonFile(fileinfo.path)
	end
end


return Conditions
