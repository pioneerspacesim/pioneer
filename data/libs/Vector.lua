-- Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Vector = {}

local function binaryOp(a, b, op)
	local typ = type(b)
	if(typ == "number") then
		return Vector(op(a.x, b), op(a.y, b), op(a.z, b))
	elseif(typ == "table") then
		return Vector(op(a.x, b.x), op(a.y, b.y), op(a.z, b.z))
	else
		error("operand to binary op in Vector not number or table but " .. typ)
	end
end

do
	local meta = {
		_metatable = "Private metatable",
		_DESCRIPTION = "Vectors in 3D",
		class = "Vector"
	}

	meta.__index = meta

	function meta:__unm( v )
		return Vector(-self.x, -self.y, -self.z)
	end

	function meta:__add( v )
		return binaryOp(self, v, function(a,b) return a+b end)
	end

	function meta:__sub( v )
		return binaryOp(self, v, function(a,b) return a-b end)
	end

	function meta:__mul( v )
		return binaryOp(self, v, function(a,b) return a*b end)
	end

	function meta:__div( v )
		return binaryOp(self, v, function(a,b) return a/b end)
	end

	function meta:__tostring()
		return ("(%.02g, %.02g, %.02g)"):format(self.x, self.y, self.z)
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
		if(type(other) == "table" and type(other.x) == "number" and type(other.y) == "number" and type(other.z) == "number") then
			return self.x * other.x + self.y * other.y + self.z * other.z
		else
			error("Incorrect parameter to dot product");
		end
	end

	function meta:cross(other)
		if(type(other) == "table" and type(other.x) == "number" and type(other.y) == "number" and type(other.z) == "number") then
			return Vector(self.y * other.z - self.z * other.y,
						  self.x * other.z - self.z * other.x,
						  self.x * other.y - self.y * other.x)
		else
			error("Incorrect parameter to cross product");
		end
	end

	function meta:angle()
		local n = self:normalized()
		local x = math.atan2(n.x, n.y)
		return math.pi * 2 - x
	end

	function meta:rotate2d(angle_rad)
		if(type(angle_rad) ~= "number") then
			error("Parameter to Vector:rotate2d not a number")
		end
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
