local Player = import_core("Player")
local Engine = import("Engine")
local Equipment = import("Equipment")

function Player:SpawnMiningContainer(body)
	-- this function is called from C++ when a mining laser shot hits the surface of an asteroid body and chips off a peice
	-- body is a valid SystemBody object
	-- return value needs to be a valid Equipment.cargo.* object

	local r = Engine.rand:Number()
	local m = body.metallicity

	if (r*20 < m) then
		return Equipment.cargo.precious_metals
	elseif (r*8 < m) then
		return Equipment.cargo.metal_alloys
	elseif r < m then
		return Equipment.cargo.metal_ore
	elseif r < 0.5 then
		return Equipment.cargo.carbon_ore
	end
	return Equipment.cargo.rubbish
end
