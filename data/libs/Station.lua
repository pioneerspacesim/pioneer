-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

DOCKING_TIMEOUT_SECONDS = 300

--for station waypoint interpolation etc
function vlerp(t, v1, v2)
	return t*v2 + (1.0-t)*v1
end
