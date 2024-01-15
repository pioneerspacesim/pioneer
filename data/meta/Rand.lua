-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

---@meta

---@class Rand
local Rand = {}

---@param seed? number|string
---@return Rand
function Rand.New(seed) end

-- Returns a random number in the range [min, max), exclusive.
---@param min number inclusive minimum value, defaults to 0
---@param max number exclusive maximum value, defaults to 1
---@return number
---@overload fun(self: self): number
---@overload fun(self: self, max: number): number
function Rand:Number(min, max) end

-- Returns a random integer in the range [min, max), exclusive.
---@param min integer inclusive minimum value, defaults to 0
---@param max integer exclusive maximum value, defaults to 1
---@return integer
---@overload fun(self: self): integer
---@overload fun(self: self, max: integer): integer
function Rand:Integer(min, max) end

-- Generates a random integer drawn from a Poisson distribution.
---@param lambda number the mean of the distribution
---@return integer
function Rand:Poisson(lambda) end

-- Generates a random number drawn from a Gaussian distribution
---@param mean number? the mean (center) of the distribution. If omitted, defaults to 0.
---@param stddev number? the standard deviation (width) of the distribution. If omitted, defaults to 1.
---@return number
function Rand:Normal(mean, stddev) end

return Rand
