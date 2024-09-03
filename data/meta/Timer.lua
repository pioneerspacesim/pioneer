-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

---@meta

---@class Timer
local Timer = {}

---@param interval number
---@param callback function
function Timer:CallEvery(interval, callback) end

---@param delay number
---@param callback function
function Timer:CallAt(delay, callback) end

return Timer
