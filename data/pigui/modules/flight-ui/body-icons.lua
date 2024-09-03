-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local utils = require 'utils'
local ui = require 'pigui'

local icons = ui.theme.icons

local ASTEROID_RADIUS = 450000 -- rocky planets smaller than this (in meters) are considered an asteroid, not a planet

-- Return the icon used to display the given body
-- If forWorld is true, returns an icon variant suitable
-- for display at small resolutions in the world view
local function getBodyIcon(body, forWorld)
	local st = body.superType
	local t = body.type
	local sb = body:isa("SystemBody") and body or body:GetSystemBody()

	if st == "STARPORT" then
		local population = sb.population
		if t == "STARPORT_ORBITAL" then
			if population > (22000 / 1e9) then
				return icons.station_orbital_large
			elseif population > (15000 / 1e9) then
				return icons.spacestation
			else
				return icons.station_orbital_small
			end
		elseif body.type == "STARPORT_SURFACE" then
			if forWorld then return icons.starport end

			if population > (180000 / 1e9) then
				return icons.starport_surface
			elseif population > (120000 / 1e9) then
				return icons.outpost_large
			else
				return icons.outpost_medium
			end
			-- don't use the outpost or surface starport icons
			-- we need to increase the size of icon rendering first
		end
	elseif st == "GAS_GIANT" then
		return icons.gas_giant
	elseif st == "STAR" then
		return icons.sun
	elseif st == "ROCKY_PLANET" then
		if t == "PLANET_ASTEROID" or sb.radius < ASTEROID_RADIUS then
			return icons.asteroid_hollow
		end

		if sb.isMoon then
			return icons.moon
		else
			return icons.rocky_planet
		end
	elseif sb == body then
		print("getBodyIcon(): not sure how to process systembody, supertype: " .. (st and st or "nil") .. ", type: " .. (t and t or "nil"))
		utils.print_r(body)
		return icons.info
	elseif body:IsShip() then
		local shipClass = body:GetShipClass()
		if icons[shipClass] then
			return icons[shipClass]
		else
			print("getBodyIcon(): unknown ship class " .. (shipClass and shipClass or "nil"))
			return icons.ship -- TODO: better icon
		end
	elseif body:IsHyperspaceCloud() then
		return icons.hyperspace -- TODO: better icon
	elseif body:IsMissile() then
		return icons.bullseye -- TODO: better icon
	elseif body:IsCargoContainer() then
		return icons.rocky_planet -- TODO: better icon
	else
		print("getBodyIcon(): not sure how to process body, supertype: " .. (st and st or "nil") .. ", type: " .. (t and t or "nil"))
		utils.print_r(body)
		return icons.ship
	end
end

return getBodyIcon
