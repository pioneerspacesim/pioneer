-- Copyright © 2008-2022 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local pigui = require 'Engine'.pigui
local Vector2 = Vector2

local defaultBaseResolution = Vector2(1600, 900)

--
-- Function: ui.rescaleUI
--
-- ui.rescaleUI(val, baseResolution, rescaleToScreenAspect, targetResolution)
--
-- Scales a set of values (normally a size or a position) based on a base
-- resolution and the current or target resultion.
--
--
-- Example:
--
-- > size = ui.rescaleUI(Vector2(96, 96), Vector2(1600, 900))
--
-- Parameters:
--
--   val                   - number|Vector2|Table, the values to scale
--   baseResolution        - (Optional) Vector2, the resolution at which val is valid
--   rescaleToScreenAspect - (Optional) number, when scaling a Vector2, scale x and y
--                           appropriately to match the given aspect ratio
--   targetResolution      - (Optional) Vector2, the target resolution to scale
--                           the value to. Default: current screen resolution.
--
-- Returns:
--
--   number|Vector2|Table - the scaled value
--
local function rescaleUI(val, baseResolution, rescaleToScreenAspect, targetResolution)
	if not targetResolution then
		targetResolution = Vector2(pigui.screen_width, pigui.screen_height)
	end

	if not baseResolution then
		baseResolution = defaultBaseResolution
	end

	local rescaleVector = Vector2(targetResolution.x / baseResolution.x, targetResolution.y / baseResolution.y)
	local rescaleFactor = math.min(rescaleVector.x, rescaleVector.y)
	local type = type(val)

	if type == 'table' then
		local result = {}
		for k, v in pairs(val) do
			result[k] = rescaleUI(v, baseResolution, rescaleToScreenAspect, targetResolution)
		end

		return result
	elseif type == 'userdata' and val.x and val.y then
		return Vector2(val.x * ((rescaleToScreenAspect and rescaleVector.x) or rescaleFactor), val.y * ((rescaleToScreenAspect and rescaleVector.y) or rescaleFactor))
	elseif type == 'number' then
		return val * rescaleFactor
	end
end

return rescaleUI
