local Vector = {}
do
	 local meta = {
			_metatable = "Private metatable",
			_DESCRIPTION = "Vectors in 3D"
	 }

	 meta.__index = meta

	 function meta:__unm( v )
		return Vector(-self.x, -self.y, -self.z)
	 end
	 
	 function meta:__add( v )
			if(type(v) == "number") then
				 return Vector(self.x + v, self.y + v, self.z + v)
			else
				 return Vector(self.x + v.x, self.y + v.y, self.z + v.z)
			end
	 end

	 function meta:__sub( v )
			if(type(v) == "number") then
				 return Vector(self.x - v, self.y - v, self.z - v)
			else
				 return Vector(self.x - v.x, self.y - v.y, self.z - v.z)
			end
	 end

	 function meta:__mul( v )
			if(type(v) == "number") then
				 return Vector(self.x * v, self.y * v, self.z * v)
			else
				 return Vector(self.x * v.x, self.y * v.y, self.z * v.z)
			end
	 end

	 function meta:__div( v )
			if(type(v) == "number") then
				 return Vector(self.x / v, self.y / v, self.z / v)
			else
				 return Vector(self.x / v.x, self.y / v.y, self.z / v.z)
			end
	 end

	 function meta:__tostring()
			return ("<%g, %g, %g>"):format(self.x, self.y, self.z)
	 end

	 function meta:magnitude()
			return math.sqrt( self.x * self.x + self.y * self.y + self.z * self.z)
	 end

	 function meta:normalized()
			local len = math.abs(self:magnitude())
			return Vector(self.x / len, self.y / len, self.z / len)
	 end

	 function meta:left()
			return Vector(-self.y, self.x, 0)
	 end

	 function meta:right()
			return Vector(self.y, -self.x, 0)
	 end

	 function meta:dot(other)
			return self.x * other.x + self.y * other.y + self.z * other.z
	 end

	 function meta:cross(other)
			return Vector(self.y * other.z - self.z * other.y,
										self.x * other.z - self.z * other.x,
										self.x * other.y - self.y * other.x)
	 end

	 function meta:angle()
		local n = self:normalized()
		local x = math.atan2(n.x, n.y)
		return math.pi * 2 - x
	 end
	 
	 function meta:rotate2d(angle_rad)
		local cs = math.cos(angle_rad)
		local sn = math.sin(angle_rad)
		return Vector(self.x * cs - self.y * sn, self.x * sn + self.y * cs, 0)
	 end
	 
	 setmetatable( Vector, {
										__call = function( V, x ,y ,z )
											 local result
											 if type(x) == "table" then
													result = { x = x.x or 0, y = x.y or 0, z = x.z or 0 }
											 else
													result = {x = x or 0, y = y or 0, z = z or 0}
											 end
											 return setmetatable( result, meta ) end
	 } )
end

Vector.__index = Vector

return Vector
