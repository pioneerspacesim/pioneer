#ifndef _PILUAAPI_H
#define _PILUAAPI_H

#include "libs.h"
#include "mylua.h"
#include "Object.h"

void RegisterPiLuaAPI(lua_State *L);

class ObjectWrapper
{
	public:
	ObjectWrapper(): m_obj(0) {}
	ObjectWrapper(Object *o);
	bool IsBody() const;
	const char *GetLabel() const;
	//void BBAddAdvert(const BBAddAdvert &a) { m_bbadverts.push_back(a); }
	void SpaceStationAddAdvert(const char *luaMod, int luaRef, const char *description);
	void SpaceStationRemoveAdvert(const char *luaMod, int luaRef);
	double GetMoney() const;
	void SetMoney(double m);
	int print() const;
	friend bool operator==(const ObjectWrapper &a, const ObjectWrapper &b) {
		return a.m_obj == b.m_obj;
	}
	virtual ~ObjectWrapper();
	// not much point making this private since it isn't exposed to lua
	Object *m_obj;
	bool Is(Object::Type t) const;
	protected:
	void OnDelete();
	sigc::connection m_delCon;
};

OOLUA_CLASS_NO_BASES(ObjectWrapper)
	OOLUA_TYPEDEFS
		OOLUA::Equal_op
	OOLUA_END_TYPES
	OOLUA_ONLY_DEFAULT_CONSTRUCTOR
//	OOLUA_CONSTRUCTORS_BEGIN
//		OOLUA_CONSTRUCTOR_1(const ObjectWrapper &)
//	OOLUA_CONSTRUCTORS_END
	OOLUA_MEM_FUNC_3(void, SpaceStationAddAdvert, const char *, int, const char *)
	OOLUA_MEM_FUNC_2(void, SpaceStationRemoveAdvert, const char *, int)
	OOLUA_MEM_FUNC_1(void, SetMoney, double)
	OOLUA_MEM_FUNC_0_CONST(int,print)
	OOLUA_MEM_FUNC_0_CONST(double,GetMoney)
	OOLUA_MEM_FUNC_0_CONST(bool, IsBody)
	OOLUA_MEM_FUNC_0_CONST(const char *, GetLabel)
OOLUA_CLASS_END

#endif /* _PILUAAPI_H */
