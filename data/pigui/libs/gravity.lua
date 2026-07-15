-- Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- Gravity helpers for PiGui. Intended for showing TWR readouts.

local Game = require 'Game'

local Gravity = {}

--- Surface gravity for a body, walking up through starports if needed.
function Gravity.GetSurfaceGravity(body)
	if not body then return nil end

	local root = body
	local sbody = root:GetSystemBody()
	if not sbody then
		root = body.frameBody
		if not root then return nil end
		sbody = root:GetSystemBody()
		if not sbody then return nil end
	end

	-- Surface-starport targets use the parent body's surface gravity.
	-- Orbital starports have no "surface gravity" and return nil.
	if sbody.superType == "STARPORT" then
		if sbody.type == "STARPORT_SURFACE" and sbody.parent then
			sbody = sbody.parent
		else
			return nil
		end
	end

	if not sbody then return nil end

	return sbody.gravity
end

--- Gravity acceleration at a body's current position.
--- Returns nil only if the body does not exist or the player is in hyperspace.
function Gravity.GetGravityAtBody(body)
	if not body or Game.InHyperspace() then return nil end
	if not body:IsDynamic() then return nil end

	local mass = body:GetMass()
	if mass <= 0 then return nil end

	return body:GetGravityForce():length() / mass
end

return Gravity
