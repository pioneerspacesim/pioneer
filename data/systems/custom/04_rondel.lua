-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- example of a custom system directly specifying a faction
local s = CustomSystem:new('Rondel',{'STAR_M'})
	:faction('Haber Corporation')
	:short_desc('Military Listening Post')
	:long_desc([[A hidden dagger pointed at the heart of the Federation]])
	:seed(1824351)

s:add_to_sector(-1,6,2,v(0.007,0.260,0.060))
