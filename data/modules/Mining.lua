-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Player = package.core["Player"]
local Engine = require 'Engine'
local Commodities = require 'Commodities'

function Player:SpawnMiningContainer(body)
	-- this function is called from C++ when a mining laser shot hits the surface of an asteroid body and chips off a peice
	-- body is a valid SystemBody object
	-- return value needs to be a valid CommodityType object

	local r = Engine.rand:Number()
	local m = body.metallicity

	if (r*20 < m) then
		return Commodities.precious_metals
	elseif (r*8 < m) then
		return Commodities.metal_alloys
	elseif r < m then
		return Commodities.metal_ore
	elseif r < 0.5 then
		return Commodities.carbon_ore
	end
	return Commodities.rubbish
end
