-- Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Gravity = {}

Gravity.INFINITY = "oo"
Gravity.TWR_INFINITY_THRESHOLD = 99.99
Gravity.TWR_HUD_WARNING_THRESHOLD = 1.1

--- Surface gravity (m/s2) for TWR at a body, or nil when not meaningful.
function Gravity.GetSurfaceGravity(body)
	if not body then return nil end

	if body.path then
		local sbody = body.path:GetSystemBody()
		if not sbody then return nil end

		if body.type == "STARPORT_ORBITAL" then
			return nil
		end

		if body.superType == "STARPORT" then
			local parent = sbody.parent
			if parent and parent.gravity and parent.gravity > 0 then
				return parent.gravity
			end
			return nil
		end

		if sbody.gravity and sbody.gravity > 0 then
			return sbody.gravity
		end
		return nil
	end

	if body.frameBody and body.frameBody ~= body then
		return Gravity.GetSurfaceGravity(body.frameBody)
	end

	return nil
end

--- Whether the nav target is a surface object (ground starport or landed/docked ship).
function Gravity.IsNavTargetOnSurface(navTarget)
	if not navTarget then return false end

	if navTarget.type == "STARPORT_SURFACE" then
		return true
	end

	if navTarget:IsShip() then
		if navTarget:IsLanded() then
			local frame = navTarget.frameBody
			return frame and not frame:IsDynamic() and frame.superType ~= "STARPORT"
		end
		if navTarget:IsDocked() then
			local station = navTarget:GetDockedWith()
			return station and station.type == "STARPORT_SURFACE"
		end
	end

	return false
end

--- Gravity (m/s²) at a body's position in its frame, or nil when negligible.
function Gravity.GetGravityAtBody(body)
	if not body then return nil end

	local frame = body.frameBody
	if not frame then return nil end

	if frame.superType == "STARPORT" then
		if frame.type == "STARPORT_ORBITAL" then
			return nil
		end
		return Gravity.GetSurfaceGravity(frame)
	end

	if frame:IsDynamic() then
		return nil
	end

	local sbody = frame.path and frame.path:GetSystemBody()
	if not sbody or not sbody.gravity or sbody.gravity <= 0 then
		return nil
	end

	local radius = sbody.radius
	if not radius or radius <= 0 then
		return nil
	end

	local distance = body:GetPositionRelTo(frame):length()
	if distance <= 0 then
		return nil
	end

	return sbody.gravity * (radius / distance)^2
end

--- Thrust-to-weight ratio, or nil when it should be shown as infinite.
function Gravity.CalcTWR(upAccel, gravity)
	if not gravity or gravity <= 0 then
		return nil
	end
	local twr = upAccel / gravity
	if twr > Gravity.TWR_INFINITY_THRESHOLD then
		return nil
	end
	return twr
end

--- Formatted TWR string for display.
function Gravity.FormatTWR(upAccel, gravity)
	local twr = Gravity.CalcTWR(upAccel, gravity)
	if not twr then
		return Gravity.INFINITY
	end
	return string.format("%.2f", twr)
end

return Gravity
