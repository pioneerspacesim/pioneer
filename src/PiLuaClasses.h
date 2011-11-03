#ifndef _PILUACLASSES_H
#define _PILUACLASSES_H

/*
 * OOLua classes for fixed, vector and matrix types. Be sure to include
 * data/pistartup.lua in your Lua startup script (for now, until
 * MyLuaMathTypes is gone)
 */

#include "oolua/oolua.h"

#include "fixed.h"
#include "vector3.h"

class pi_fixed {
public:
	pi_fixed(): f() { }
	pi_fixed(int n, int d): f(n,d) { }
	pi_fixed(const pi_fixed &a) { f = a; }
	pi_fixed(const fixed &a) { f = a; }

    inline operator fixed () const { return f; }
	inline fixed to_fixed() const { return f; }

	inline float tonumber() const { return f.ToFloat(); }

	inline bool operator==(const pi_fixed &a) const { return f == a.to_fixed(); }
	inline bool operator!=(const pi_fixed &a) const { return f != a.to_fixed(); }
	inline pi_fixed operator+(const pi_fixed &a) const { return f + a.to_fixed(); }
	inline pi_fixed operator-(const pi_fixed &a) const { return f - a.to_fixed(); }
	inline pi_fixed operator*(const pi_fixed &a) const { return f * a.to_fixed(); }
	inline pi_fixed operator/(const pi_fixed &a) const { return f / a.to_fixed(); }
	
private:
	fixed f;
};

OOLUA_CLASS_NO_BASES(pi_fixed)
	OOLUA_TYPEDEFS
		Equal_op,
		Not_equal_op,
		Add_op,
		Sub_op,
		Mul_op,
		Div_op
	OOLUA_END_TYPES
	OOLUA_CONSTRUCTORS_BEGIN
		OOLUA_CONSTRUCTOR_2(int, int)
	OOLUA_CONSTRUCTORS_END
	OOLUA_MEM_FUNC_0_CONST(float, tonumber)
OOLUA_CLASS_END


class pi_vector {
public:
	pi_vector(): v(0,0,0) { }
	pi_vector(float x_, float y_, float z_): v(x_,y_,z_) { }
	pi_vector(const pi_vector &a) { v = a.to_vector3f(); }
	pi_vector(const vector3f &a) { v = a; }

    inline operator vector3f () const { return v; }
	inline vector3f to_vector3f() const { return v; }

	//inline bool operator==(const pi_vector &a) const { return v == a.to_vector3f(); }
	//inline bool operator!=(const pi_vector &a) const { return v != a.to_vector3f(); }
	inline pi_vector operator+(const pi_vector &a) const { return v + a.to_vector3f(); }
	inline pi_vector operator-(const pi_vector &a) const { return v - a.to_vector3f(); }

	inline float x() const { return v.x; }
	inline float y() const { return v.y; }
	inline float z() const { return v.z; }

	inline pi_vector norm() const { return v.Normalized(); }
	inline float dot(const pi_vector &a) const { return v.Dot(a.to_vector3f()); }
	inline pi_vector cross(const pi_vector &a) const { return v.Cross(a.to_vector3f()); }
	inline float len() const { return v.Length(); }

	inline void print() const { v.Print(); }
	
private:
	vector3f v;
};

OOLUA_CLASS_NO_BASES(pi_vector)
	OOLUA_TYPEDEFS
		//Equal_op,
		//Not_equal_op,
		Add_op,
		Sub_op
	OOLUA_END_TYPES
	OOLUA_CONSTRUCTORS_BEGIN
		OOLUA_CONSTRUCTOR_3(float, float, float)
	OOLUA_CONSTRUCTORS_END
	OOLUA_MEM_FUNC_0_CONST(float, x)
	OOLUA_MEM_FUNC_0_CONST(float, y)
	OOLUA_MEM_FUNC_0_CONST(float, z)
	OOLUA_MEM_FUNC_0_CONST(pi_vector, norm)
	OOLUA_MEM_FUNC_1_CONST(float, dot, const pi_vector &)
	OOLUA_MEM_FUNC_1_CONST(pi_vector, cross, const pi_vector &)
	OOLUA_MEM_FUNC_0_CONST(float, len)
	OOLUA_MEM_FUNC_0_CONST(void, print)
OOLUA_CLASS_END

namespace PiLuaClasses
{
	void RegisterClasses(lua_State *l);
}

#endif
