-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local onConfigureStarSystem = function (star_system)

	local colour     = star_system.factionColour
	local population = star_system.population / 2
	
	if population < 0.1 then
		colour.a = 0.1
	elseif population < 1 then 
		colour.a = population
	else                            
		colour.a = 1
	end
		
	star_system:SetFactionColour(colour.r, colour.g, colour.b, colour.a)
end

Event.Register("onConfigureStarSystem", onConfigureStarSystem)

