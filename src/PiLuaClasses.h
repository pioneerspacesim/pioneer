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
#include "matrix4x4.h"

class pi_fixed {
public:
	pi_fixed(): f(0,0) { }
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

	inline bool operator==(const pi_vector &a) const { return v == a.to_vector3f(); }
	inline bool operator!=(const pi_vector &a) const { return v != a.to_vector3f(); }
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
		Equal_op,
		Not_equal_op,
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


class pi_matrix {
public:
	pi_matrix(): m(0.0) { }
	pi_matrix(float v): m(v) { }
	pi_matrix(const pi_matrix &a) { m = a.to_matrix4x4f(); }
	pi_matrix(const matrix4x4f &a) { m = a; }

	pi_matrix(const pi_vector &v1, const pi_vector &v2, const pi_vector &v3)
	{
		m = matrix4x4f::MakeRotMatrix(v1,v2,v3); 
	}
	pi_matrix(const pi_vector &v1, const pi_vector &v2, const pi_vector &v3, const pi_vector &v4)
	{ 
		m = matrix4x4f::MakeRotMatrix(v1,v2,v3); 
		m[12] = v4.x();
		m[13] = v4.y();
		m[14] = v4.z();
	}

    inline operator matrix4x4f () const { return m; }
	inline matrix4x4f to_matrix4x4f() const { return m; }

	inline pi_matrix operator+(const pi_matrix &a) const { return m + a.to_matrix4x4f(); }
	inline pi_matrix operator-(const pi_matrix &a) const { return m - a.to_matrix4x4f(); }
	inline pi_matrix operator*(const pi_matrix &a) const { return m * a.to_matrix4x4f(); }

	inline pi_matrix inverse() const { return m.InverseOf(); }

	static inline pi_matrix identity() { return matrix4x4f::Identity(); }
	static inline pi_matrix rotation(float ang, const pi_vector &v) { return matrix4x4f::RotateMatrix(ang, v.x(), v.y(), v.z()); }
	static inline pi_matrix translation(const pi_vector &v) { return matrix4x4f::Translation(v.x(), v.y(), v.z()); }
	static inline pi_matrix scale(const pi_vector &v) { return matrix4x4f::ScaleMatrix(v.x(), v.y(), v.z()); }

	static inline pi_matrix orient(const pi_vector &pos, const pi_vector &x, const pi_vector &y)
	{
		vector3f z = x.to_vector3f().Cross(y.to_vector3f()).Normalized();

		matrix4x4f o = matrix4x4f::MakeInvRotMatrix(
			y.to_vector3f().Cross(z).Normalized(),
			z.Cross(x.to_vector3f()),
			z);
		o[12] = pos.x();
		o[13] = pos.y();
		o[14] = pos.z();

		return o;
	}

	inline void print() const { m.Print(); }

private:
	matrix4x4f m;
};

OOLUA_CLASS_NO_BASES(pi_matrix)
	OOLUA_TYPEDEFS
		Add_op,
		Sub_op,
		Mul_op
	OOLUA_END_TYPES
	OOLUA_CONSTRUCTORS_BEGIN
		OOLUA_CONSTRUCTOR_1(float)
		OOLUA_CONSTRUCTOR_3(const pi_vector&, const pi_vector&, const pi_vector&)
		OOLUA_CONSTRUCTOR_4(const pi_vector&, const pi_vector&, const pi_vector&, const pi_vector&)
	OOLUA_CONSTRUCTORS_END
	OOLUA_MEM_FUNC_0_CONST(pi_matrix, inverse)
	OOLUA_MEM_FUNC_0_CONST(void, print)
OOLUA_CLASS_END

namespace PiLuaClasses
{
	void RegisterClasses(lua_State *l);
}

#endif
