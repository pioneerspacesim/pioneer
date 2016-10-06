-- Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Color = {}
do
	local meta = {
		_metatable = "Private metatable",
		_DESCRIPTION = "Colors RGBA 0..255"
	}

	meta.__index = meta

	function meta:__tostring()
		return ("<Color %g, %g, %g, %g>"):format(self.r, self.g, self.b, self.a)
	end
	function meta:rgb()
		return r,g,b
	end
	function meta:rgba()
		return r,g,b,a
	end
	function meta:shade(factor)
		return Color(self.r * (1 - factor),
					 self.g * (1 - factor),
					 self.b * (1 - factor),
					 self.a)
	end
	function meta:tint(factor)
		return Color(self.r + (255 - self.r) * factor,
					 self.g + (255 - self.g) * factor,
					 self.b + (255 - self.b) * factor,
					 self.a)
	end
	setmetatable( Color, {
					  __call = function( V, r ,g ,b, a )
						  local result
						  if type(x) == "table" then
							  result = { r = r.r or 0, g = r.g or 0, b = r.b or 0, a = r.a or 255 }
						  else
							  result = {r = r or 0, g = g or 0, b = b or 0, a = a or 255}
						  end
						  return setmetatable( result, meta ) end
	} )
end

Color.__index = Color

return Color

