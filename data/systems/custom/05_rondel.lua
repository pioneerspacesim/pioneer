-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- example of a custom system directly specifying a faction
local s = CustomSystem:new('Rondel',{'STAR_M'})
	:faction('Red')
	:short_desc('Military Listening Post')
	:long_desc([[A hidden dagger pointed at the heart of the Federation]])
	:seed(1347)

s:add_to_sector(-2,0,0,v(0.007,0.260,0.060))
