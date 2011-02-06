#ifndef _PILUAAPI_H
#define _PILUAAPI_H

#include "libs.h"
#include "mylua.h"
#include "Object.h"
#include "StarSystem.h"
#include <map>

void RegisterPiLuaAPI(lua_State *L);

/*
 * Push Object* to lua like so:
 *
 * OOLUA::push2lua(Pi::player);
 *
 * This wrapper makes sure it is safe to delete Object*, and that objects can be used
 * as keys in lua tables (need one userdata object per Object* for this to work).
 */
class ObjectWrapper
{
	public:
	ObjectWrapper(): m_obj(0) {}
	bool IsBody() const;
	bool IsValid() const { return m_obj != NULL; }
	const char *GetLabel() const;
	//void BBAddAdvert(const BBAddAdvert &a) { m_bbadverts.push_back(a); }
	void SpaceStationAddAdvert(const char *luaMod, int luaRef, const char *description);
	void SpaceStationRemoveAdvert(const char *luaMod, int luaRef);
	double GetMoney() const;
	void SetMoney(double m);
	void AddMoney(double m);
	void SetLabel(const char *label);
	double GetEquipmentPrice(int equip_type);


	void ShipAIDoKill(ObjectWrapper &o);
	void ShipAIDoFlyTo(ObjectWrapper &o);
	void ShipAIDoLowOrbit(ObjectWrapper &o);
	void ShipAIDoMediumOrbit(ObjectWrapper &o);
	void ShipAIDoHighOrbit(ObjectWrapper &o);
	void ShipAIDoJourney(SBodyPath *destination);

	SBodyPath *GetSBody();
	Object *GetDockedWith();
	friend bool operator==(const ObjectWrapper &a, const ObjectWrapper &b) {
		return a.m_obj == b.m_obj;
	}
	virtual ~ObjectWrapper();
	// not much point making this private since it isn't exposed to lua
	Object *m_obj;
	bool Is(Object::Type t) const;
	static void Push(lua_State* const s, Object * const &o);
	protected:
	void OnDelete();
	sigc::connection m_delCon;
	private:
	// lua object reference (int)
	static std::map<Object *, int> objWrapLookup;
	ObjectWrapper(Object *o);
};

OOLUA_CLASS_NO_BASES(Object)
	OOLUA_NO_TYPEDEFS
	OOLUA_ONLY_DEFAULT_CONSTRUCTOR
OOLUA_CLASS_END

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
	OOLUA_MEM_FUNC_1(void, SetLabel, const char *)
	OOLUA_MEM_FUNC_1(void, SetMoney, double)
	OOLUA_MEM_FUNC_1(void, AddMoney, double)
	OOLUA_MEM_FUNC_1(double, GetEquipmentPrice, int)
	OOLUA_MEM_FUNC_1(void, ShipAIDoKill, ObjectWrapper&)
	OOLUA_MEM_FUNC_1(void, ShipAIDoFlyTo, ObjectWrapper&)
	OOLUA_MEM_FUNC_1(void, ShipAIDoLowOrbit, ObjectWrapper&)
	OOLUA_MEM_FUNC_1(void, ShipAIDoMediumOrbit, ObjectWrapper&)
	OOLUA_MEM_FUNC_1(void, ShipAIDoHighOrbit, ObjectWrapper&)
	OOLUA_MEM_FUNC_1(void, ShipAIDoJourney, SBodyPath*)
	OOLUA_MEM_FUNC_0(OOLUA::lua_out_p<SBodyPath*>, GetSBody);
	OOLUA_MEM_FUNC_0(Object*, GetDockedWith);
	OOLUA_MEM_FUNC_0_CONST(double,GetMoney)
	OOLUA_MEM_FUNC_0_CONST(bool, IsBody)
	OOLUA_MEM_FUNC_0_CONST(bool, IsValid)
	OOLUA_MEM_FUNC_0_CONST(const char *, GetLabel)
OOLUA_CLASS_END

namespace OOLUA {
	template<>
	inline bool push2lua(lua_State* const s, Object * const &  value) {
		ObjectWrapper::Push(s, value);
		return true;
	}
	template<>
	inline bool push2lua(lua_State* const s, Object * const &  value,Owner owner) {
		ObjectWrapper::Push(s, value);
		return true;
	}
	template<>
	inline bool push2lua(lua_State* const s, OOLUA::lua_acquire_ptr<Object * const>&  value) {
		ObjectWrapper::Push(s, value.m_ptr);
		return true;
	}
	static inline bool push2lua(lua_State* const s, Object* const &  value) {
		ObjectWrapper::Push(s, value);
		return true;
	}
	//template<typename T>bool push2lua(lua_State* const s, T * const &  value,OOLUA::Owner);
}
	

#endif /* _PILUAAPI_H */
