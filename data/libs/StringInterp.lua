-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- a nice string interpolator
string.interp = function (s, t)
	return (s:gsub('(%b{})', function(w) return t[w:sub(2,-2)] or w end))
end
