-- Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine     = require 'Engine'
local Space      = require 'Space'
local Timer      = require 'Timer'
local utils      = require 'utils'
local Game       = require 'Game'

local ProximityQuery = {}

-- ─── Context ─────────────────────────────────────────────────────────────────

local onBodyEnter = function(ctx, enter) end
local onBodyLeave = function(ctx, leave) end

-- List of bodies currently in proximity is stored with weak keys, so deleted
-- bodies are quietly cleaned up during a GC cycle
local bodyListMt = {
	__mode = "k"
}

---@class ProximityQuery.Context
---@field New fun(body, dist, interval, type): self
---@field body Body
---@field dist number
local Context = utils.class 'ProximityQuery.Context'

function Context:Constructor(body, dist, type)
	self.bodies = setmetatable({}, bodyListMt)
	self.body = body
	self.dist = dist
	self.filter = type
	self.iter = 0

	self.onBodyEnter = onBodyEnter
	self.onBodyLeave = onBodyLeave
end

function Context:Cancel()
	self.iter = nil
end

---@param fn fun(ctx: ProximityQuery.Context, enter: Body)
function Context:SetBodyEnter(fn)
	self.onBodyEnter = fn
	return self
end

---@param fn fun(ctx: ProximityQuery.Context, leave: Body)
function Context:SetBodyLeave(fn)
	self.onBodyLeave = fn
	return self
end

-- Class: ProximityQuery
--
-- This class provides a helper utility to allow using Space.GetBodiesNear() in
-- an efficient manner, providing an event-based API when bodies enter or leave
-- a user-specified relevancy range of the reference body.

-- Function: CreateQuery
--
-- Register a new periodic proximity test relative to the given body
--
-- Parameters:
--
--   body     - the reference body to perform testing for
--   dist     - the distance (in meters) of the proximity test to perform
--   interval - how often a proximity test should be performed in seconds.
--              Smaller values are more performance-hungry.
--   type     - optional body classname filter, see Space.GetBodiesNear
--   overlap  - if false, all bodies of type in the radius will generate
--                initial proximity events on the first proximity test
--
-- Returns:
--
--   context - the context object for the registered test
--
function ProximityQuery.CreateQuery(body, dist, interval, type, overlap)
	local context = Context.New(body, dist, type)
	local cb = ProximityQuery.MakeCallback(context)

	-- Queue the start of the timer at a random timestamp inside the first interval period
	-- This provides natural load balancing for large numbers of callbacks created on the same frame
	-- (e.g. at game start / hyperspace entry)
	Timer:CallAt(Game.time + Engine.rand:Number(interval), function()

		if overlap == false then
			cb()
		else
			-- Pre-fill the list of nearby bodies (avoid spurious onBodyEnter() callbacks when creating)
			for i, locBody in ipairs(Space.GetBodiesNear(body, dist, type)) do
				context.bodies[locBody] = context.iter
			end
		end

		Timer:CallEvery(interval, cb)
	end)

	return context
end

---@private
---@param context ProximityQuery.Context
function ProximityQuery.MakeCallback(context)
	return function()
		-- Callback has been cancelled or query body no longer exists,
		-- stop ticking this query
		if not context.iter or not context.body:exists() then
			return true
		end

		local newIter = (context.iter + 1) % 2
		context.iter = newIter

		for i, locBody in ipairs(Space.GetBodiesNear(context.body, context.dist, context.filter)) do
			if not context.bodies[locBody] then
				context.onBodyEnter(context, locBody)
			end

			context.bodies[locBody] = newIter
		end

		local remove = {}
		for locBody, ver in pairs(context.bodies) do
			if ver ~= newIter then
				context.onBodyLeave(context, locBody)
				table.insert(remove, locBody)
			end
		end

		for _, v in ipairs(remove) do
			context.bodies[v] = nil
		end
	end
end

return ProximityQuery
